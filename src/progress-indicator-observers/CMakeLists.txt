add_library(progress-indicator-observers STATIC)

target_link_libraries(
    progress-indicator-observers
    PRIVATE
        file-system-composite
        calculators
)

target_sources(
    progress-indicator-observers
    PRIVATE
        "Observable.cpp"
        "ProgressReporter.cpp"
)