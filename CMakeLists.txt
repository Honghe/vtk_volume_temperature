cmake_minimum_required(VERSION 3.6)
project(demo_vtk_Tutorial)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# REQUIRED 自动找需要的
find_package(VTK REQUIRED
        )
find_package(Boost REQUIRED
        COMPONENTS  system filesystem

        )
include(${VTK_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES
        vr_temperature_1014.cpp
        vr_temperature_1014_by_ruilang.cpp)

foreach(name ${SOURCE_FILES})
    add_executable(${name}  ${name} )
    target_link_libraries(${name} ${VTK_LIBRARIES})
    target_link_libraries(${name} ${Boost_LIBRARIES})

endforeach()



