add_library(directory-iteration-visitors STATIC)

target_link_libraries(
    directory-iteration-visitors
    PRIVATE
        file-system-composite
        calculators
        progress-indicator-observers
)

target_sources(
    directory-iteration-visitors
    PRIVATE
        "DirectoryIterationVisitor.cpp"
        "HashStreamWriter.cpp"
        "ReportWriter.cpp"
        "VerificationVisitor.cpp"
)