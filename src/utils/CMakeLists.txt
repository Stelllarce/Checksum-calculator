add_library(utils
    ChecksumFileReader.cpp
    VerificationResultPrinter.cpp
)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
