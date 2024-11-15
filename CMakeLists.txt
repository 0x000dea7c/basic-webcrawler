cmake_minimum_required(VERSION 3.31)

project(basic-webcrawler)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Winline -Wuninitialized -Wshadow \
    -Wconversion -Wstrict-aliasing=2 -Wnon-virtual-dtor -Wnull-dereference \
    -Woverloaded-virtual -Wformat -Wfloat-equal -Weffc++ -Wstrict-overflow=5 -fno-rtti")

# Debug, Release compiler
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address,leak,undefined -ggdb -pg")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g -flto")

# Debug, Release, linker
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcurl")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--as-needed -Wl,-O1 -Wl,--strip-all")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--as-needed")

set(SEED_URL_1 "https://motherfuckingwebsite.com/")
set(SEED_URL_2 "https://www.fsme.es")
set(SEED_URL_3 "http://bettermotherfuckingwebsite.com/")
set(DEFAULT_REQUEST_DELAY 1)            # In seconds!
set(DEFAULT_DEPTH_LIMIT 2)
set(METADATA_FILENAME "crawler_output.txt")

configure_file(
  "${PROJECT_SOURCE_DIR}/inc/config.h.in"
  "${PROJECT_BINARY_DIR}/inc/config.hh"
)

file(GLOB WEBCRAWLER_SOURCES "src/basic-webcrawler/*.cc")

# Create a library for the core functionality
add_library(webcrawler_lib ${WEBCRAWLER_SOURCES})

# Include directories for the library
target_include_directories(webcrawler_lib PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../vcpkg/installed/x64-linux/include
    ${PROJECT_BINARY_DIR}/inc
)

find_package(CURL REQUIRED)
target_link_libraries(webcrawler_lib PRIVATE CURL::libcurl ${PROJECT_SOURCE_DIR}/../vcpkg/installed/x64-linux/lib/liblexbor_static.a)

# Create the main executable
add_executable(basic-webcrawler src/main.cc)

# Link the main executable to the library
target_link_libraries(basic-webcrawler PRIVATE webcrawler_lib)

# Include directories for the main executable
target_include_directories(basic-webcrawler PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../vcpkg/installed/x64-linux/include
    ${PROJECT_BINARY_DIR}/inc
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

enable_testing()

set(TEST_FILES
  test/test_http_parser.cc
  test/test_robots_parser.cc)

foreach(TEST_FILE ${TEST_FILES})
  get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_FILE})

  target_link_libraries(${TEST_NAME} PRIVATE webcrawler_lib)

  target_include_directories(${TEST_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../vcpkg/installed/x64-linux/include
    ${PROJECT_BINARY_DIR}/inc
  )

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

  set_tests_properties(${TEST_NAME} PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test)
endforeach()
