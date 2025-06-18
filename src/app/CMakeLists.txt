add_executable(application)

# set_target_properties(application PROPERTIES OUTPUT_NAME "app")

target_link_libraries(
	application
	PRIVATE
		calculators
		tclap::tclap
)

target_sources(
	application
	PRIVATE
		"application.cpp"
)