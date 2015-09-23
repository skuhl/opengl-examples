if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(FATAL_ERROR "Don't run cmake here. Run it in the root folder of this repository.")
endif()
cmake_minimum_required(VERSION 2.6)

if(VRPN_FOUND)
    add_executable(fake-server fake-server.cpp)
    target_link_libraries(fake-server kuhl ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(fake-server kuhl)
    
    add_executable(recorder recorder.c)
    target_link_libraries(recorder kuhl ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(recorder kuhl)
endif()
