add_library(directory-tree-builders STATIC)

target_link_libraries(
    directory-tree-builders
    PRIVATE
        file-system-composite
)

target_sources(
    directory-tree-builders
    PRIVATE
        "CycleDetector.cpp"
)