project(buddy_pool)
cmake_minimum_required(VERSION 3.16)
get_filename_component(CURRENT_SOURCE_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(LIB_NAME ${CURRENT_SOURCE_DIR_NAME})

# force static runtime libraries for msvc builds
if(MSVC)
  set(variables 
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(variable ${variables})
	if(${variable} MATCHES "/MD")
	  string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
	endif()
  endforeach()
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE TEST_SOURCE_FILES test/*.cpp)


include_directories(${CMAKE_SOURCE_DIR}/include/)

add_library(${LIB_NAME} STATIC ${SOURCE_FILES})
set_property(TARGET ${LIB_NAME} PROPERTY FOLDER ${PROJ_NAME})


if(TEST_SOURCE_FILES)
    find_package(GTest)

    if (LINUX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --static")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --static") 
    endif(LINUX)

    set(LIBTEST_NAME ${LIB_NAME}_test)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_executable(${LIBTEST_NAME} ${TEST_SOURCE_FILES})
    target_link_libraries(${LIBTEST_NAME} GTest::GTest GTest::Main ${LIB_NAME})
    set_property(TARGET ${LIBTEST_NAME} PROPERTY FOLDER "test")
    gtest_add_tests(${LIBTEST_NAME} "" AUTO)
endif(TEST_SOURCE_FILES)
