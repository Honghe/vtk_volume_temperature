//
// Created by honhe on 10/27/16.
//

#include <iostream>
#include <vtkTextActor.h>
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
#include<vtkPoints.h>
#include <vtkVolumePicker.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkDataSetMapper.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>

using namespace std;
using namespace boost::filesystem;

class MyRenderer {
public:
    vector<vector<double>> myLookTable;
    string fileBaseDir;
    vector<path> fileNames;

    MyRenderer() {
        data_axis_x = 80;
        data_axis_z = 99;
        data_axis_y = 36;
        temperature_min = 18;
        temperature_max = 30;
        renderer = vtkSmartPointer<vtkRenderer>::New();
        renderWin = vtkSmartPointer<vtkRenderWindow>::New();
        renderWin->AddRenderer(renderer);
        renderInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        renderInteractor->SetRenderWindow(renderWin);
        volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
        colorNumber = 256;
        rgbaLength = 4;
    }

    void init(string fileBaseDir) {
        this->fileBaseDir = fileBaseDir;
        vector<vector<double>> p((unsigned long) colorNumber, vector<double>(4));
        myLookTable = p;
        renderer->SetBackground(0.1, 0.1, 0.1);
        renderWin->SetSize(1000, 800);
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
        cout << "read file " << fileName << endl;

        string subName = fileName.substr(fileName.length() - 24, 20);
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
        vector<vector<vector<int>>> xyzs((unsigned long) data_axis_x,
                                         vector<vector<int>>((unsigned long) data_axis_z,
                                                             vector<int>((unsigned long) data_axis_y)));

        for (int i = 0; i < volume_size; ++i) {
            fscanf(f, "%f %f %f %f", &x, &z, &y, &t);
//            printf("%d %f %f %f %f\n", i, x, z, y, t);
            xyzs[positionNormalize(x)][positionNormalize(z)][positionNormalize(y)] =
                    temperatureNormalize(t);
        }
        fclose(f);
        imgData = vtkSmartPointer<vtkStructuredPoints>::New();
        imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
        imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
        int *dims = imgData->GetDimensions();

        for (int k = 0; k < dims[2]; k++) {
            for (int j = 0; j < dims[1]; j++) {
                for (int i = 0; i < dims[0]; i++) {
                    unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(i, j, k));
                    pixel[0] = (unsigned char) xyzs[i][k][j];
                }
            }
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
        // 使用库内配置好的颜色值，Build 再获取，不然是无效值
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

    void addGrid() {
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
    }

    void addOrientationMarkerWidget() {
        const vtkSmartPointer<vtkAxesActor> &axesActor = vtkSmartPointer<vtkAxesActor>::New();
        orientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
        orientationMarkerWidget->SetOutlineColor(0.93, 0.57, 0.13);
        orientationMarkerWidget->SetOrientationMarker(axesActor);
        orientationMarkerWidget->SetInteractor(renderInteractor);
        orientationMarkerWidget->SetViewport(0, 0, 0.2, 0.2);;
        orientationMarkerWidget->SetEnabled(1);
        orientationMarkerWidget->SetInteractive(1);
    }

    void prepareVolume() {
        const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        for (int i = 2; i < colorNumber; ++i) {
            vector<double> &rgba = myLookTable[colorNumber - i];
            colorTransferFunction->AddRGBPoint(i, rgba[0], rgba[1], rgba[2]);
        }
        const vtkSmartPointer<vtkPiecewiseFunction> alphaChannelFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
        alphaChannelFunction->AddPoint(0, 0.00);
        alphaChannelFunction->AddPoint(1, 0.4);     // for floor
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
        volumeMapper->SetSampleDistance(1);
        volumeMapper->SetImageSampleDistance(1);
        volumeMapper->AutoAdjustSampleDistancesOff();   // for better display when rotate
        const vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
        volume->SetMapper(volumeMapper);
        volume->SetProperty(volumeProperty);
        renderer->AddVolume(volume);
    }

    void setCamera() {
        vtkCamera *pCamera = renderer->GetActiveCamera();
        vtkVolumeCollection *pCollection = renderer->GetVolumes();
        vtkVolume *pVolume = (vtkVolume *) pCollection->GetItemAsObject(0);
        pCamera->SetFocalPoint(pVolume->GetCenter());
        pCamera->SetViewUp(0, 1, 0);
//        pCamera->SetPosition(data_axis_x * 3, data_axis_y * 3, -data_axis_z * 2);
        pCamera->SetPosition(0, 0, -200);
        double *position = pCamera->GetPosition();
        pCamera->Elevation(30);
        pCamera->Azimuth(-40);
    }

    void addTextWidget() {
        text_actor = vtkSmartPointer<vtkTextActor>::New();
        text_actor->SetInput("日期");
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

    virtual ~MyRenderer() {

    }

private:
    int temperature_max;
    int data_axis_x;
    int data_axis_z;
    int data_axis_y;
    int temperature_min;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWin;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    vtkSmartPointer<vtkStructuredPoints> imgData;
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget;
    vtkIdType colorNumber;
    int rgbaLength;
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;

    int positionNormalize(float scalar) {
        return (scalar - 5) / 10;
    }

    int temperatureNormalize(float t) {
        return (t - temperature_min) / (temperature_max - temperature_min) * 254 + 1;
    }

    vtkSmartPointer<vtkTextActor> text_actor;
    vtkSmartPointer<vtkTextWidget> text_widget;
};

int main() {
    string fileBaseDir = "/home/honhe/Downloads/volume_render/temperature_data/idw_penalty-result_20161024/";
    MyRenderer *myRenderer = new MyRenderer();
    myRenderer->init(fileBaseDir);
    myRenderer->addScalarBarWidget();
    myRenderer->addTextWidget();
    myRenderer->listTemperatureFiles();
    myRenderer->readFile(myRenderer->fileNames[0].string());
    myRenderer->prepareVolume();
    myRenderer->addGrid();
    myRenderer->addOrientationMarkerWidget();
    myRenderer->setCamera();
    myRenderer->render();
}