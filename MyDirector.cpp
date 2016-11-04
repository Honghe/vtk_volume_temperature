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
    renderWin->SetSize(800, 400);

    std::string fileBaseDir("/home/honhe/Downloads/volume_render/temperature_data/2016-11-02");
    // make output dir
    std::string screenShotDirBase("/home/honhe/Downloads/volume_render/temperature_output/");
    std::string screenShotDir = screenShotDirBase + "/" + getDate() + "/";
    bool batch = false;
    double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
    double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};
    //
    fpsRenderer = new FpsRenderer(renderWin, renderInteractor, this);
    fpsRenderer->setViewPort(leftViewport);
    // TpsRenderer
    // set after fpsRender
    tpsRenderer = new TpsRenderer(renderWin, renderInteractor, this);
    tpsRenderer->setViewPort(rightViewport);
    //*
    fpsRenderer->init(fileBaseDir, screenShotDir);
    fpsRenderer->addScalarBarWidget();
    fpsRenderer->addFileNameTextWidget();
    fpsRenderer->addTemperatureTextWidget();
    fpsRenderer->listTemperatureFiles();
    fpsRenderer->addOrientationMarkerWidget();
    fpsRenderer->setCamera();
    fpsRenderer->initVolumeDataMemory();
    fpsRenderer->addGrid();
    fpsRenderer->prepareVolume();
    fpsRenderer->addVolumePicker();
    fpsRenderer->readFile(fpsRenderer->fileNames[0].string());

    //
//    batch = true;
    if (batch) {
        mkdir(screenShotDir.c_str(), 0744);
        fpsRenderer->setTimeEventObserver();
    }
}

void MyDirector::startInteractor() {
    renderInteractor->Initialize();
    fpsRenderer->render();
    renderInteractor->Start();
}

void MyDirector::fpsRenderUpdate() {
}

void MyDirector::fpsRendererInit(FpsRenderer *fpsRenderer) {
    tpsRenderer->init(fpsRenderer);
}

void MyDirector::fpsRendererPrepareVolume() {
    tpsRenderer->prepareVolume();
}

void MyDirector::fpsRendererAddGrid() {
    tpsRenderer->addGrid();
}

void MyDirector::fpsRendererReadFile() {
    tpsRenderer->updateImgData();
}

void MyDirector::fpsRendererRender() {
    tpsRenderer->render();
}

void MyDirector::fpsRendererAddOrientationMarkerWidget() {
    tpsRenderer->addOrientationMarkerWidget();
}

void MyDirector::fpsRendererInitVolumeDataMemory() {
    tpsRenderer->initVolumeDataMemory();
}

void MyDirector::fpsRendererAddScalarBarWidget() {
    tpsRenderer->addScalarBarWidget();
}

void MyDirector::fpsRendererSetCamera() {
    tpsRenderer->setCamera();
}
