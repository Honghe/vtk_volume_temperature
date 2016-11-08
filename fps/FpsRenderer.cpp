//
// Created by honhe on 11/3/16.
//

#include <boost/multi_array.hpp>
#include "FpsRenderer.h"
#include "TimerCallback.h"
#include "CameraEventCallback.h"
#include <MyDirector.h>

using namespace boost::filesystem;

FpsRenderer::FpsRenderer() {
}

FpsRenderer::FpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                         vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor, MyDirector *myDirector)
        : BasePsRenderer(renderWin, renderInteractor, myDirector) {
}

void FpsRenderer::init(std::string fileBaseDir, std::string screenShotDir) {
    BasePsRenderer::init(fileBaseDir, screenShotDir);
    myDirector->fpsRendererInit(this);
}

void FpsRenderer::initVolumeDataMemory() {
    imgData = vtkSmartPointer<vtkStructuredPoints>::New();
    imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
    imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    myDirector->fpsRendererInitVolumeDataMemory();
}

void FpsRenderer::refreshRender() {
    BasePsRenderer::refreshRender();
}

void FpsRenderer::render() {
    BasePsRenderer::render();
    this->myDirector->fpsRendererRender();
}

void FpsRenderer::screenShot(int number) {
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

void FpsRenderer::listTemperatureFiles() {
    path p(fileBaseDir);
    vector<path> fileNames;

    if (is_directory(p)) {
        copy(directory_iterator(p), directory_iterator(), back_inserter(fileNames));
        sort(fileNames.begin(), fileNames.end());
//            for (vector<path>::const_iterator it(fileNames.begin()), it_end(fileNames.end()); it != it_end; ++it) {
//                cout << "   " << *it << '\n';
//            }
        this->fileNames = fileNames;
    }
}

void FpsRenderer::readFile(std::string fileName) {
    std::string subName = fileName.substr(fileName.length() - 23, 19);
    cout << "read file " << subName << endl;

    text_actor->SetInput(subName.c_str());

    FILE *f = fopen(fileName.c_str(), "r");
    // pass 2 lines
    const size_t line_size = 300;
    char *line = (char *) malloc(line_size);
    fgets(line, line_size, f);
    fgets(line, line_size, f);
    //
    float x, y, z, t;
    int volume_size = data_axis_x * data_axis_y * data_axis_z;

    typedef boost::multi_array<float, 3> array_type;
    typedef array_type::index index;
    typedef boost::array<index, 3> tIndexArray;
    tIndexArray idx;

    boost::multi_array<float, 3> xyzs(boost::extents[data_axis_x][data_axis_z][data_axis_y]);

    for (int i = 0; i < volume_size; ++i) {
        fscanf(f, "%f %f %f %f", &x, &z, &y, &t);
//            printf("%d %f %f %f %f\n", i, x, z, y, t);
        int reverse_x_index = data_axis_x - 1 - positionNormalize(x);
        idx = {{reverse_x_index, positionNormalize(z), positionNormalize(y)}};
        xyzs(idx) = t;
    }
    fclose(f);

    float *originMax = std::max_element(xyzs.origin(), xyzs.origin() + xyzs.num_elements());
    float *originMin = std::min_element(xyzs.origin(), xyzs.origin() + xyzs.num_elements());
    cout << "origin min max" << originMin[0] << " " << originMax[0] << endl;

    int *dims = imgData->GetDimensions();
    for (int k = 0; k < dims[2]; k++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int i = 0; i < dims[0]; i++) {
                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
                idx = {{i, k, j}};
                pixel[0] = (unsigned char) temperatureNormalize(xyzs(idx));
            }
        }
    }
    // update MTime for pipeline can refresh.
    imgData->Modified();
    myDirector->fpsRendererReadFile();
}

void FpsRenderer::addScalarBarWidget() {
    BasePsRenderer::addScalarBarWidget();
    myDirector->fpsRendererAddScalarBarWidget();
}

void FpsRenderer::addGrid() {
    BasePsRenderer::addGrid();
    myDirector->fpsRendererAddGrid();
}

void FpsRenderer::setTimeEventObserver(int interval) {
    const vtkSmartPointer<TimerCallback> &timerCallback = vtkSmartPointer<TimerCallback>::New();
    timerCallback->init(this);
    renderInteractor->AddObserver(vtkCommand::TimerEvent, timerCallback);
    int timerId = renderInteractor->CreateRepeatingTimer(interval);
    std::cout << "timerId: " << timerId << std::endl;
}

std::string FpsRenderer::scalarToTemperature(int scalar) {
    float tmp = (float) scalar / (colorNumber - 1) * (temperature_max - temperature_min) + temperature_min;
    // set precision
    std::stringstream stream;
    stream << fixed << setprecision(1) << tmp;
    std::string s = stream.str();
    return s;
}

