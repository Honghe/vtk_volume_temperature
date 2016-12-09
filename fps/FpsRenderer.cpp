//
// Created by honhe on 11/3/16.
//

#include <boost/multi_array.hpp>
#include "FpsRenderer.h"
#include "TimerCallback.h"
#include "CameraEventCallback.h"
#include "vtkPolyDataMapper.h"
#include "vtkTransform.h"
#include "WindTimerCallback.h"
#include "RenderEndEventCallback.h"
#include <MyDirector.h>
#include <vtkCubeSource.h>
#include <chrono>
#include <vtkAssembly.h>
#include <stdio.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>

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
    isAddWindFlow = false;
    statusBarPositionY1 = 0.0;
    statusBarPositionY2 = 0.03;
}

void FpsRenderer::initVolumeDataMemory() {
    imgData = vtkSmartPointer<vtkStructuredPoints>::New();
    imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
    imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

    // TODO 2016.11.08 若第一帧是纯色的话，Render 就不会更新后面的, 如何解决这个bug
    int *dims = imgData->GetDimensions();
    for (int k = 0; k < dims[2]; k++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int i = 0; i < dims[0]; i++) {
                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
                pixel[0] = rand() % 256;
            }
        }
    }
    //
    myDirector->fpsRendererInitVolumeDataMemory();
}

