add_executable(network
	main.cpp
)

target_link_libraries(network PRIVATE
	networklib
)

install(TARGETS network RUNTIME COMPONENT network)

