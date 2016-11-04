//
// Created by honhe on 11/3/16.
//

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include "TpsRenderer.h"

TpsRenderer::TpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                         vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor, MyDirector *myDirector)
        : BasePsRenderer(renderWin, renderInteractor, myDirector) {
}

void TpsRenderer::init(FpsRenderer *fpsRenderer) {
    this->fpsRenderer = fpsRenderer;
    myLookTable = fpsRenderer->myLookTable;
    colorNumber = fpsRenderer->colorNumber;
    rgbaLength = fpsRenderer->rgbaLength;
    data_axis_x = fpsRenderer->data_axis_x;
    data_axis_z = fpsRenderer->data_axis_z;
    data_axis_y = fpsRenderer->data_axis_y;
    temperature_min = fpsRenderer->temperature_min;
    temperature_max = fpsRenderer->temperature_max;
}

void TpsRenderer::initVolumeDataMemory() {
    this->imgData = fpsRenderer->imgData;
}

void TpsRenderer::updateImgData() {
    this->imgData = fpsRenderer->imgData;
}

void TpsRenderer::prepareVolume() {
    BasePsRenderer::prepareVolume();
}

void TpsRenderer::setCamera() {
    vtkCamera *pCamera = renderer->GetActiveCamera();
    pCamera->SetFocalPoint(data_axis_x / 2, data_axis_y / 2, data_axis_z / 2);
    pCamera->SetViewUp(0, 1, 0);
//        pCamera->SetPosition(data_axis_x * 3, data_axis_y * 3, -data_axis_z * 2);
    pCamera->SetPosition(0, 0, -250);
    double *position = pCamera->GetPosition();
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
    pCamera->Elevation(30);
    pCamera->Azimuth(-110);
}

