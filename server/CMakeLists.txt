add_executable(
        query_pdb_server
        main.cpp
        downloader.cpp
        pdb_parser.cpp
        pdb_helper.cpp
        ExampleMemoryMappedFile.cpp
)

set_target_properties(
        query_pdb_server
        PROPERTIES
        CXX_STANDARD 17
)

target_include_directories(
        query_pdb_server
        PRIVATE
        ${CMAKE_SOURCE_DIR}/lib
)

# find openssl library
# on Linux, we use the library installed by system package manager
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR "No longer supports Windows")
else ()
    find_package(OpenSSL 3.0.0 REQUIRED)
endif ()

target_link_libraries(
        query_pdb_server
        PRIVATE
        raw_pdb
        spdlog
        cxxopts
        nlohmann_json
        httplib
        OpenSSL::SSL
        OpenSSL::Crypto
)
