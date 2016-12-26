#rj_volume_temperature

## 代码目录结构

    ├── CMakeLists.txt
    ├── difference                          // 显示温度差的Viewport
    │   ├── DifferenceRenderer.cpp
    │   └── DifferenceRenderer.h
    ├── fps                                 // 显示温度变化的主Viewport
    │   ├── BasePsRenderer.cpp
    │   ├── BasePsRenderer.h
    │   ├── CameraEventCallback.cpp
    │   ├── CameraEventCallback.h
    │   ├── FpsRenderer.cpp
    │   ├── FpsRenderer.h
    │   ├── MouseInteractorStyle.cpp
    │   ├── MouseInteractorStyle.h
    │   ├── RenderEndEventCallback.cpp
    │   ├── RenderEndEventCallback.h
    │   ├── TimerCallback.cpp
    │   ├── TimerCallback.h
    │   ├── WindActorWrapper.cpp
    │   ├── WindActorWrapper.h
    │   ├── WindTimerCallback.cpp
    │   └── WindTimerCallback.h
    ├── jet256colormap.cpp                  // Jet256配色文件
    ├── jet256colormap.h
    ├── MyDirector.cpp                      // 导演类
    ├── MyDirector.h                                
    ├── README.md
    ├── tps                                 // 第三视角的Viewport
    │   ├── TpsRenderer.cpp
    │   └── TpsRenderer.h
    └── vr_temperature_1103.cpp             // 入口
    └── example_data                        // 示例数据的目录


## 分支说明
1. step1 分支是比较早的Project所有代码在单个文件中.
2. master 工作分支.
