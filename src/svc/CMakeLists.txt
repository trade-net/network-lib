add_executable(network
	main.cpp
)

add_executable(client
	tcp_client.cpp
)

target_link_libraries(network PRIVATE
	networklib
	protolib
)

target_link_libraries(client PRIVATE
	protolib
	networklib
)

install(TARGETS network RUNTIME COMPONENT network)
install(TARGETS client RUNTIME COMPONENT client)

