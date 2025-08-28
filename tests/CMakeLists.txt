add_executable(unit-tests)

target_link_libraries(
    unit-tests
    PRIVATE
        Catch2::Catch2WithMain
        calculators
        file-system-composite
        directory-tree-builders
        directory-iteration-visitors
        utils
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
        "test-utils/test_checksum_file_reader.cpp"
        "test-utils/test_verification_result_printer.cpp"
        "test-builders/test_follow_link_builder.cpp"
        "test-builders/test_non_follow_link_builder.cpp"
        "test-builders/test_directory_constructor.cpp"
        "test-visitors/test_hash_writer.cpp"
        "test-visitors/test_report_writer.cpp"
        "test-visitors/test_verification_visitor.cpp"
)

# Automatically register all tests
catch_discover_tests(unit-tests)

