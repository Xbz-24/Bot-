cmake_minimum_required(VERSION 3.24)
project(Botpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(PkgConfig)
pkg_check_modules(DPP_PKG dpp)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil)
pkg_check_modules(LIBOGGZ REQUIRED oggz)
pkg_check_modules(OPUSFILE REQUIRED opusfile)
pkg_check_modules(LIBOPUS REQUIRED opus)

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost 1.83.0 REQUIRED)
message("Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message("Boost_LIBRARIES: ${Boost_LIBRARIES}")

if(DPP_PKG_FOUND)
    message(STATUS "DPP found with pkg-config")
    set(DPP_INCLUDE_DIRS ${DPP_PKG_INCLUDE_DIRS})
    set(DPP_LIBRARIES ${DPP_PKG_LIBRARIES})
else()
    find_package(DPP REQUIRED)
    if(DPP_FOUND)
        message(STATUS "DPP found with CMake find_package")
    else()
        message(FATAL_ERROR "DPP not found")
    endif()
endif()

find_package(OpenSSL REQUIRED)
include_directories(
        ${Boost_INCLUDE_DIRS}
        ${DPP_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIRS}
        ${LIBOGGZ_INCLUDE_DIRS}
        ${OPUSFILE_INCLUDE_DIRS}
        ${LIBOPUS_INCLUDE_DIRS}
)
set(CMAKE_CXX_STANDARD 23)

add_executable(Botpp bot.cpp)
target_link_libraries(
        Botpp
        ${DPP_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${FFMPEG_LIBRARIES}
        ${LIBOGGZ_LIBRARIES}
        ${OPUSFILE_LIBRARIES}
        ${LIBOPUS_LIBRARIES}
)
