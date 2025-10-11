cmake_minimum_required(VERSION 3.10)
project(picore_xr LANGUAGES CXX CUDA)

find_package(Boost 1.70 REQUIRED)

# Optionally build CUDA kernels if available
find_package(CUDAToolkit QUIET)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library sources
set(PICORE_SOURCES
    src/bigint_rns.cpp
    src/convolution_cpu_ntt.cpp
    src/bs_chudnovsky.cpp
    src/chudnovsky.cpp
    src/resources.cpp
    src/thread_pool.cpp
    src/far_memory.cpp
    src/bbp_spotcheck.cpp
)

# CUDA kernel (optional)
if(CUDAToolkit_FOUND)
    # Add CUDA kernels for NTT and Chudnovsky term evaluation.
    list(APPEND PICORE_SOURCES cuda/ntt_kernels.cu cuda/chudnovsky_terms.cu src/chudnovsky_gpu.cu)
    add_definitions(-DPICORE_WITH_CUDA)

endif()

add_library(picore STATIC ${PICORE_SOURCES})

if(Boost_FOUND)
    target_include_directories(picore PRIVATE ${Boost_INCLUDE_DIR})
endif()

if(CUDAToolkit_FOUND)
    target_include_directories(picore PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
endif()


if(DEFINED DEC_DIGITS)
    target_compile_definitions(picore PRIVATE DEC_DIGITS=${DEC_DIGITS})
endif()

# Executable
add_executable(pi-error-wtf tools/pi_cli.cpp)
set_target_properties(pi-error-wtf PROPERTIES LINK_FLAGS "/MANIFEST:NO")
target_link_libraries(pi-error-wtf PRIVATE picore)

# Installation and packaging
include(GNUInstallDirs)
install(TARGETS pi-error-wtf DESTINATION ${CMAKE_INSTALL_BINDIR})

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "pi-error-wtf")
set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lino Casu")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Pi computation suite for the collective.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)