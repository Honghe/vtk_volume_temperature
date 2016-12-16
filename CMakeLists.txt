cmake_minimum_required(VERSION 3.6)
project(demo_vtk_Tutorial)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# REQUIRED 自动找需要的
find_package(VTK REQUIRED
        )
find_package(Boost REQUIRED
        COMPONENTS system filesystem

        )
include(${VTK_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})

set(SOURCE_FILES
        vr_temperature_1103.cpp
        tps/TpsRenderer.cpp
        fps/FpsRenderer.cpp
        fps/MouseInteractorStyle.cpp
        fps/BasePsRenderer.cpp
        fps/TimerCallback.cpp
        jet256colormap.cpp
        fps/CameraEventCallback.cpp
        MyDirector.cpp
        difference/DifferenceRenderer.cpp
        fps/WindTimerCallback.cpp
        fps/WindTimerCallback.cpp
        fps/RenderEndEventCallback.cpp
        fps/WindActorWrapper.cpp
        )

set(name "vr_temperature_1103")
add_executable(${name}
        ${SOURCE_FILES})
target_link_libraries(${name} ${VTK_LIBRARIES})
target_link_libraries(${name} ${Boost_LIBRARIES})



