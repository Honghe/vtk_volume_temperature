//
// Created by honhe on 11/3/16.
//

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include "TpsRenderer.h"
#include <vtkConeSource.h>

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
    pCamera->SetPosition(0, 0, -350);
    double *position = pCamera->GetPosition();
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
    pCamera->Elevation(30);
    pCamera->Azimuth(-110);
    //
    addFpsCameraPoly();
}

void TpsRenderer::addFpsCameraPoly() {
    CameraConeSource = vtkSmartPointer<vtkConeSource>::New();
    CameraConeSource->SetRadius(5);
    CameraConeSource->SetHeight(11);
    // set Direction to follow FPS Camera
    CameraConeSource->SetDirection(fpsRenderer->renderer->GetActiveCamera()->GetDirectionOfProjection());
    CameraConeSource->Update();

    vtkSmartPointer<vtkPolyData> cone = CameraConeSource->GetOutput();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(cone);

    fpsCameraActor = vtkSmartPointer<vtkActor>::New();
    fpsCameraActor->SetMapper(mapper);

    renderer->AddActor(fpsCameraActor);
    fpsCameraActor->SetPosition(fpsRenderer->renderer->GetActiveCamera()->GetPosition());
}

void TpsRenderer::updateTpsCamera() {
    // Update Direction
    CameraConeSource->SetDirection(fpsRenderer->renderer->GetActiveCamera()->GetDirectionOfProjection());
    CameraConeSource->Update();
    fpsCameraActor->SetPosition(fpsRenderer->renderer->GetActiveCamera()->GetPosition());
}

