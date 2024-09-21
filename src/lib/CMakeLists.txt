add_library(
)

target_link_libraries(networklib PRIVATE
	pthread
)

target_include_directories(networklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
