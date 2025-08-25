add_executable(unit-tests)

target_link_libraries(
    unit-tests
    PRIVATE
        Catch2::Catch2WithMain
        calculators
        file-system-composite
        directory-tree-builders
)

target_sources(
    unit-tests
    PRIVATE
        "test-calculators/test_md5.cpp"
        "test-calculators/test_sha1.cpp"
        "test-calculators/test_sha256.cpp"
        "test-file-system/test_file.cpp"
        "test-file-system/test_directory.cpp"
        "test-file-system/test_link.cpp"
        "test-utils/test-cycle-detector.cpp"
        "test-builders/test_follow_link_builder.cpp"
        "test-builders/test_non_follow_link_builder.cpp"
        "test-builders/test_directory_constructor.cpp"
)

# Automatically register all tests
catch_discover_tests(unit-tests)

# Add a custom target to run tests with suppressed stderr
add_custom_target(test-quiet
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure 2>/dev/null
    DEPENDS unit-tests
    COMMENT "Running tests with suppressed error output"
)
