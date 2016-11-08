//
// Created by honhe on 11/3/16.
//

#include "MyDirector.h"

MyDirector::MyDirector() {

}

void MyDirector::init() {
    renderWin = vtkSmartPointer<vtkRenderWindow>::New();
    renderInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderInteractor->SetRenderWindow(renderWin);
    renderWin->SetSize(1000, 800);

    std::string fileBaseDir("/home/honhe/Downloads/volume_render/temperature_data/2016-11-02");
    // make output dir
    std::string screenShotDirBase("/home/honhe/Downloads/volume_render/temperature_output/");
    std::string screenShotDir = screenShotDirBase + "/" + getDate() + "/";
    bool batch = false;
    double leftViewport[4] = {0.0, 0.5, 0.5, 1.0};
    double leftViewport2[4] = {0.0, 0.0, 0.5, 0.5};
    double rightViewport[4] = {0.5, 0.5, 1.0, 1.0};
    //
    fpsRenderer = new FpsRenderer(renderWin, renderInteractor, this);
    fpsRenderer->setViewPort(leftViewport);
    // TpsRenderer
    // set after fpsRender
    tpsRenderer = new TpsRenderer(renderWin, renderInteractor, this);
    tpsRenderer->setViewPort(rightViewport);
    // Temperature Difference Renderer
    differenceRenderer = new DifferenceRenderer(renderWin, renderInteractor, this);
    differenceRenderer->setViewPort(leftViewport2);
    //*
    fpsRenderer->init(fileBaseDir, screenShotDir);
    fpsRenderer->addScalarBarWidget();
    fpsRenderer->addFileNameTextWidget();
    fpsRenderer->addTemperatureTextWidget();
    fpsRenderer->listTemperatureFiles();
    fpsRenderer->addOrientationMarkerWidget();
    fpsRenderer->addCameraEventCallback();
    fpsRenderer->initVolumeDataMemory();
    fpsRenderer->setCamera();
    fpsRenderer->addGrid();
    fpsRenderer->prepareVolume();
    fpsRenderer->addVolumePicker();
    // do anything after Initialize, otherwise will throw error
    renderInteractor->Initialize();

    fpsRenderer->readFile(fpsRenderer->fileNames[0].string());

    batch = true;
    if (batch) {
        mkdir(screenShotDir.c_str(), 0744);
        fpsRenderer->setTimeEventObserver(100);
    }
}

void MyDirector::startInteractor() {
    fpsRenderer->render();
    renderInteractor->Start();
}

void MyDirector::fpsRendererInit(FpsRenderer *fpsRenderer) {
    tpsRenderer->init(fpsRenderer);
    differenceRenderer->init(fpsRenderer);
}

void MyDirector::fpsRendererPrepareVolume() {
    tpsRenderer->prepareVolume();
    differenceRenderer->prepareVolume();
}

void MyDirector::fpsRendererAddGrid() {
    tpsRenderer->addGrid();
    differenceRenderer->addGrid();
}

void MyDirector::fpsRendererReadFile() {
    tpsRenderer->updateImgData();
    differenceRenderer->updateImgData();
}

void MyDirector::fpsRendererRender() {
    tpsRenderer->render();
    differenceRenderer->render();
}

void MyDirector::fpsRendererAddOrientationMarkerWidget() {
    tpsRenderer->addOrientationMarkerWidget();
    differenceRenderer->addOrientationMarkerWidget();
}

void MyDirector::fpsRendererInitVolumeDataMemory() {
    tpsRenderer->initVolumeDataMemory();
    differenceRenderer->initVolumeDataMemory();
}

void MyDirector::fpsRendererAddScalarBarWidget() {
    tpsRenderer->addScalarBarWidget();
    differenceRenderer->addScalarBarWidget();
}

void MyDirector::fpsRendererSetCamera() {
    tpsRenderer->setCamera();
    differenceRenderer->setCamera();
}

void MyDirector::fpsRendererCameraUpdateEvent() {
    tpsRenderer->updateTpsCamera();
}

void MyDirector::fpsRendererAddFileNameTextWidget() {
    differenceRenderer->addTitleTextWidget();
}
