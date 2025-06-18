add_library(lib STATIC)

# Add external library for hashing functions
target_include_directories(
    lib
    PUBLIC
        hash-library
)

target_sources(
    lib
    PRIVATE
        hash-library/md5.h
        hash-library/md5.cpp
        hash-library/sha1.cpp
        hash-library/sha256.cpp
)