cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 11)

project(gru)

find_package(AWSSDK REQUIRED COMPONENTS ec2 dynamodb)

add_subdirectory(availabilityZone)
add_subdirectory(region)
add_subdirectory(subroutines)
add_subdirectory(database)
add_subdirectory(latency)

add_executable(${PROJECT_NAME} "./gru.cpp")

add_compile_options(-Wall -Wextra -pedantic -Werror -O3)

target_link_libraries(${PROJECT_NAME}
                      ${AWSSDK_LINK_LIBRARIES}
                      latency
                      database
                      subroutines
                      region
                      availabilityZone)
