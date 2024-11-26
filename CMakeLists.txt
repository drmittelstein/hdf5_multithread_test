# Farfield v.0.18.0
cmake_minimum_required(VERSION 3.12)
include(FetchContent)
project(hdf5_multithread_test)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Select HDF5 build strategy
set(HDF5_BUILD_FROM_SOURCE OFF CACHE BOOL "" FORCE)

# HDF5 configuration flags
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(BUILD_STATIC_LIBS OFF CACHE BOOL "Build static libraries" FORCE)
set(HDF5_ENABLE_THREADSAFE ON CACHE BOOL "" FORCE)
set(HDF5_BUILD_HL_LIB OFF CACHE BOOL "" FORCE) # Disable high-level APIs for thread safety
set(HDF5_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Disable examples
set(HDF5_BUILD_TOOLS OFF CACHE BOOL "" FORCE) # Disable tools

# Load HDF5 package and build from source
if (HDF5_BUILD_FROM_SOURCE)
    FetchContent_Declare(
        hdf5
        GIT_REPOSITORY https://github.com/HDFGroup/hdf5.git
        GIT_TAG hdf5_1.14.5
    )
    FetchContent_MakeAvailable(hdf5)
    set (LINK_LIBS ${LINK_LIBS} hdf5-shared)
    set (HDF5_INCLUDE_DIRS ${hdf5_BINARY_DIR}/src)
else()
    # Load pre-installed HDF5 package
    set (LIB_TYPE SHARED) # STATIC or SHARED
    string(TOLOWER ${LIB_TYPE} SEARCH_TYPE)
    find_package(HDF5 NAMES hdf5 COMPONENTS C ${SEARCH_TYPE})
    set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_${LIB_TYPE}_LIBRARY})
endif()

# HDF5 package information
message(STATUS "hdf5_POPULATED: ${hdf5_POPULATED}")
message(STATUS "hdf5_BINARY_DIR: ${hdf5_BINARY_DIR}")
message(STATUS "hdf5_SOURCE_DIR: ${hdf5_SOURCE_DIR}")
message(STATUS "HDF5 Found: ${HDF5_FOUND}")
message(STATUS "HDF5_VERSION: ${HDF5_VERSION}")
message(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}")
message(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
message(STATUS "HDF5_DEFINITIONS: ${HDF5_DEFINITIONS}")
message(STATUS "HDF5 include: ${HDF5_INCLUDE_DIR}")
message(STATUS "HDF5 library: ${LINK_LIBS}")

# Include CPP files
file(GLOB_RECURSE SOURCES "src/*.cpp")
message(STATUS "Cpp file sources:")
foreach(SOURCE ${SOURCES})
    message(STATUS "- ${SOURCE}")
endforeach()

# Executable definition and properties
add_executable(${PROJECT_NAME} ${SOURCES})
add_subdirectory(src)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Linking and include directories
set(CMAKE_VERBOSE_MAKEFILE ON)
target_link_libraries (${PROJECT_NAME} ${LINK_LIBS}) # Link HDF5 libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${HDF5_INCLUDE_DIRS}) # Include HDF5 headers