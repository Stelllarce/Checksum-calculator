add_library(fscomp STATIC)

target_sources(
    fscomp
    PRIVATE
        "Directory.cpp"
        "File.cpp"
        "FileObject.cpp"
)