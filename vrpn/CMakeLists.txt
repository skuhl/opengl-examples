if(VRPN_FOUND)
    add_executable(fake-server fake-server.cpp)
    target_link_libraries(fake-server kuhl ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(fake-server kuhl)
    
    add_executable(recorder recorder.cpp)
    target_link_libraries(recorder kuhl ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(recorder kuhl)
endif()
