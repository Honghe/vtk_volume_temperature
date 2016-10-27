//
// Created by honhe on 10/27/16.
//

#include <iostream>
#include <vtkStructuredPointsReader.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkImageImport.h>
#include <vtkStructuredPoints.h>
#include <vtkInformation.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include "vtkBoxWidget.h"
#include "vtkCamera.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageResample.h"
#include "vtkProperty.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkXMLImageDataReader.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <vtkScalarBarActor.h>
#include <vtkLookupTable.h>
#include <vtkTextProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkScalarBarWidget.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkVolumeProperty.h>

using namespace std;
using namespace boost::filesystem;

class MyRenderer {
public:
    vector<vector<double>> myLookTable;

    MyRenderer() {
        data_axis_x = 80;
        data_axis_z = 99;
        data_axis_y = 36;
        temperature_min = 18;
        temperature_max = 30;
        fileBaseDir = "/home/honhe/Downloads/volume_render/temperature_data/idw_penalty-result_20161024/";
        renderer = vtkSmartPointer<vtkRenderer>::New();
        renderWin = vtkSmartPointer<vtkRenderWindow>::New();
        renderWin->AddRenderer(renderer);
        renderInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        renderInteractor->SetRenderWindow(renderWin);
        dataImporter = vtkSmartPointer<vtkImageImport>::New();
        volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
        colorNumber = 256;
        rgbaLength = 4;
    }

    void init() {
        vector<vector<double>> p(colorNumber, vector<double>(4));
        myLookTable = p;
        renderer->SetBackground(0.1, 0.1, 0.1);
        renderWin->SetSize(900, 800);
        renderInteractor->Initialize();
    }

    void render() {
        renderWin->Render();
        renderInteractor->Start();
    }

    void listTemperatureFiles() {
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

    void readFile(string fileName) {
        imgData = vtkSmartPointer<vtkStructuredPoints>::New();
        imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
        imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        int *dims = imgData->GetDimensions();
        int increaseI = 1;

        for (int z = 0; z < dims[2]; z++) {
            for (int y = 0; y < dims[1]; y++) {
                for (int x = 0; x < dims[0]; x++) {
                    unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(x, y, z));
                    pixel[0] = increaseI % 255;
                }
            }
            increaseI += 1;
        }
    }

    void addScalarBarWidget() {
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
        // 使用库内置好的颜色值，Build 再获取，不然是无效值
        for (int i = 0; i < colorNumber; ++i) {
            vector<double> &row = myLookTable[i];
            double *tableValue = lookupTable->GetTableValue(i);
            for (int j = 0; j < rgbaLength; ++j) {
                row[j] = tableValue[j];
            }
        }
        // 倒过来赋值
        for (int k = 0; k < colorNumber; ++k) {
            vector<double> &row = myLookTable[colorNumber - 1 - k];
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

    void prepareVolume() {
        const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        for (int i = 2; i < colorNumber; ++i) {
            vector<double> &rgba = myLookTable[colorNumber - i];
            colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
        }
        const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
        alphaChannelFunction->AddPoint(0, 0.00);
        alphaChannelFunction->AddPoint(1, 0.4);
        alphaChannelFunction->AddPoint(2, 0.4);
        alphaChannelFunction->AddPoint(50, 0.08);
        alphaChannelFunction->AddPoint(60, 0.05);
        alphaChannelFunction->AddPoint(110, 0.03);
        alphaChannelFunction->AddPoint(150, 0.03);
        alphaChannelFunction->AddPoint(200, 0.05);
        alphaChannelFunction->AddPoint(210, 0.4);
        alphaChannelFunction->AddPoint(255, 0.5);

        const vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
        volumeProperty->SetColor(colorTransferFunction);
        volumeProperty->SetScalarOpacity(alphaChannelFunction);
        volumeMapper->SetInputData(imgData);
        const vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
        volume->SetMapper(volumeMapper);
        volume->SetProperty(volumeProperty);
        renderer->AddVolume(volume);
    }

    virtual ~MyRenderer() {

    }

private:

    int temperature_max;
    int data_axis_x;
    int data_axis_z;
    int data_axis_y;
    int temperature_min;
    string fileBaseDir;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWin;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;
    vtkSmartPointer<vtkImageImport> dataImporter;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    vector<path> fileNames;
    vtkSmartPointer<vtkStructuredPoints> imgData;
    vtkIdType colorNumber;
    int rgbaLength;
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;
};

int main() {
    MyRenderer *myRenderer = new MyRenderer();
    myRenderer->init();
    myRenderer->addScalarBarWidget();
    myRenderer->listTemperatureFiles();
    myRenderer->readFile("");
    myRenderer->prepareVolume();
    myRenderer->render();
}