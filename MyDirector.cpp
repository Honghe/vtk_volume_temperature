//
// Created by honhe on 11/3/16.
//

#include "MyDirector.h"

MyDirector::MyDirector() {

}

void MyDirector::init() {
    bool batch = false;     // true 依次读取文件夹中的文件逐帧渲染，false 只显示第一帧的文件数据
    show4Viewport = false;  // 是否显示4个 Viewport, false 只显示主 Viewport
    // 温度源数据的文件目录
    std::string fileBaseDir("/home/honhe/ClionProjects/rj_volume_temperature/example_data");
    // 截屏的输出目录
    std::string screenShotDirBase("/home/honhe/Downloads/volume_render/temperature_output/");
    std::string screenShotDir = screenShotDirBase + "/" + getDate() + "/";

    renderWin = vtkSmartPointer<vtkRenderWindow>::New();
    renderInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderInteractor->SetRenderWindow(renderWin);
    renderWin->SetSize(1100, 800);
    // do anything after Initialize, otherwise will throw error
    renderInteractor->Initialize();

    double leftViewport[4] = {0.0, 0.5, 0.5, 1.0};
    double leftViewport2[4] = {0.0, 0.0, 0.5, 0.5};
    double rightViewport[4] = {0.5, 0.5, 1.0, 1.0};
    if (!show4Viewport) {
        leftViewport[0] = 0.0;
        leftViewport[1] = 0.0;
        leftViewport[2] = 1.0;
        leftViewport[3] = 1.0;
    }
    //
    fpsRenderer = new FpsRenderer(renderWin, renderInteractor, this);
    fpsRenderer->setViewPort(leftViewport);
    // TpsRenderer
    // set after fpsRender
    if (show4Viewport) {
        tpsRenderer = new TpsRenderer(renderWin, renderInteractor, this);
        tpsRenderer->setViewPort(rightViewport);
        // Temperature Difference Renderer
        differenceRenderer = new DifferenceRenderer(renderWin, renderInteractor, this);
        differenceRenderer->setViewPort(leftViewport2);
    }
    //*
    fpsRenderer->init(fileBaseDir, screenShotDir);
    fpsRenderer->addScalarBarWidget();
    fpsRenderer->addFileNameTextWidget();
    fpsRenderer->addTemperatureTextWidget();
    fpsRenderer->addPickPositionTextWidget();
    fpsRenderer->listTemperatureFiles();
    fpsRenderer->addOrientationMarkerWidget();
    fpsRenderer->addCameraEventCallback();
    fpsRenderer->initVolumeDataMemory();
    fpsRenderer->setCamera();
    fpsRenderer->addGrid();
    fpsRenderer->addGridWall();
    fpsRenderer->prepareVolume();
    fpsRenderer->addVolumePicker();
    fpsRenderer->addRenderEndEventCallback();
    //
    fpsRenderer->addWindFlow();

    fpsRenderer->readFile(fpsRenderer->fileNames[0].string());

    if (batch) {
        mkdir(screenShotDir.c_str(), 0744); // screenshot 图片保存目录
        fpsRenderer->setTimeEventObserver(100);
    }
}

void MyDirector::startInteractor() {
    fpsRenderer->render();
    renderInteractor->Start();
}

void MyDirector::fpsRendererInit(FpsRenderer *fpsRenderer) {
    if (show4Viewport) {
        tpsRenderer->init(fpsRenderer);
        differenceRenderer->init(fpsRenderer);
    }
}

void MyDirector::fpsRendererPrepareVolume() {
    if (show4Viewport) {
        tpsRenderer->prepareVolume();
        differenceRenderer->prepareVolume();
    }
}

void MyDirector::fpsRendererAddGrid() {
    if (show4Viewport) {
        tpsRenderer->addGrid();
        differenceRenderer->addGrid();
    }
}

void MyDirector::fpsRendererReadFile() {
    if (show4Viewport) {
        tpsRenderer->updateImgData();
        differenceRenderer->updateImgData();
    }
}

void MyDirector::fpsRendererRender() {
    if (show4Viewport) {
        tpsRenderer->render();
        differenceRenderer->render();
    }
}

void MyDirector::fpsRendererAddOrientationMarkerWidget() {
    if (show4Viewport) {
        tpsRenderer->addOrientationMarkerWidget();
        differenceRenderer->addOrientationMarkerWidget();
    }
}

void MyDirector::fpsRendererInitVolumeDataMemory() {
    if (show4Viewport) {
        tpsRenderer->initVolumeDataMemory();
        differenceRenderer->initVolumeDataMemory();
    }
}

void MyDirector::fpsRendererAddScalarBarWidget() {
    if (show4Viewport) {
        tpsRenderer->addScalarBarWidget();
        differenceRenderer->addScalarBarWidget();
    }
}

void MyDirector::fpsRendererSetCamera() {
    if (show4Viewport) {
        tpsRenderer->setCamera();
        differenceRenderer->setCamera();
    }
}

void MyDirector::fpsRendererCameraUpdateEvent() {
    if (show4Viewport) {
        tpsRenderer->updateTpsCamera();
    }
}

void MyDirector::fpsRendererAddFileNameTextWidget() {
    if (show4Viewport) {
        differenceRenderer->addTitleTextWidget();
    }
}
