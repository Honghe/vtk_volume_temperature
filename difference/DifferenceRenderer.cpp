//
// Created by honhe on 11/7/16.
//

#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <vtkFloatArray.h>
#include <vtkCharArray.h>
#include "DifferenceRenderer.h"

struct MyException : public std::exception {
    std::string s;

    MyException(std::string ss) : s(ss) {}

    ~MyException() throw() {} // Updated
    const char *what() const throw() { return s.c_str(); }
};

void DifferenceRenderer::init(FpsRenderer *fpsRenderer) {
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

DifferenceRenderer::DifferenceRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                                       vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor,
                                       MyDirector *myDirector) : BasePsRenderer(renderWin, renderInteractor,
                                                                                myDirector) {
}

void DifferenceRenderer::setCamera() {
    vtkCamera *pCamera = renderer->GetActiveCamera();
    pCamera->SetFocalPoint(data_axis_x / 2, data_axis_y / 2, data_axis_z / 2);
    pCamera->SetViewUp(0, 1, 0);
//        pCamera->SetPosition(data_axis_x * 3, data_axis_y * 3, -data_axis_z * 2);
    pCamera->SetPosition(0, 0, -200);
    double *position = pCamera->GetPosition();
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
    pCamera->Elevation(30);
    pCamera->Azimuth(-40);
}

void DifferenceRenderer::addScalarBarWidget() {
//    BasePsRenderer::addScalarBarWidget();
    const vtkSmartPointer<vtkScalarBarActor> scalarBarActor = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBarActor->SetMaximumWidthInPixels(72);
    scalarBarActor->SetOrientationToHorizontal();
    scalarBarActor->UnconstrainedFontSizeOn();
    vtkTextProperty *pProperty = scalarBarActor->GetLabelTextProperty();
    pProperty->SetFontSize(16);
    pProperty->SetColor(1, 1, 1);
    lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    // change range, the min temperature is 0
    lookupTable->SetRange(0, temperature_max);
    lookupTable->SetRampToLinear();
    lookupTable->SetNumberOfTableValues(colorNumber);
    lookupTable->Build();

    // 使用自己的配色
//    for (int i = 0; i < colorNumber; ++i) {
//        vector<double> &row = myLookTable[i];
//        row[0] = ((float) colorNumber - 1 - i) / (colorNumber - 1) * ((colorNumber - 30 - 130.0) / (colorNumber - 30))
//                 + 130.0 / (colorNumber - 30);
//        row[1] = ((float) colorNumber - 1 - i) / (colorNumber - 1) * ((colorNumber - 45.0) / (colorNumber - 1));
//        row[2] = ((float) colorNumber - 1 - i) / (colorNumber - 1) * ((colorNumber - 30.0) / (colorNumber - 1));
//        row[3] = 1;
//        printf("%03d %03.2f %03.2f %03.2f\n", i, row[0], row[1], row[2]);
//    }

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

void DifferenceRenderer::initVolumeDataMemory() {
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
}

void DifferenceRenderer::updateImgData() {
    // 使用 vtkUnsignedCharArray，与 imgData 中的类型一样，不然操作数据不正确
    vtkUnsignedCharArray *aa = vtkUnsignedCharArray::New();
    aa->DeepCopy(fpsRenderer->imgData->GetPointData()->GetScalars());
    // TODO 2016.11.17 一直累加存储，内存溢出
    imgDataArrayVector.push_back(aa);

    unsigned long imgDataArrayLength = imgDataArrayVector.size();
    if (imgDataArrayLength < 2) {   // >=2个温度值才能计算温度差
        return;
    }
    //
    int *dims = imgData->GetDimensions();
    int min = 255;
    int max = 0;
    // 第次都从头计算温度差，数据多的话计算量过大。因此，用累加的方法：新温度差之和=原温度差之和+最近一帧温度差
    for (int k = 0; k < dims[2]; k++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int i = 0; i < dims[0]; i++) {
                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
                if (imgDataArrayLength <= 2) {  // 赋初值为0
                    pixel[0] = 0;
                }
                pixel[0] = accumulateTemperatureDifference(i, j, k, pixel[0]); // 累加
                min = min > pixel[0] ? pixel[0] : min;
                max = max > pixel[0] ? max : pixel[0];
            }
        }
    }

