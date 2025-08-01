cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(networklib 
	LANGUAGES C CXX
	VERSION 1.0
)

enable_testing()

add_subdirectory(src)
#add_subdirectory(tests)

