cmake_minimum_required(VERSION 3.1)
project(space_simulation CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(./space_simulation)

#find_package(Eigen3 REQUIRED)

add_executable(simulation main.cpp)
#target_include_directories(simulation PRIVATE ${EIGEN3_INCLUDE_DIR})
target_include_directories(simulation PRIVATE /Users/yuasatakuhiroshi/Downloads/Sprout-master)
#target_link_libraries(main.out )
