//
// Created by honhe on 11/3/16.
//

#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include "TpsRenderer.h"

TpsRenderer::TpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                         vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor, MyDirector *myDirector) {
    this->renderWin = renderWin;
    this->myDirector = myDirector;
    this->renderInteractor = renderInteractor;
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWin->AddRenderer(renderer);
    renderer->SetBackground(0.1, 0.1, 0.1);
    volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
}

void TpsRenderer::setViewPort(double *_args) {
    renderer->SetViewport(_args);
}

void TpsRenderer::update(FpsRenderer *fpsRenderer) {

}

void TpsRenderer::init(FpsRenderer *fpsRenderer) {
    this->prepareVolume(fpsRenderer);
}

void TpsRenderer::addGrid(FpsRenderer *fpsRenderer) {
    float padding = 0.5;
    const vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int k : vector<int>{0 + padding, fpsRenderer->data_axis_z - padding}) {
        for (int j : vector<int>{0 + padding, fpsRenderer->data_axis_y - padding}) {
            for (int i  : vector<int>{0 + padding, fpsRenderer->data_axis_x - padding}) {
                points->InsertNextPoint(i, j, k);
            }
        }
    }

    const vtkSmartPointer<vtkCellArray> &lines = vtkSmartPointer<vtkCellArray>::New();
    for (int i: vector<int>{0, 4}) {
        vtkLine *line = vtkLine::New();
        line->GetPointIds()->SetId(0, 0 + i);
        line->GetPointIds()->SetId(1, 1 + i);
        lines->InsertNextCell(line);

        line = vtkLine::New();
        line->GetPointIds()->SetId(0, 2 + i);
        line->GetPointIds()->SetId(1, 3 + i);
        lines->InsertNextCell(line);

        line = vtkLine::New();
        line->GetPointIds()->SetId(0, 0 + i);
        line->GetPointIds()->SetId(1, 2 + i);
        lines->InsertNextCell(line);

        line = vtkLine::New();
        line->GetPointIds()->SetId(0, 1 + i);
        line->GetPointIds()->SetId(1, 3 + i);
        lines->InsertNextCell(line);
    }

    for (int l = 0; l < 4; ++l) {
        vtkLine *line = vtkLine::New();
        line->GetPointIds()->SetId(0, l);
        line->GetPointIds()->SetId(1, 4 + l);
        lines->InsertNextCell(line);
    }
    const vtkSmartPointer<vtkPolyData> &polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);
    const vtkSmartPointer<vtkDataSetMapper> &mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(polyData);
    const vtkSmartPointer<vtkActor> &actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    renderer->AddActor(actor);
}

void TpsRenderer::prepareVolume(FpsRenderer *fpsRenderer) {
    const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i = 2; i < fpsRenderer->colorNumber; ++i) {
        vector<double> &rgba = fpsRenderer->myLookTable[i];
        colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
    }
    const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    alphaChannelFunction->AddPoint(0, 0.00);
    alphaChannelFunction->AddPoint(0, 0.03);
    alphaChannelFunction->AddPoint(255, 0.03);

    const vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(alphaChannelFunction);
    volumeMapper->SetInputData(fpsRenderer->imgData);
    volumeMapper->SetSampleDistance(1);
    volumeMapper->SetImageSampleDistance(1);
    volumeMapper->AutoAdjustSampleDistancesOff();   // for better display when rotate
    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);
    renderer->AddVolume(volume);
}