void FpsRenderer::refreshRender() {
    // 若已添加模拟空调，则刷新动作由模拟空调统一做
    if (isAddWindFlow) {
        return;
    } else {
        BasePsRenderer::refreshRender();
    }
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

void *FpsRenderer::_readFiel_helper(void *context) {
    FpsRenderer *thisContext = (FpsRenderer *) context;
    thisContext->_readFile(thisContext->lastFileName);
}

void FpsRenderer::readFile(std::string fileName) {
    this->lastFileName = fileName;
    pthread_t tid;
    int ret = pthread_create(&tid, NULL, &FpsRenderer::_readFiel_helper, this);
    if (ret != 0) {
        cout << "readFile pthread_create error: error_code=" << ret << endl;
    } else {
        cout << "new thread id: " << tid << endl;
    }
}

void *FpsRenderer::_readFile(std::string fileName) {
    std::string subName = fileName.substr(fileName.length() - 23, 19);
    cout << "read file " << subName << endl;
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

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
    cout << "origin min max: " << originMin[0] << " " << originMax[0] << endl;

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
    updateLastPickPointTemperature();
    myDirector->fpsRendererReadFile();

    chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
    cout << "readFile used ms: " << duration / 1000 << endl;
}

void FpsRenderer::updateLastPickPointTemperature() {
    if (isLastPickPoint) {
        updateTemperatureTextWidget(
                scalarToTemperature((static_cast<unsigned char *>(imgData->GetScalarPointer(lastPickPoint)))[0]));
    }
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
    std::cout << "read file timerId: " << timerId << std::endl;
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
    std::string header = "TMP: ";
    if (temperatureTextActor != nullptr) {
        temperatureTextActor->SetInput(header.append(tmp + "℃").c_str());
    }
}

void FpsRenderer::updatePickPositionWidget(float x, float y, float z) {
//    std::string positionString = boost::str((boost::format(" Pick X: %.2f, Y: %.2f, Z: %.2f") % x % y % z));
    // 传入的是实际长度的dm单位，转成cm整数显示
    x *= 10;
    y *= 10;
    z *= 10;
    std::string positionString = boost::str((boost::format(" Pos(cm) X: %d, Y: %d, Z: %d") % int(x) % int(y) % int(z)));
    if (pickPositionTextActor != nullptr) {
        pickPositionTextActor->SetInput(positionString.c_str());
    }
}

void FpsRenderer::setStatusBarTextActor(vtkSmartPointer<vtkTextActor> textActor,
                                        vtkSmartPointer<vtkTextWidget> textWidget,
                                        double positionX1, double positionX2) {
    textActor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
    textActor->GetTextProperty()->SetFontFile(
            "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf");
    textActor->GetTextProperty()->SetColor(0.9, .9, 0.9);
    textActor->GetTextProperty()->SetFontSize(14);

    textWidget->SetCurrentRenderer(renderer);
    textWidget->SetInteractor(renderInteractor);
    textWidget->SetTextActor(textActor);
    textWidget->SelectableOff();
    textWidget->GetBorderRepresentation()->SetShowBorderToOff();
    vtkTextRepresentation *pRepresentation = (vtkTextRepresentation *) textWidget->GetRepresentation();
    pRepresentation->SetPosition(positionX1, statusBarPositionY1);
    pRepresentation->SetPosition2(positionX2, statusBarPositionY2);

    // TextActor 的 align 要在 vtkTextWidget 之后设置，不然会莫名被重置
//    textActor->GetScaledTextProperty()->SetJustificationToLeft();
//    textActor->GetScaledTextProperty()->SetVerticalJustificationToCentered();
    textActor->GetTextProperty()->SetJustificationToLeft();
    textActor->GetTextProperty()->SetVerticalJustificationToCentered();
    textWidget->On();
}

void FpsRenderer::addPickPositionTextWidget() {
    pickPositionTextActor = vtkSmartPointer<vtkTextActor>::New();
    pickPositionTextWidget = vtkSmartPointer<vtkTextWidget>::New();
    vtkTextRepresentation *temperatureRepresentation = (vtkTextRepresentation *) temperatureTextWidget->GetRepresentation();
    setStatusBarTextActor(pickPositionTextActor, pickPositionTextWidget,
                          temperatureRepresentation->GetPosition()[0] + temperatureRepresentation->GetPosition2()[0],
                          0.40);
}

void FpsRenderer::addTemperatureTextWidget() {
    temperatureTextActor = vtkSmartPointer<vtkTextActor>::New();
    temperatureTextWidget = vtkSmartPointer<vtkTextWidget>::New();
    // 宽度设置不够大，导致在初始时文字即被拉伸，则后续窗口大小变化时文字的拉伸很奇怪，因此初始宽度要适当
    setStatusBarTextActor(temperatureTextActor, temperatureTextWidget, 0.05, 0.15);
}

void FpsRenderer::addFileNameTextWidget() {
    text_actor = vtkSmartPointer<vtkTextActor>::New();
    // TODO 2016.11.17 使用DroidSansFallbackFull字体（未确认其它非ASCII字体）时，更新文字在某些窗口大小下文字会大小闪烁
//    text_actor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
//    text_actor->GetTextProperty()->SetFontFile(
//            "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf");
    text_actor->GetTextProperty()->SetColor(0.9, .9, 0.9);
    text_widget = vtkSmartPointer<vtkTextWidget>::New();
    text_widget->SetCurrentRenderer(renderer);
    text_widget->SetInteractor(renderInteractor);
    text_widget->SetTextActor(text_actor);
    text_widget->SelectableOff();
    text_widget->GetBorderRepresentation()->SetShowBorderToOff();
    vtkTextRepresentation *pRepresentation = (vtkTextRepresentation *) text_widget->GetRepresentation();
    pRepresentation->SetPosition(0.3, 0.9);
    pRepresentation->SetPosition2(0.4, 0.05);
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
    pCamera->SetPosition(0, 0, -170);
    double *position = pCamera->GetPosition();
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
    pCamera->Elevation(40);
    pCamera->Azimuth(-58);
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

void FpsRenderer::addRenderEndEventCallback() {
    const vtkSmartPointer<RenderEndEventCallback> &callback = vtkSmartPointer<RenderEndEventCallback>::New();
    callback->init(this);
    renderer->AddObserver(vtkCommand::EndEvent, callback);
}

void FpsRenderer::addWindFlow() {
    if (isAddWindFlow) {
        return;
    } else {
        isAddWindFlow = true;
    }

    // create Wind flow
    int wind_axis_x = 20;
    int wind_axis_y = 20;
    int wind_axis_z = 20;
    int windcolorNumber = wind_axis_x;

    const vtkSmartPointer<vtkPolyData> &polyData = vtkSmartPointer<vtkPolyData>::New();
    const vtkSmartPointer<vtkPoints> &points = vtkSmartPointer<vtkPoints>::New();
    const vtkSmartPointer<vtkCellArray> &vertices = vtkSmartPointer<vtkCellArray>::New();
    const vtkSmartPointer<vtkUnsignedCharArray> &scalars = vtkSmartPointer<vtkUnsignedCharArray>::New();

    points->SetDataTypeToFloat();
    points->Reset();

    scalars->SetName("Scalar");
    for (int i = 0; i < wind_axis_x; ++i) {
        for (int j = 0; j < wind_axis_y; ++j) {
            for (int k = 0; k < wind_axis_z; ++k) {
                double x = i - rand() % 20 * 0.05;
                // 主要初始给个随机，随后可以不用偏移，如何随后随机偏移的话看过去动态比较厉害
                double y = j - (rand() % 20 - 10) * 0.05;
                double z = k - (rand() % 20 - 10) * 0.05;
                points->InsertNextPoint(x, y, z);
                // scalar 用作颜色渲染
                scalars->InsertNextValue(i);
            }
        }
    }

    for (vtkIdType j = 0; j < (vtkIdType) points->GetNumberOfPoints(); ++j) {
        // 只需要一个Cell就可以，所以点都属于此Cell
        vertices->InsertNextCell(1);
        vertices->InsertCellPoint(j);
    }
    polyData->SetPoints(points);
    polyData->SetVerts(vertices);
    polyData->GetPointData()->SetScalars(scalars);
    polyData->Modified();

    vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);
    mapper->SetScalarRange(0, windcolorNumber - 1);

    const vtkSmartPointer<vtkLookupTable> &lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfTableValues(windcolorNumber);
    lut->Build();
    for (int l = 0; l < windcolorNumber; ++l) {
        double *pDouble = lut->GetTableValue(l);
        // 离出风口越远，透明度越小
        lut->SetTableValue(l, pDouble[0], pDouble[1], pDouble[2], float(l) / (windcolorNumber * 1) + 0.05);
    }

    mapper->SetLookupTable(lut);
    mapper->SetColorModeToMapScalars();
    mapper->SetScalarModeToUsePointData();

    const vtkSmartPointer<vtkProperty> actorProperty = vtkSmartPointer<vtkProperty>::New();

    windActor = vtkSmartPointer<vtkActor>::New();
    windActor->SetMapper(mapper);
    windActor->SetProperty(actorProperty);
    windActor->GetProperty()->SetPointSize(2);

    vtkSmartPointer<WindTimerCallback> timerCallback = vtkSmartPointer<WindTimerCallback>::New();
    timerCallback->Setwind_axis_x(wind_axis_x);
    timerCallback->Setwind_axis_y(wind_axis_y);
    timerCallback->Setwind_axis_z(wind_axis_z);

    timerCallback->init(renderWin, points, scalars);
    renderInteractor->AddObserver(vtkCommand::TimerEvent, timerCallback);
    int interval = 30;
    int timerId = renderInteractor->CreateRepeatingTimer(interval);
    std::cout << "addWindFlow timerId: " << timerId << std::endl;

    // Create a AirCondition cube.
    vtkSmartPointer<vtkCubeSource> cubeSource =
            vtkSmartPointer<vtkCubeSource>::New();
    cubeSource->SetXLength(3);
    cubeSource->SetYLength(20);
    cubeSource->SetZLength(20);
    // 使其生成的数据(0,0,0)与坐标世界坐标0重合
    cubeSource->SetCenter(wind_axis_x, cubeSource->GetYLength() / 2.0,
                          cubeSource->GetZLength() / 2.0);
    // Create a mapper and airConditionActor.
    vtkSmartPointer<vtkPolyDataMapper> airConditionmapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
    airConditionmapper->SetInputConnection(cubeSource->GetOutputPort());

    vtkSmartPointer<vtkActor> airConditionActor =
            vtkSmartPointer<vtkActor>::New();
    airConditionActor->SetMapper(airConditionmapper);
    airConditionActor->GetProperty()->SetOpacity(0.99);

    // Combine the sphere and cube into an assembly
    // if hierarchy is deep may render slow
    vtkSmartPointer<vtkAssembly> airConditionAssembly =
            vtkSmartPointer<vtkAssembly>::New();
    airConditionAssembly->AddPart(windActor);
    airConditionAssembly->AddPart(airConditionActor);

    const vtkSmartPointer<vtkTransform> &transForm = vtkSmartPointer<vtkTransform>::New();
    transForm->Translate(60, 20, 30);
    transForm->RotateZ(90);
    airConditionAssembly->SetUserTransform(transForm);

    renderer->AddActor(airConditionAssembly);
}

void FpsRenderer::addGridWall() {
    int data_axis_x = 80;
    int data_axis_y = 36;
    int data_axis_z = 99;
    int thickNess = 4;      // 墙的厚度
    int floorExtend = 20;   // 地板的延伸长度
    int data_axis_x_delta = 2;  // X轴方向的上偏移，使墙角不重叠
    int door_height = 22;
    int door_width = 16;
    int door_x_delta = 20;

    std::vector<vtkAlgorithmOutput *> polys;
// Combine the sphere and cube into an ssembly
    wallPolyAssembly = vtkSmartPointer<vtkAssembly>::New();

    //  四面墙的结构图
    //                       这边开个小洞作为门
    //                      ++
    //  (0.0)      wall 1   |
    //  +--> +--------------v ----+
    //       |                    |
    //       |                    |
    //       |                    |
    //       |                    |
    //       |                    |
    // wall2 |                    | wall4
    //       |                    |
    //       |                    |
    //       |                    |
    //       |                    |
    //       |                    |
    //       +--------------------+
    //                wall3

    // 一号墙上开个门，所以分为3个cube组合
    const vtkSmartPointer<vtkCubeSource> &cubeSource1_1 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource1_1->SetXLength(door_x_delta);
    cubeSource1_1->SetYLength(data_axis_y);
    cubeSource1_1->SetZLength(thickNess);
    cubeSource1_1->SetCenter(data_axis_x - cubeSource1_1->GetXLength() / 2.0 - thickNess / 2.0, 0, 0);
    cubeSource1_1->Update();

    const vtkSmartPointer<vtkCubeSource> &cubeSource1_2 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource1_2->SetXLength(data_axis_x - door_x_delta - door_width);
    cubeSource1_2->SetYLength(data_axis_y);
    cubeSource1_2->SetZLength(thickNess);
    cubeSource1_2->SetCenter(cubeSource1_2->GetXLength() / 2.0, 0, 0);
    cubeSource1_2->Update();

    const vtkSmartPointer<vtkCubeSource> &cubeSource1_3 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource1_3->SetXLength(door_width - data_axis_x_delta);
    cubeSource1_3->SetYLength(data_axis_y - door_height);
    cubeSource1_3->SetZLength(thickNess);
    cubeSource1_3->SetCenter(cubeSource1_2->GetXLength() + cubeSource1_3->GetXLength() / 2.0,
                             -(data_axis_y / 2.0 - door_height - cubeSource1_3->GetYLength() / 2.0), 0);
    cubeSource1_3->Update();
    // end 一号墙上开个门，所以分为3个cube组合

    const vtkSmartPointer<vtkCubeSource> &cubeSource2 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource2->SetXLength(thickNess);
    cubeSource2->SetYLength(data_axis_y);
    cubeSource2->SetZLength(data_axis_z + thickNess);
    cubeSource2->SetCenter(-data_axis_x_delta, 0, data_axis_z / 2.0);
    cubeSource2->Update();

    const vtkSmartPointer<vtkCubeSource> &cubeSource3 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource3->SetXLength(data_axis_x - data_axis_x_delta);
    cubeSource3->SetYLength(data_axis_y);
    cubeSource3->SetZLength(thickNess);
    cubeSource3->SetCenter(cubeSource3->GetXLength() / 2.0, 0, data_axis_z);
    cubeSource3->Update();

    const vtkSmartPointer<vtkCubeSource> &cubeSource4 = vtkSmartPointer<vtkCubeSource>::New();
    cubeSource4->SetXLength(thickNess);
    cubeSource4->SetYLength(data_axis_y);
    cubeSource4->SetZLength(data_axis_z + thickNess);
    cubeSource4->SetCenter(data_axis_x, 0, data_axis_z / 2.0);
    cubeSource4->Update();

    const vtkSmartPointer<vtkCubeSource> &cubeSourceFloor = vtkSmartPointer<vtkCubeSource>::New();
    cubeSourceFloor->SetXLength(data_axis_x + floorExtend);
    cubeSourceFloor->SetYLength(thickNess);
    cubeSourceFloor->SetZLength(data_axis_z + floorExtend);
    cubeSourceFloor->SetCenter(data_axis_x / 2.0, -data_axis_y / 2.0, data_axis_z / 2.0);
    cubeSourceFloor->Update();

    // 组合1号墙
    vtkSmartPointer<vtkAppendPolyData> wall1Poly =
            vtkSmartPointer<vtkAppendPolyData>::New();
    wall1Poly->AddInputData(cubeSource1_1->GetOutput());
    wall1Poly->AddInputData(cubeSource1_2->GetOutput());
    wall1Poly->AddInputData(cubeSource1_3->GetOutput());
    wall1Poly->Update();

    // Remove any duplicate points.
    // 加上这个后会变暗，同时多了奇怪的分隔条
//    vtkSmartPointer<vtkCleanPolyData> cleanFilter =
//            vtkSmartPointer<vtkCleanPolyData>::New();
//    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
//    cleanFilter->Update();

    polys.push_back(wall1Poly->GetOutputPort());
    polys.push_back(cubeSource2->GetOutputPort());
    polys.push_back(cubeSource3->GetOutputPort());
    polys.push_back(cubeSource4->GetOutputPort());
    polys.push_back(cubeSourceFloor->GetOutputPort());

    for (auto &&item : polys) {
        vtkSmartPointer<vtkPolyDataMapper> mapper =
                vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(item);
        mapper->ScalarVisibilityOff();
        vtkSmartPointer<vtkActor> actor =
                vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        wallPolyAssembly->AddPart(actor);
    }

    const vtkSmartPointer<vtkTransform> &transform = vtkSmartPointer<vtkTransform>::New();
    transform->Translate(0, data_axis_y / 2.0, 0);
    wallPolyAssembly->SetUserTransform(transform);

    renderer->AddActor(wallPolyAssembly);
}

// 自定义排序方法
//template<typename double, typename int>
bool comparePairs(const std::pair<double, int> &lhs, const std::pair<double, int> &rhs) {
    return lhs.first <= rhs.first;
}

void FpsRenderer::updateWall() {
    vtkCamera *pCamera = renderer->GetActiveCamera();
    double *cameraPosition;
    cameraPosition = pCamera->GetPosition();
//     遍历计算各墙与Camera的距离
//     Extract each actor from the assembly and change its opacity
    std::vector<double> distances;
    vtkSmartPointer<vtkPropCollection> collection =
            vtkSmartPointer<vtkPropCollection>::New();
    wallPolyAssembly->GetActors(collection);
    collection->InitTraversal();
    std::vector<vtkActor *> actors;
    // 就4个墙进行变化，地板不变化
    for (vtkIdType i = 0; i < 4; i++) {
        double *position;
        vtkActor *pActor = vtkActor::SafeDownCast(collection->GetNextProp());
        actors.push_back(pActor);
        position = pActor->GetCenter();
        distances.push_back(sqrt(pow(position[0] - cameraPosition[0], 2.0) +
                                 pow(position[1] - cameraPosition[1], 2.0)) +
                            pow(position[2] - cameraPosition[2], 2.0));
    }
    // 距离排序
    typedef std::pair<double, int> mypair;
    std::vector<mypair> distancesPair;
    for (int j = 0; j < distances.size(); ++j) {
        mypair p;
        p.first = distances[j];
        p.second = j;
        distancesPair.push_back(p);
    }
    std::sort(distancesPair.begin(), distancesPair.end(), comparePairs);
//     [<5,0>, <8,1>, <7,2>] >> 逆[<5,0>, <7,2>, <8,1>]
    for (int k = 0; k < distancesPair.size(); ++k) {
        vtkActor *pActor = actors[distancesPair[k].second];
        // 写死只设置前两个为低透明
        if (k <= 1) {
            pActor->GetProperty()->SetOpacity(0.3);
        } else {
            pActor->GetProperty()->SetOpacity(1);
        }
    }
    cout << "camera event done." << endl;
}
