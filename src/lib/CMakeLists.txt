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

include(GNUInstallDirs)
install(TARGETS networklib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include)

