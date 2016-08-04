include(FindPackageHandleStandardArgs)

find_library(VRPN_LIBRARY_VRPN vrpn)
find_library(VRPN_LIBRARY_QUAT quat)
set(VRPN_LIBRARIES ${VRPN_LIBRARY_VRPN} ${VRPN_LIBRARY_QUAT})

find_path(VRPN_INCLUDE_VRPN vrpn_Tracker.h)
find_path(VRPN_INCLUDE_QUAT quat.h)
set(VRPN_INCLUDE_DIRS ${VRPN_INCLUDE_VRPN} ${VRPN_INCLUDE_QUAT})
    
find_package(Threads)
set(VRPN_LIBRARIES ${VRPN_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

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
