cmake_minimum_required(VERSION 3.12)
project(space_simulation CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(./space_simulation)

#find_package(Eigen3 REQUIRED)

add_executable(simulation main.cpp)

#target_include_directories(simulation PRIVATE ${EIGEN3_INCLUDE_DIR})

target_include_directories(simulation PRIVATE /Users/yuasatakuhiroshi/Downloads/Sprout-master)
target_include_directories(simulation PRIVATE /Users/yuasatakuhiroshi/GitHub/simulation/space_simulation/include)
#target_link_libraries(simulation gnuplot)
