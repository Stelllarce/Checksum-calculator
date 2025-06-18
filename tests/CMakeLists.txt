add_executable(unit-tests)

target_link_libraries(
    unit-tests
    PRIVATE
        Catch2::Catch2WithMain
        calculators
)

target_sources(
    unit-tests
    PRIVATE
        test_calculators/test_md5.cpp
        test_calculators/test_sha1.cpp
        test_calculators/test_sha256.cpp
)

# Automatically register all tests
catch_discover_tests(unit-tests)
