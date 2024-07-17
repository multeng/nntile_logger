cmake_minimum_required(VERSION 3.10)

project(nntile_logger)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_DIR ./src/)
file(GLOB SRC_FILES "${SRC_DIR}/*.cpp")

include_directories(/opt/homebrew/include/)

link_directories(/opt/homebrew/lib/)

find_package(PkgConfig REQUIRED)
pkg_check_modules(STARPU REQUIRED starpu-1.4)

include_directories(${STARPU_INCLUDE_DIRS})
link_directories(${STARPU_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${STARPU_LIBRARIES})
