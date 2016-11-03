//
// Created by honhe on 11/3/16.
//

#include <boost/multi_array.hpp>
#include "FpsRenderer.h"
#include "TimerCallback.h"
#include <MyDirector.h>

using namespace boost::filesystem;

FpsRenderer::FpsRenderer() {
}

FpsRenderer::FpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                         vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor, MyDirector *myDirector) {

    data_axis_x = 80;
    data_axis_z = 99;
    data_axis_y = 36;
    temperature_min = 18;
    temperature_max = 30;
    this->renderWin = renderWin;
    this->myDirector = myDirector;
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWin->AddRenderer(renderer);
    this->renderInteractor = renderInteractor;
    volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
    colorNumber = 256;
    rgbaLength = 4;
}

void FpsRenderer::init(std::string fileBaseDir, std::string screenShotDir) {
    this->fileBaseDir = fileBaseDir;
    this->screenShotDir = screenShotDir;
    vector<vector<double>> p((unsigned long) colorNumber, vector<double>(4));
    myLookTable = p;
    renderer->SetBackground(0.1, 0.1, 0.1);
}

void FpsRenderer::initVolumeDataMemory() {
    imgData = vtkSmartPointer<vtkStructuredPoints>::New();
    imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
    imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
}

void FpsRenderer::refreshRender() {
//        cout << "imgData GetMTime(): " << imgData->GetMTime()<< endl;
    renderWin->Render();
}

void FpsRenderer::render() {
    renderWin->Render();
    this->myDirector->fpsRendererPrepareVolume(this);
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
    cout << "origin max min " << originMax[0] << " " << originMin[0] << endl;

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
}

void FpsRenderer::addScalarBarWidget() {
    const vtkSmartPointer<vtkScalarBarActor> scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBarActor->SetMaximumWidthInPixels(72);
    scalarBarActor->SetOrientationToHorizontal();
    scalarBarActor->UnconstrainedFontSizeOn();
    vtkTextProperty *pProperty = scalarBarActor->GetLabelTextProperty();
    pProperty->SetFontSize(16);
    pProperty->SetColor(1, 1, 1);
    const vtkSmartPointer<vtkLookupTable> &lookupTable = vtkSmartPointer<vtkLookupTable>::New();
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
    scalarBarWidget->SetScalarBarActor(scalarBarActor);
    vtkScalarBarRepresentation *pRepresentation = scalarBarWidget->GetScalarBarRepresentation();
    pRepresentation->SetPosition(0.9, 0.053796);
    pRepresentation->SetShowBorderToOff();
    scalarBarWidget->On();
}

void FpsRenderer::addGrid() {
    float padding = 0.5;
    const vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int k : vector<int>{0 + padding, data_axis_z - padding}) {
        for (int j : vector<int>{0 + padding, data_axis_y - padding}) {
            for (int i  : vector<int>{0 + padding, data_axis_x - padding}) {
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

    myDirector->fpsRendereraddGrid(this);
}

void FpsRenderer::setTimeEventObserver() {
    const vtkSmartPointer<TimerCallback> &timerCallback = vtkSmartPointer<TimerCallback>::New();
    timerCallback->init(this);
    renderInteractor->AddObserver(vtkCommand::TimerEvent, timerCallback);
    int timerId = renderInteractor->CreateRepeatingTimer(100);
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
    text_widget->SetInteractor(renderInteractor);
    text_widget->SetTextActor(text_actor);
    text_widget->SelectableOff();
    text_widget->GetBorderRepresentation()->SetShowBorderToOff();
    text_widget->On();
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
}

void FpsRenderer::prepareVolume() {
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

void FpsRenderer::addOrientationMarkerWidget() {
    const vtkSmartPointer<vtkAxesActor> &axesActor = vtkSmartPointer<vtkAxesActor>::New();
    orientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientationMarkerWidget->SetOutlineColor(0.93, 0.57, 0.13);
    orientationMarkerWidget->SetOrientationMarker(axesActor);
    orientationMarkerWidget->SetInteractor(renderInteractor);
    orientationMarkerWidget->SetViewport(0, 0, 0.2, 0.2);;
    orientationMarkerWidget->SetEnabled(1);
    orientationMarkerWidget->SetInteractive(1);
}

void FpsRenderer::setViewPort(double *_args) {
    renderer->SetViewport(_args);
}

int FpsRenderer::positionNormalize(float scalar) {
    return (scalar - 5) / 10;
}
