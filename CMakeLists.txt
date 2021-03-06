cmake_minimum_required(VERSION 2.8)

cmake_policy(SET CMP0072 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0079 NEW)

project(PicoRender)

option (USE_MSVC_RUNTIME_LIBRARY_DLL OFF)


if(MSVC)
    set(COMMON_OPTIONS /W3 /wd4996 /we4239 /we4002 /we4700 /we4305 /EHsc /MP)
    set(DEBUG_OPTIONS /MTd /Od /Zi)
    set(RELEASE_OPTIONS /MT /Ox /fp:fast /GL /GR-)
else()
	set(DEBUG_OPTIONS -g -Wall)
	set(RELEASE_OPTIONS -g -Ofast -fno-rtti -flto -fPIC -funsafe-math-optimizations -fno-strict-aliasing  -ffast-math -msse2 -msse3 -msse4)
endif()

add_compile_options(
        "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>"
        "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")


configure_file(teapot.obj teapot.obj COPYONLY)
configure_file(wood.tga wood.tga COPYONLY)
configure_file(config.txt config.txt  COPYONLY)
configure_file(config_post.txt config_post.txt  COPYONLY)

file(GLOB SOURCES *.cpp  *.c)

add_executable(PicoRender ${SOURCES})

add_executable(PostProcess PostProcess/PostProcess.cpp ConfigReader.cpp fileio.cpp)

if(MSVC)
else()
	target_link_libraries(PicoRender PRIVATE pthread)
	target_link_options(PicoRender PRIVATE -static-libstdc++ -flto)
endif()
