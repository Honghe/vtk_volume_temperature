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

void DifferenceRenderer::initVolumeDataMemory() {
    imgData = vtkSmartPointer<vtkStructuredPoints>::New();
    imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
    imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
}

void DifferenceRenderer::updateImgData() {
    // 使用 vtkUnsignedCharArray，与 imgData 中的类型一样，不然操作数据不正确
    vtkUnsignedCharArray *aa = vtkUnsignedCharArray::New();
    aa->DeepCopy(fpsRenderer->imgData->GetPointData()->GetScalars());
    imgDataArrayVector.push_back(aa);
    //
    int *dims = imgData->GetDimensions();
    int min = 255;
    int max = 0;
    for (int k = 0; k < dims[2]; k++) {
        for (int j = 0; j < dims[1]; j++) {
            for (int i = 0; i < dims[0]; i++) {
                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
                pixel[0] = calTemperatureDifference(i, j, k);
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

unsigned char DifferenceRenderer::calTemperatureDifference(int i, int j, int k) {
    int result = 0;
    if (imgDataArrayVector.size() < 2) {
        return rand() % 20;
    }
    for (int l = 1; l < imgDataArrayVector.size(); ++l) {
        // 使用 imgData 这个类的内置方法
        int idx[3];
        idx[0] = i;
        idx[1] = j;
        idx[2] = k;
        // 目前先用简单方法，取绝对值
        result += abs(((static_cast<unsigned char *>(imgData->GetArrayPointer(imgDataArrayVector[l], idx)))[0] -
                       (static_cast<unsigned char *>(imgData->GetArrayPointer(imgDataArrayVector[l - 1], idx)))[0]));
    }
//    result += 200;
    if (result < 0 || result > 255) {
        cout << "temperature difference < 0 || > 255\n";
//        throw MyException("temperature difference < 0 || > 255");
    }
    if (result < 0) {
        result = 0;
    }
    if(result > 255) {
        result = 255;
    }
    return (unsigned char) result;
}


void DifferenceRenderer::prepareVolume() {
//    BasePsRenderer::prepareVolume();

    const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i = 2; i < colorNumber; ++i) {
        vector<double> &rgba = myLookTable[i];
        colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
    }
    const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    alphaChannelFunction->AddPoint(0, 0.0);
    alphaChannelFunction->AddPoint(255, 0.04);

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