int FpsRenderer::temperatureNormalize(float t) {
    t = t > temperature_max ? temperature_max : t;
    t = t < temperature_min ? temperature_min : t;
    return (t - temperature_min) / (temperature_max - temperature_min) * (colorNumber - 1);
}

void FpsRenderer::addVolumePicker() {
    // Set the custom stype to use for interaction.
    vtkSmartPointer<MouseInteractorStyle> style =
            vtkSmartPointer<MouseInteractorStyle>::New();
    style->init(this);
    style->Data = imgData;
    renderInteractor->SetInteractorStyle(style);
}

void FpsRenderer::updateTemperatureTextWidget(std::string tmp) {
    std::string header = "TMP\n";
    if (temperatureTextActor != nullptr) {
        temperatureTextActor->SetInput(header.append(tmp + "℃").c_str());
    }
}

void FpsRenderer::addTemperatureTextWidget() {
    temperatureTextActor = vtkSmartPointer<vtkTextActor>::New();
    temperatureTextActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
    temperatureTextActor->GetTextProperty()->SetFontFile(
            "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf");
    temperatureTextActor->GetTextProperty()->SetColor(0.9, .9, 0.9);
    temperatureTextActor->GetTextProperty()->SetFontSize(14);
    temperatureTextActor->GetTextProperty()->SetJustificationToCentered();
    temperatureTextActor->GetTextProperty()->SetVerticalJustificationToCentered();
    const vtkSmartPointer<vtkTextRepresentation> &text_representation = vtkSmartPointer<vtkTextRepresentation>::New();
    text_representation->GetPositionCoordinate()->SetValue(0.05, 0.92);
    text_representation->GetPosition2Coordinate()->SetValue(0.06, 0.06);
    temperatureTextWidget = vtkSmartPointer<vtkTextWidget>::New();
    temperatureTextWidget->SetRepresentation(text_representation);
    temperatureTextWidget->SetCurrentRenderer(renderer);
    temperatureTextWidget->SetInteractor(renderInteractor);
    temperatureTextWidget->SetTextActor(temperatureTextActor);
    temperatureTextWidget->SelectableOff();
    temperatureTextWidget->GetBorderRepresentation()->SetShowBorderToOff();
    temperatureTextWidget->On();
}

void FpsRenderer::addFileNameTextWidget() {
    text_actor = vtkSmartPointer<vtkTextActor>::New();
    text_actor->SetInput("");
    text_actor->GetTextProperty()->SetColor(0.9, .9, 0.9);
    const vtkSmartPointer<vtkTextRepresentation> &text_representation = vtkSmartPointer<vtkTextRepresentation>::New();
    text_representation->GetPositionCoordinate()->SetValue(0.3, 0.85);
    text_representation->GetPosition2Coordinate()->SetValue(0.3, 0.2);
    text_widget = vtkSmartPointer<vtkTextWidget>::New();
    text_widget->SetRepresentation(text_representation);
    text_widget->SetCurrentRenderer(renderer);
    text_widget->SetInteractor(renderInteractor);
    text_widget->SetTextActor(text_actor);
    text_widget->SelectableOff();
    text_widget->GetBorderRepresentation()->SetShowBorderToOff();
    text_widget->On();
    myDirector->fpsRendererAddFileNameTextWidget();
}

void FpsRenderer::setCamera() {
    vtkCamera *pCamera = renderer->GetActiveCamera();
//        vtkVolumeCollection *pCollection = renderer->GetVolumes();
//        vtkVolume *pVolume = (vtkVolume *) pCollection->GetItemAsObject(0);
//        pCamera->SetFocalPoint(pVolume->GetCenter());
    pCamera->SetFocalPoint(data_axis_x / 2, data_axis_y / 2, data_axis_z / 2);
    pCamera->SetViewUp(0, 1, 0);
//        pCamera->SetPosition(data_axis_x * 3, data_axis_y * 3, -data_axis_z * 2);
    pCamera->SetPosition(0, 0, -200);
    double *position = pCamera->GetPosition();
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
    pCamera->Elevation(30);
    pCamera->Azimuth(-40);
    myDirector->fpsRendererSetCamera();
}

void FpsRenderer::prepareVolume() {
    BasePsRenderer::prepareVolume();
    this->myDirector->fpsRendererPrepareVolume();
}

int FpsRenderer::positionNormalize(float scalar) {
    return (scalar - 5) / 10;
}

void FpsRenderer::addOrientationMarkerWidget() {
    BasePsRenderer::addOrientationMarkerWidget();
    myDirector->fpsRendererAddOrientationMarkerWidget();
}

void FpsRenderer::addCameraEventCallback() {
    const vtkSmartPointer<CameraEventCallback> &callback = vtkSmartPointer<CameraEventCallback>::New();
    callback->init(this);
    renderer->AddObserver(vtkCommand::StartEvent, callback);
}
