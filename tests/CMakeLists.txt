add_executable(networklib.t
)

target_link_libraries(networklib.t PRIVATE
	gtest
	gtest_main
	rt
	pthread
	gmock
	networklib
)

add_test(
	NAME networklib.t
	COMMAND networklib.t
)
