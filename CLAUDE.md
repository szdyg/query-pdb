# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

An HTTP service that downloads Windows PDB files from a Microsoft symbol server (or mirror) on demand, parses them, and returns symbol RVAs / struct field offsets / enum values as JSON. Clients never download the full PDB.

This fork diverges from upstream `zouxianyu/query-pdb` (see README "关于这个分支"): requests are `GET` instead of `POST` so CDN/nginx can cache them, `guid` is the concatenated PE GUID **+ age** as a single parameter, and the Windows server build and the client were removed.

## Build

Linux only — `server/CMakeLists.txt` hard-fails with `FATAL_ERROR` on Windows. Requires OpenSSL >= 3.0.0 (`libssl-dev` on Debian/Ubuntu, `openssl-dev` on Alpine) and CMake >= 3.16. On this Windows dev box, build inside Docker or WSL, not natively.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel --target query_pdb_server
# binary: build/server/query_pdb_server
```

Docker (this is the production path):

```sh
docker build -t szdyg/query-pdb:latest .   # multi-stage: compiles in a builder, ships only the binary
docker compose up -d                       # port 8080
```

`docker-compose.yml` pulls `szdyg/query-pdb:latest` rather than building, so tag a local build with that exact name or compose will silently run the last image from Docker Hub instead of your changes.

The image is Alpine-based, so **the container links against musl, not glibc**. Two consequences worth remembering: glibc-only extensions won't compile (this is why `pdb_parser.cpp` includes `<cstring>` explicitly for `strnlen` — libstdc++ pulled it in implicitly, musl does not), and musl's default thread stack is 128k against glibc's 8M, which is why the builder passes `-Wl,-z,stack-size=1048576`. The CI workflow still builds against glibc on `ubuntu-*` runners, so a musl-only breakage will not show up there.

The runtime stage carries just `ca-certificates`, `libssl3`, `libcrypto3` and `libstdc++` — if you add a dependency that pulls in another shared library, add it there too or the image will build fine and then fail at startup. `ca-certificates` in particular is load-bearing: without it OpenSSL cannot verify the symbol server's certificate and every HTTPS download fails. The builder passes `-DHTTPLIB_USE_{ZLIB,BROTLI,ZSTD}_IF_AVAILABLE=OFF` on purpose; those default to ON, which would otherwise make the build's behaviour depend on which `-dev` packages are installed (see the `Content-Length` note below).

There are no tests and no lint config. Verify changes by running the server and hitting the endpoints:

```sh
QUERY_PDB_SAVE_PATH=/tmp/pdb MSDL_DOWNLOAD_SERVER=https://msdl.microsoft.com ./build/server/query_pdb_server
curl 'http://localhost:8080/symbol?pdb=ntkrnlmp.pdb&guid=8F0F3D677778391600F4EB2301FFC7A51&query=KdpStub,MmAccessFault'
```

## Runtime configuration

Two environment variables, both **required** — `require_env()` in `main.cpp` logs at `critical` and exits with `EXIT_FAILURE` if either is unset or empty:

- `QUERY_PDB_SAVE_PATH` — on-disk PDB cache root (`/pdb` volume in Docker)
- `MSDL_DOWNLOAD_SERVER` — symbol server base URL; `/download/symbols/<pdb>/<guid>/<pdb>` is appended

The listen address `0.0.0.0:8080` is hardcoded at the bottom of `main.cpp`.

## Architecture

Three layers, all under `server/`:

1. **`main.cpp`** — cpp-httplib routes `/symbol`, `/struct`, `/enum`. All three share the same shape: `parse_request()` → `downloader::download()` → construct a `pdb_parser` → `set_result()`. `parse_request()` is the only place `pdb`/`guid` are validated; since both end up in a filesystem path *and* in the request line sent to the symbol server, they go through a strict character whitelist (`is_valid_pdb_name` / `is_valid_guid`) — do not bypass it when adding an endpoint. Errors propagate as exceptions to a server-wide `set_exception_handler` returning 500 with the `what()` text. Successful responses are marked `Cache-Control: public, max-age=31536000, immutable` (a `(pdb, guid, query)` tuple is immutable by construction); 4xx/5xx are marked `no-store` so a CDN never pins a transient download failure.

2. **`downloader`** — content-addressed disk cache at `<save_path>/<name>/<guid>/<name>`. Downloads to a `.tmp` sibling and `rename()`s into place so a torn download is never observed as a valid cache entry, and rejects responses whose body length doesn't match `Content-Length`. That check assumes the response is not compressed — if httplib is ever built with a compression backend enabled it will advertise `Accept-Encoding` and transparently decompress, while the header still reports the compressed size, and then *every* download is rejected. This is why the Dockerfile disables them explicitly. Because publication is an atomic rename, the cache-hit path (`std::filesystem::exists`) needs no lock at all; only a miss takes a lock, and that lock is per-`(pdb, guid)` (`file_lock()`, a `weak_ptr` map pruned on each call) followed by a double-check. Downloading a cold PDB must never block requests for a different one.

3. **`pdb_parser`** — wraps [raw_pdb](https://github.com/MolecularMatters/raw_pdb) over a memory-mapped file (`handle_guard` is the RAII wrapper around `MemoryMappedFile::Handle`; it is non-copyable and non-movable). The `call_with_pdb_stream` template in `pdb_parser.h` performs all the stream validation once and forwards `(raw_file, dbi_stream, tpi_stream, ...)` to each `*_impl` static — new query kinds should be written as another static taking that signature.

Parsing details worth knowing before touching `pdb_parser.cpp`:

- **Symbols** are searched across three sources in order — public symbol stream, global symbol stream, then per-module symbol streams. Symbols with RVA 0 (e.g. CFG symbols) are skipped. Names in `query` that are never found are returned as `-1` rather than omitted, so the response always has one key per requested name. Struct/enum queries do *not* do this — unmatched names are simply absent.
- **Structs and enums** are found by linearly scanning `type_table.GetTypeRecords()` for `LF_STRUCTURE`/`LF_UNION`/`LF_ENUM` with `property.fwdref == 0`. This is a full scan per request, and it does not stop early once every requested name has been found.
- `get_struct_single` and `get_enum_single` walk CodeView `FieldList` records by hand with pointer arithmetic and 4-byte alignment (`i = (i + 3) & ~3`). Offsets past a leaf name are computed from `leaf_name - (const char*)field_record`. This code is easy to break — the `LF_NUMERIC` branch selects between an inline `uint16` and one past the kind tag.
- `ExampleTypeTable.{h,cpp}`, `ExampleMemoryMappedFile.{h,cpp}` and `Examples_PCH.h` are copied verbatim from raw_pdb's `src/Examples/` directory; `pdb_helper.{h,cpp}` is a lightly edited copy of `ExampleTypes.cpp`. When bumping raw_pdb, re-sync all of them — the upstream `Examples/` are the reference for how the current API is meant to be used.
- `TypeTable` is that API's entry point for type lookups: upstream moved `GetTypeRecord(typeIndex)` / `GetTypeRecords()` off `TPIStream` into this helper, which walks the stream once to build an index→record array. Building it coalesces the whole TPI stream into memory, so it is constructed inside `get_struct_impl` / `get_enum_impl` rather than in `call_with_pdb_stream` — `/symbol` does not need it and should not pay for it.

## Dependencies

All of `thirdparty/` (cpp-httplib, cxxopts, nlohmann_json, raw_pdb, spdlog) is vendored in-tree as plain source, not submodules — there is no `.gitmodules` and nothing to `git submodule update`. Updating a dependency means replacing its directory. `cxxopts` is still linked in `server/CMakeLists.txt` but is no longer used anywhere (it was for the removed CLI/client); configuration moved to environment variables.

## Known stale bits

`.github/workflows/build_server.yml` has drifted from the current source: it still has a `build-on-windows` job (the CMake build now `FATAL_ERROR`s on Windows) and its `build-docker` job `sed`s a `supervisord.conf` that no longer exists in the repo. Don't treat that workflow as a description of how the project builds today.
