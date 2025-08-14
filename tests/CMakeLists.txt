add_executable(unit-tests)

target_link_libraries(
    unit-tests
    PRIVATE
        Catch2::Catch2WithMain
        calculators
        file-system-composite
)

target_sources(
    unit-tests
    PRIVATE
        "test-calculators/test_md5.cpp"
        "test-calculators/test_sha1.cpp"
        "test-calculators/test_sha256.cpp"
        "test-file-system/test_file.cpp"
        "test-file-system/test_directory.cpp"
)

# Automatically register all tests
catch_discover_tests(unit-tests)
