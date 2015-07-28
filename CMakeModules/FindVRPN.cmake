include(FindPackageHandleStandardArgs)

# Search paths for CCSR
set(CMAKE_LIBRARY_PATH /home/kuhl/public-ogl/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /home/kuhl/public-ogl/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-ogl/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /home/kuhl/public-ogl/vrpn/quat ${CMAKE_INCLUDE_PATH})
# Search paths for IVS
set(CMAKE_LIBRARY_PATH /research/kuhl/public-ogl/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /research/kuhl/public-ogl/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-ogl/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /research/kuhl/public-ogl/vrpn/quat ${CMAKE_INCLUDE_PATH})
# Search path for Rekhi lab
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-ogl/vrpn/build ${CMAKE_LIBRARY_PATH})
set(CMAKE_LIBRARY_PATH /home/campus11/kuhl/public-ogl/vrpn/build/quat ${CMAKE_LIBRARY_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-ogl/vrpn/ ${CMAKE_INCLUDE_PATH})
set(CMAKE_INCLUDE_PATH /home/campus11/kuhl/public-ogl/vrpn/quat ${CMAKE_INCLUDE_PATH})



if(${CMAKE_HOST_UNIX})
    find_library(VRPN_LIBRARY_VRPN vrpn)
    find_library(VRPN_LIBRARY_QUAT quat)
    set(VRPN_LIBRARIES ${VRPN_LIBRARY_VRPN} ${VRPN_LIBRARY_QUAT})

    find_path(VRPN_INCLUDE_VRPN vrpn_Tracker.h)
    find_path(VRPN_INCLUDE_QUAT quat.h)
    set(VRPN_INCLUDE_DIRS ${VRPN_INCLUDE_VRPN} ${VRPN_INCLUDE_QUAT})
    
    find_package(Threads)
    set(VRPN_LIBRARIES ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    
endif()

if(VRPN_SOURCE)
find_package_handle_standard_args(VRPN DEFAULT_MSG
    VRPN_INCLUDE_DIRS
    VRPN_SOURCE
)
else()
find_package_handle_standard_args(VRPN DEFAULT_MSG
    VRPN_LIBRARY_VRPN VRPN_LIBRARY_QUAT VRPN_LIBRARIES 
    Threads_FOUND
    VRPN_INCLUDE_DIRS VRPN_INCLUDE_VRPN VRPN_INCLUDE_QUAT
)
endif()
