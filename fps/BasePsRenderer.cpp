//
// Created by honhe on 11/3/16.
//

#include <boost/multi_array.hpp>
#include "BasePsRenderer.h"
#include "TimerCallback.h"
#include <MyDirector.h>

using namespace boost::filesystem;

BasePsRenderer::BasePsRenderer() {
}

BasePsRenderer::BasePsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                               vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor, MyDirector *myDirector) {
    this->renderWin = renderWin;
    this->myDirector = myDirector;
    this->renderInteractor = renderInteractor;
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.1, 0.1, 0.1);
    renderWin->AddRenderer(renderer);
    volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
}

void BasePsRenderer::init(std::string fileBaseDir, std::string screenShotDir) {
    this->fileBaseDir = fileBaseDir;
    this->screenShotDir = screenShotDir;
    colorNumber = 256;
    rgbaLength = 4;
    data_axis_x = 80;
    data_axis_z = 99;
    data_axis_y = 36;
    temperature_min = 18;
    temperature_max = 30;
    vector<vector<double>> p((unsigned long) colorNumber, vector<double>(4));
    myLookTable = p;
//    temperature_min = 20;
//    temperature_max = 30.5;
}

void BasePsRenderer::initVolumeDataMemory() {
}

void BasePsRenderer::refreshRender() {
//        cout << "imgData GetMTime(): " << imgData->GetMTime()<< endl;
    renderWin->Render();
}

void BasePsRenderer::screenShot(int number) {
    const vtkSmartPointer<vtkWindowToImageFilter> &imageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    imageFilter->SetInput(renderWin);
    imageFilter->SetInputBufferTypeToRGBA();
    imageFilter->ReadFrontBufferOff();
    imageFilter->Update();

    const vtkSmartPointer<vtkPNGWriter> &pngWriter = vtkSmartPointer<vtkPNGWriter>::New();
    std::string fileName = boost::str((boost::format("%04d") % number));
    pngWriter->SetFileName((this->screenShotDir + "/" + fileName + ".png").c_str());
    pngWriter->SetInputConnection(imageFilter->GetOutputPort());
    pngWriter->Write();
}

void BasePsRenderer::listTemperatureFiles() {
}

void BasePsRenderer::readFile(std::string fileName) {
}

void BasePsRenderer::addScalarBarWidget() {
    const vtkSmartPointer<vtkScalarBarActor> scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBarActor->SetMaximumWidthInPixels(72);
    scalarBarActor->SetOrientationToHorizontal();
    scalarBarActor->UnconstrainedFontSizeOn();
    vtkTextProperty *pProperty = scalarBarActor->GetLabelTextProperty();
    pProperty->SetFontSize(16);
    pProperty->SetColor(1, 1, 1);
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetRange(temperature_min, temperature_max);
    lookupTable->SetRampToLinear();
    lookupTable->SetNumberOfTableValues(colorNumber);
    lookupTable->Build();

    // 使用 jet 256 配色
    for (int i = 0; i < colorNumber; ++i) {
        vector<double> &row = myLookTable[i];
        float *tableValue = Jet256ColorMap::colormap[i];
        for (int j = 0; j < rgbaLength; ++j) {
            row[j] = tableValue[j];
        }
//            printf("%d %03.2f %03.2f %03.2f\n", i, row[0], row[1], row[2]);
    }

    for (int k = 0; k < colorNumber; ++k) {
        vector<double> &row = myLookTable[k];
        lookupTable->SetTableValue(k, row[0], row[1], row[2], row[3]);
    }

    scalarBarActor->SetLookupTable(lookupTable);
    scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
    scalarBarWidget->SetInteractor(renderInteractor);
    scalarBarWidget->SetCurrentRenderer(renderer);
    scalarBarWidget->SetScalarBarActor(scalarBarActor);
    vtkScalarBarRepresentation *pRepresentation = scalarBarWidget->GetScalarBarRepresentation();
    pRepresentation->SetPosition(0.9, 0.053796);
    pRepresentation->SetShowBorderToOff();
    scalarBarWidget->On();
}

void BasePsRenderer::addGrid() {
    float padding = 0.5;
    const vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (float k : vector<float>{0 + padding, data_axis_z - 1 - padding}) {
        for (float j : vector<float>{0 + padding, data_axis_y - 1 - padding}) {
            for (float i  : vector<float>{0 + padding, data_axis_x - 1 - padding}) {
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

std::string BasePsRenderer::scalarToTemperature(int scalar) {
    float tmp = (float) scalar / (colorNumber - 1) * (temperature_max - temperature_min) + temperature_min;
    // set precision
    std::stringstream stream;
    stream << fixed << setprecision(1) << tmp;
    std::string s = stream.str();
    return s;
}

int BasePsRenderer::temperatureNormalize(float t) {
    t = t > temperature_max ? temperature_max : t;
    t = t < temperature_min ? temperature_min : t;
    return (t - temperature_min) / (temperature_max - temperature_min) * (colorNumber - 1);
}

void BasePsRenderer::updateTemperatureTextWidget(std::string tmp) {
    std::string header = "TMP\n";
    if (temperatureTextActor != nullptr) {
        temperatureTextActor->SetInput(header.append(tmp + "℃").c_str());
    }
}

void BasePsRenderer::addTemperatureTextWidget() {
}

void BasePsRenderer::addFileNameTextWidget() {

}


void BasePsRenderer::prepareVolume() {
    const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i = 2; i < colorNumber; ++i) {
        vector<double> &rgba = myLookTable[i];
        colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
    }
    const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    alphaChannelFunction->AddPoint(0, 0.00);
//        alphaChannelFunction->AddPoint(1, 0.4);     // for floor
//        alphaChannelFunction->AddPoint(2, 0.4);
//        alphaChannelFunction->AddPoint(50, 0.08);
//        alphaChannelFunction->AddPoint(60, 0.05);
//        alphaChannelFunction->AddPoint(110, 0.03);
//        alphaChannelFunction->AddPoint(150, 0.03);
//        alphaChannelFunction->AddPoint(200, 0.05);
//        alphaChannelFunction->AddPoint(210, 0.4);
//        alphaChannelFunction->AddPoint(255, 0.5);
    alphaChannelFunction->AddPoint(0, 0.03);
    alphaChannelFunction->AddPoint(255, 0.03);

    const vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(alphaChannelFunction);
    volumeMapper->SetInputData(imgData);
    volumeMapper->SetSampleDistance(1);
    volumeMapper->SetImageSampleDistance(1);
    volumeMapper->AutoAdjustSampleDistancesOff();   // for better display when rotate
    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);
    renderer->AddVolume(volume);
}

void BasePsRenderer::addOrientationMarkerWidget() {
    const vtkSmartPointer<vtkAxesActor> &axesActor = vtkSmartPointer<vtkAxesActor>::New();
    orientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientationMarkerWidget->SetOutlineColor(0.93, 0.57, 0.13);
    orientationMarkerWidget->SetOrientationMarker(axesActor);
    orientationMarkerWidget->SetInteractor(renderInteractor);
    orientationMarkerWidget->SetCurrentRenderer(renderer);
    orientationMarkerWidget->SetViewport(0, 0, 0.2, 0.2);;
    orientationMarkerWidget->SetEnabled(1);
    orientationMarkerWidget->SetInteractive(1);
}

void BasePsRenderer::setViewPort(double *_args) {
    renderer->SetViewport(_args);
}

int BasePsRenderer::positionNormalize(float scalar) {
    return (scalar - 5) / 10;
}

void BasePsRenderer::render() {
    renderWin->Render();
}
