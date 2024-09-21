add_library(networklib
	tcp_connection_handler.cpp
	tcp_async_server.cpp
)

target_link_libraries(networklib PRIVATE
	pthread
)

target_include_directories(networklib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

