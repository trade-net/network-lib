add_executable(network
	main.cpp
)

add_executable(client
	tcp_client.cpp
)

target_link_libraries(network PRIVATE
	protolib
	networklib
)

target_link_libraries(client PRIVATE
	networklib
)

install(TARGETS network RUNTIME COMPONENT network)
install(TARGETS client RUNTIME COMPONENT client)

