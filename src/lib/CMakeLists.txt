add_library(networklib
	tcp_session.cpp
	tcp_async_server.cpp
	threadpool.cpp
)

#find_package(Protobuf REQUIRED)

#message(STATUS "Protobuf include dirs: ${Protobuf_INCLUDE_DIRS}")
#message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")

target_link_libraries(networklib PRIVATE
	pthread
	#${Protobuf_LIBRARIES}
)

target_include_directories(networklib PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	#${Protobuf_INCLUDE_DIRS}
)

