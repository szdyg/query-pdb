# ---------- build ----------
FROM alpine:3.20 AS builder

RUN apk add --no-cache \
        build-base \
        cmake \
        linux-headers \
        openssl-dev

WORKDIR /src
COPY . .

# musl defaults to a 128k thread stack (glibc uses 8M). the httplib worker
# threads parse pdb records recursively, so raise the default before it bites.
#
# the httplib compression backends default to "use if available", which makes the
# build depend on whichever -dev packages happen to be present. they are turned
# off explicitly so the result is reproducible and the runtime image needs no
# compression libraries.
RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,-z,stack-size=1048576" \
        -DHTTPLIB_USE_ZLIB_IF_AVAILABLE=OFF \
        -DHTTPLIB_USE_BROTLI_IF_AVAILABLE=OFF \
        -DHTTPLIB_USE_ZSTD_IF_AVAILABLE=OFF && \
    cmake --build build --parallel "$(nproc)" --target query_pdb_server && \
    strip build/server/query_pdb_server

# ---------- runtime ----------
FROM alpine:3.20

LABEL maintainer="szdyg <szddyg@outlook.com>"

# ca-certificates is required: without it OpenSSL cannot verify the symbol
# server's certificate and every https download fails
RUN apk add --no-cache \
        ca-certificates \
        libcrypto3 \
        libssl3 \
        libstdc++

COPY --from=builder /src/build/server/query_pdb_server /usr/local/bin/query_pdb_server

ENV MSDL_DOWNLOAD_SERVER=https://msdl.microsoft.com
ENV QUERY_PDB_SAVE_PATH=/pdb

VOLUME /pdb
EXPOSE 8080

ENTRYPOINT ["/usr/local/bin/query_pdb_server"]
