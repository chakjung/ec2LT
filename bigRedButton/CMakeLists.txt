cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 11)

project(bigRedButton)

find_package(AWSSDK REQUIRED COMPONENTS ec2)

add_executable(${PROJECT_NAME} "./bigRedButton.cpp")

add_compile_options(-Wall -Wextra -pedantic -Werror)

target_link_libraries(${PROJECT_NAME}
                      ${AWSSDK_LINK_LIBRARIES})
