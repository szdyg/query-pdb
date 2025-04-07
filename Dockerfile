FROM ubuntu:22.04

MAINTAINER szdyg "szddyg@outlook.com"

ENV MSDL_DOWNLOAD_SERVER = "https://msdl.microsoft.com/download/symbols"
ENV QUERY_PDB_SAVE_PATH = "/pdb"

RUN apt-get update

RUN apt-get install -y \
    build-essential \
    cmake \
    libssl-dev

COPY . /query-pdb/

RUN cd /query-pdb && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    cmake --build . --target query_pdb_server

ENTRYPOINT ["/query-pdb/build/server/query_pdb_server"]
