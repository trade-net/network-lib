find_package(Protobuf REQUIRED)
find_package(absl REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})

set(PROTO_FILES
	order.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(protolib 
	${PROTO_SRCS}
	${PROTO_HDRS}
)

target_link_libraries(protolib PRIVATE
	protobuf::libprotobuf
	absl::base
	absl::strings
	absl::log
	absl::raw_logging_internal
	absl::check
	${Protobuf_LIBRARIES}
)

target_include_directories(protolib PUBLIC 
	${CMAKE_CURRENT_BINARY_DIR}
	${Protobuf_INCLUDE_DIRS}
)
