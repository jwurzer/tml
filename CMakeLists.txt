
cmake_minimum_required(VERSION 3.0.0)

cmake_policy(VERSION 3.0.0)

set(PRJ_NAME "tml")
set(PRJ_EXENAME "tml")

project(${PRJ_NAME})

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_COMPILER_IS_GNUCXX)
	set (CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wconversion -Winit-self -Wfloat-equal -Wreorder")
	set (CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0")
	set (CMAKE_CXX_FLAGS_RELEASE  "-O3")
endif ()

file(GLOB_RECURSE PRJ_HEADERFILES include/*.h)
file(GLOB_RECURSE PRJ_SOURCEFILES src/*.cpp)

add_executable(${PRJ_EXENAME} ${PRJ_HEADERFILES} ${PRJ_SOURCEFILES})

target_include_directories(${PRJ_EXENAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

if (WIN32)

set(_src_root_path "${CMAKE_SOURCE_DIR}")

foreach(_source IN ITEMS ${PRJ_SOURCEFILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

foreach(_source IN ITEMS ${PRJ_HEADERFILES})
    get_filename_component(_source_path "${_source}" PATH)
    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

endif()
