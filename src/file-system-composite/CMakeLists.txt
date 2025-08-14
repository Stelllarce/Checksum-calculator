add_library(file-system-composite STATIC)

target_sources(
    file-system-composite
    PRIVATE
        "Directory.cpp"
        "File.cpp"
        "FileObject.cpp"
)