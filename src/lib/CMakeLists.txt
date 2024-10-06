add_library(networklib
	tcp_session.cpp
	tcp_async_server.cpp
	threadpool.cpp
)

target_link_libraries(networklib PRIVATE
	pthread
)

target_include_directories(networklib PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
)

install(FILES
	tcp_async_server.h
	tcp_session.h
	request_processor.h
	threadpool.h
	DESTINATION include/networklib
)

#install(TARGETS networklib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS networklib
	EXPORT NetworkLibTargets
	LIBRARY DESTINATION lib
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/networklibConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY AnyNewerVersion
)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/networklibConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/networklibConfig.cmake"
	@ONLY
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/networklibConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/networklibConfigVersion.cmake"
	DESTINATION lib/cmake/networklib
)

install(EXPORT NetworkLibTargets
	FILE NetworkLibTargets.cmake
	DESTINATION lib/cmake/networklib
)

