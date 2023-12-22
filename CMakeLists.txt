cmake_minimum_required(VERSION 3.10...3.22)
project(SpatialTree LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Zi")
    set(FREETYPE "")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(FREETYPE "freetype")
endif()

find_package(Boost 1.83.0 REQUIRED)
find_package(SFML 2.6.1 REQUIRED COMPONENTS graphics window system)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB ASSETS "assets/*")
file(COPY ${ASSETS} DESTINATION ${CMAKE_BINARY_DIR}/assets)

foreach(ASSET ${ASSETS})
  get_filename_component(ASSET_NAME ${ASSET} NAME)
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/assets/${ASSET_NAME}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSET}" "${CMAKE_BINARY_DIR}/assets/${ASSET_NAME}"
    DEPENDS "${ASSET}"
  )
endforeach()

add_executable(app ${SOURCES} ${ASSETS})
source_group("Assets" FILES ${ASSETS})

target_include_directories(app PRIVATE src)
target_link_libraries(app ${Boost_LIBRARIES} sfml-graphics sfml-window sfml-system ${FREETYPE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
    target_compile_options(app PRIVATE /O2)
    else()
    target_compile_options(app PRIVATE -O3)
    endif()
else() # Release
    if(MSVC)
        target_compile_options(app PRIVATE /Zi)
        target_link_options(app PRIVATE /DEBUG)
    else()
        target_compile_options(app PRIVATE -O3 -g -pg)
    endif()
endif()
