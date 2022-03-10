cmake_minimum_required(VERSION 3.19)
set(PROJECT_NAME LanCon)

project(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#Make this project the default VS project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set (CMAKE_CXX_STANDARD 17)

#add_executable(${PROJECT_NAME} "")
add_library(${PROJECT_NAME} SHARED "")

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC interface)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(interface)

find_package(asio CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE asio asio::asio)

#Combines IDE filters to match file structure
include(CMakeGroupSources.txt)

