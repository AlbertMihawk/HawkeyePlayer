
cmake_minimum_required(VERSION 3.4.1)

include_directories(include)


#此方法动态库有兼容问题,静态库没有问题
#add_library(
#        avcodec
#        STATIC #*.a
#        IMPORTED)
#set_target_properties(
#        avcodec
#        PROPERTIES
#        IMPORTED_LOCATION
#        ${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.a)
#message(WARNING "CMakeLists.txt -> CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
#message(WARNING "CMakeLists.txt -> ANDROID_ABI: ${CMAKE_ANDROID_ARCH_ABI}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${CMAKE_ANDROID_ARCH_ABI}")

#aux_source_directory()
file(GLOB src_files *.cpp)

add_library(
        hawkeyeplayer
        SHARED
        ${src_files})

target_link_libraries( # Specifies the target library.
        hawkeyeplayer
        -Wl,--start-group
        avformat
        avcodec
        avfilter
        avutil
        swresample
        swscale
        -Wl,--end-group
        z
        log
        android
        OpenSLES
        )