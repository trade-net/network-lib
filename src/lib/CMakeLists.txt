add_library(networklib
	tcp_session.cpp
	tcp_async_server.cpp
	threadpool.cpp
)

target_link_libraries(networklib PRIVATE
	protolib
	pthread
)

target_include_directories(networklib PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
)

