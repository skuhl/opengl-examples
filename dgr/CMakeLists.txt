if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository.")
endif()
cmake_minimum_required(VERSION 2.6)


if(Threads_FOUND)
	add_executable(dgr-relay dgr-relay.cpp)
	target_link_libraries(dgr-relay ${CMAKE_THREAD_LIBS_INIT})
else()
	message(WARNING "Not compiling dgr-relay because pthreads was not found on this system.")
endif()