//    for (int k = 0; k < dims[2]; k++) {
//        for (int j = 0; j < dims[1]; j++) {
//            for (int i = 0; i < dims[0]; i++) {
//                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
//                cout << (unsigned int) pixel[0] << " ";
//            }
//            cout << endl;
//        }
//    }
    imgData->Modified();
    cout << "difference min max " << min << " " << max << endl;
}

unsigned char DifferenceRenderer::accumulateTemperatureDifference(int i, int j, int k, unsigned char tmpDelta) {
    int newTmpDelta = calTemperatureDifference(i,j,k) + tmpDelta;
    if (newTmpDelta < 0 || newTmpDelta > 255) {
//        cout << "temperature difference < 0 || > 255\n";
//        throw MyException("temperature difference < 0 || > 255");
    }
    if (newTmpDelta < 0) {
        newTmpDelta = 0;
    }
    if (newTmpDelta > colorNumber - 1) {
        newTmpDelta = colorNumber - 1;
    }
    return newTmpDelta;
}

unsigned char DifferenceRenderer::calTemperatureDifference(int i, int j, int k) {
    int result = 0;
    // 第次都从头计算温度差，数据多的话计算量过大。因此，用累加的方法：新温度差之和=原温度差之和+最近一帧温度差
    unsigned long imgDataArrayLength = imgDataArrayVector.size();
    for (int l = imgDataArrayLength - 1; l < imgDataArrayLength; ++l) {  // 只计算一帧
        // 使用 imgData 这个类的内置方法
        int idx[3];
        idx[0] = i;
        idx[1] = j;
        idx[2] = k;
        // 温度差目前先用简单方法，取绝对值
        result += abs(((static_cast<unsigned char *>(imgData->GetArrayPointer(imgDataArrayVector[l], idx)))[0] -
                       (static_cast<unsigned char *>(imgData->GetArrayPointer(imgDataArrayVector[l - 1], idx)))[0]));
        // TODO 2016.11.17 更好的规一化方法
        result *= 0.5;  // 减小幅度
    }
//    if (result < 0 || result > 255) {
////        cout << "temperature difference < 0 || > 255\n";
////        throw MyException("temperature difference < 0 || > 255");
//    }
//    if (result < 0) {
//        result = 0;
//    }
//    if (result > colorNumber - 1) {
//        result = colorNumber - 1;
//    }
    return (unsigned char) result;
}

void DifferenceRenderer::addTitleTextWidget() {
    text_actor = vtkSmartPointer<vtkTextActor>::New();
    text_actor->SetInput("空间各点随时间温度变化程度(℃)");
    text_actor->GetTextProperty()->SetFontFamily(VTK_FONT_FILE);
    text_actor->GetTextProperty()->SetFontFile(
            "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf");
    text_actor->GetTextProperty()->SetColor(0.9, .9, 0.9);
//    text_actor->GetTextProperty()->SetFontSize(4);
    text_widget = vtkSmartPointer<vtkTextWidget>::New();
    text_widget->SetCurrentRenderer(renderer);
    text_widget->SetInteractor(renderInteractor);
    text_widget->SetTextActor(text_actor);
    text_widget->SelectableOff();
    text_widget->GetBorderRepresentation()->SetShowBorderToOff();
    vtkTextRepresentation *pRepresentation = (vtkTextRepresentation *) text_widget->GetRepresentation();
    // Widget中 字体的大小好像是根据 Y 轴即其高度决定的
    pRepresentation->SetPosition(0.2, 0.9);     // PositionX 数值如何计算才居中 PositionX = (1-Position2X)/2
    pRepresentation->SetPosition2(0.6, 0.04);   // 如果文字是NON-ASCII,则文字会依据大小超出设置的最大宽度，为毛？
    text_widget->On();
}

void DifferenceRenderer::prepareVolume() {
//    BasePsRenderer::prepareVolume();
    const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i = 2; i < colorNumber; ++i) {
        vector<double> &rgba = myLookTable[i];
        colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
    }
    const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
//    alphaChannelFunction->AddPoint(0, 0.00);
    alphaChannelFunction->AddPoint(1, 0.01);
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

