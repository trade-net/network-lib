find_package(GTest QUIET)

if(NOT GTest_FOUND)
	message(STATUS "GTest not found, skipping networklib tests")
	return()
endif()

add_executable(networklib.t
	framing.t.cpp
)

target_link_libraries(networklib.t PRIVATE
	GTest::gtest
	GTest::gtest_main
	pthread
	networklib
	protolib
)

add_test(
	NAME networklib.t
	COMMAND networklib.t
)
