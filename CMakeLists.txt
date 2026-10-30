# CMake build for the texturing library and the texrecon command-line tool.
#
# Dependencies:
#   vcpkg (vcpkg.json): Eigen, TBB, libjpeg-turbo, libpng, libtiff
#   FetchContent:       MVE (util, math and mve libraries), rayint, mapmap

cmake_minimum_required(VERSION 3.21)
project(Texturing LANGUAGES C CXX)

option(TEX_UNSAFE_MATH "Use -funsafe-math-optimizations" OFF)
option(TEX_NATIVE_ARCH "Optimize for the build machine (-march=native)" OFF)
option(TEX_USE_OPENMP  "Use OpenMP if available" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Executables (and the OpenMP runtime on Windows) in one directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
# Debug information is only needed for profiling: configure with
# -DCMAKE_CXX_FLAGS_RELEASE="-O3 -g".
set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "")

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
if(TEX_USE_OPENMP)
    if(WIN32)
        # Prefer LLVM's own libomp.lib/libomp.dll (redistributable) over the one
        # shipped with Visual Studio, which imports libomp140.x86_64.dll.
        get_filename_component(llvm_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
        if(EXISTS "${llvm_bin_dir}/../lib/libomp.lib")
            get_filename_component(llvm_omp_lib "${llvm_bin_dir}/../lib/libomp.lib" ABSOLUTE)
            set(OpenMP_libomp_LIBRARY "${llvm_omp_lib}" CACHE FILEPATH "" FORCE)
        endif()
    endif()
    find_package(OpenMP COMPONENTS CXX)
endif()

# ---- Sources of the dependencies ------------------------------------------

include(FetchContent)

# MVE provides the util, math and mve libraries. Use
# -DFETCHCONTENT_SOURCE_DIR_MVE=<path> to build against a local checkout.
set(MVE_BUILD_APPS     OFF CACHE BOOL "" FORCE)
set(MVE_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(MVE_USE_HIGHWAY    OFF CACHE BOOL "" FORCE)
set(MVE_USE_MIMALLOC   OFF CACHE BOOL "" FORCE)
set(MVE_UNSAFE_MATH    ${TEX_UNSAFE_MATH} CACHE BOOL "" FORCE)
set(MVE_NATIVE_ARCH    ${TEX_NATIVE_ARCH} CACHE BOOL "" FORCE)
set(MVE_USE_OPENMP     ${TEX_USE_OPENMP}  CACHE BOOL "" FORCE)
FetchContent_Declare(mve
    GIT_REPOSITORY https://github.com/rhiestan/mve.git
    GIT_TAG        e60c25f5703bdb803e192c9e4422f7b5e094ba32)

# rayint (acc/bvh_tree.h) and mapmap are header-only; they have their own
# CMakeLists.txt that must not be used, hence the non-existent SOURCE_SUBDIR.
FetchContent_Declare(rayint
    GIT_REPOSITORY https://github.com/nmoehrle/rayint.git
    GIT_TAG        b62c8905f6dd11a179128517e814e0e00f9bb809
    SOURCE_SUBDIR  no-cmake)
FetchContent_Declare(mapmap
    GIT_REPOSITORY https://github.com/dthuerck/mapmap_cpu.git
    GIT_TAG        fa526e0
    SOURCE_SUBDIR  no-cmake)
FetchContent_MakeAvailable(mve rayint mapmap)

# ---- Flags shared by all targets -------------------------------------------

add_library(tex_options INTERFACE)
target_include_directories(tex_options SYSTEM INTERFACE
    ${rayint_SOURCE_DIR}/libs
    ${mapmap_SOURCE_DIR}
    ${mapmap_SOURCE_DIR}/mapmap
    ${mapmap_SOURCE_DIR}/ext/dset)
target_include_directories(tex_options INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_compile_definitions(tex_options INTERFACE EIGEN_MPL2_ONLY)
target_compile_options(tex_options INTERFACE -Wall -Wextra -Wundef -pedantic)
if(TEX_UNSAFE_MATH)
    target_compile_options(tex_options INTERFACE -funsafe-math-optimizations)
endif()
if(TEX_NATIVE_ARCH)
    target_compile_options(tex_options INTERFACE -march=native)
endif()
if(WIN32)
    target_compile_definitions(tex_options INTERFACE
        _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN _USE_MATH_DEFINES
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif()
target_link_libraries(tex_options INTERFACE
    mve mve_util mve_math Eigen3::Eigen TBB::tbb
    JPEG::JPEG PNG::PNG TIFF::TIFF)
if(OpenMP_CXX_FOUND)
    target_link_libraries(tex_options INTERFACE OpenMP::OpenMP_CXX)
endif()

add_subdirectory(libs)
add_subdirectory(apps)

# On Windows the LLVM OpenMP runtime is a DLL that has to ship with the tool.
if(WIN32 AND OpenMP_CXX_FOUND)
    get_filename_component(compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    find_file(TEX_LIBOMP_DLL libomp.dll HINTS "${compiler_dir}" NO_DEFAULT_PATH)
    if(TEX_LIBOMP_DLL)
        add_custom_target(tex_copy_libomp ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TEX_LIBOMP_DLL}" "${CMAKE_BINARY_DIR}")
        add_dependencies(texrecon tex_copy_libomp)
        install(FILES "${TEX_LIBOMP_DLL}" DESTINATION bin)
    else()
        message(WARNING "libomp.dll not found next to the compiler; copy it manually")
    endif()
endif()
