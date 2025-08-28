add_executable(main)

# Set a more descriptive name for the command-line executable
set_target_properties(main PROPERTIES OUTPUT_NAME "checksum-calculator")

target_link_libraries(
	main
	PRIVATE
		calculators
		file-system-composite
        directory-iteration-visitors
        directory-tree-builders
        utils
        # progress_reporter_lib
		tclap::tclap
)

target_sources(
	main
	PRIVATE
		"main.cpp"
)