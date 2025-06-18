add_library(calculators STATIC)

# set_target_properties(calculators PROPERTIES OUTPUT_NAME "calculators")

target_link_libraries(
    calculators
    PRIVATE
        lib
)

target_sources(
    calculators
    PRIVATE
        Md5Calculator.cpp
)