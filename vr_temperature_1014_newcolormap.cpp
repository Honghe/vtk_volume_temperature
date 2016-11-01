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
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkExtractSelection.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <sstream>

#include "./jet256colormap.h"


using namespace std;
using namespace boost::filesystem;

class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    static MouseInteractorStyle *New();

    MouseInteractorStyle();

    virtual void OnRightButtonDown();

    vtkSmartPointer<vtkStructuredPoints> Data;
    vtkSmartPointer<vtkDataSetMapper> selectedMapper;
    vtkSmartPointer<vtkActor> selectedActor;
};

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
//        pCamera->SetClippingRange(20, 1000);  // 每次有事件导致Render后，会被重置。
        pCamera->Elevation(30);
        pCamera->Azimuth(-40);
    }

    void addFileNameTextWidget() {
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

    void addTemperatureTextWidget() {
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

    void updateTemperatureTextWidget(string tmp) {
        string header = "TMP\n";
        temperatureTextActor->SetInput(header.append(tmp + "℃").c_str());
    }

    virtual ~MyRenderer() {

    }

    void addVolumePicker() {
        // Set the custom stype to use for interaction.
        vtkSmartPointer<MouseInteractorStyle> style =
                vtkSmartPointer<MouseInteractorStyle>::New();
        style->SetDefaultRenderer(renderer);
        style->Data = imgData;
        renderInteractor->SetInteractorStyle(style);
    }

    int positionNormalize(float scalar) {
        return (scalar - 5) / 10;
    }

    int temperatureNormalize(float t) {
        return (t - temperature_min) / (temperature_max - temperature_min) * colorNumber - 2 + 1;
    }

    string scalarToTemperature(int scalar) {
        float tmp = (float)scalar / (colorNumber - 1) * (temperature_max - temperature_min) + temperature_min;
        // set precision
        stringstream stream;
        stream << fixed << setprecision(1) << tmp;
        string s = stream.str();
        return s;
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

    vtkSmartPointer<vtkTextActor> text_actor;
    vtkSmartPointer<vtkTextWidget> text_widget;
    vtkSmartPointer<vtkTextActor> temperatureTextActor;
    vtkSmartPointer<vtkTextWidget> temperatureTextWidget;
};

MyRenderer *myRenderer;

// Catch mouse events
MouseInteractorStyle::MouseInteractorStyle() {
    selectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    selectedActor = vtkSmartPointer<vtkActor>::New();
}

void MouseInteractorStyle::OnRightButtonDown() {
    // Get the location of the click (in window coordinates)
    // 支持5个点目前
    int *pos = this->GetInteractor()->GetEventPosition();

    vtkSmartPointer<vtkVolumePicker> picker =
            vtkSmartPointer<vtkVolumePicker>::New();
    picker->PickClippingPlanesOn();
    picker->SetPickClippingPlanes(100);
    picker->GetPickClippingPlanes();
    picker->SetTolerance(0.0005);
    picker->SetVolumeOpacityIsovalue(0.001);    // threshold for select volume
    // Pick from this location.
    // 只有 x,y 没有 z,因为是平面
    picker->Pick(pos[0], pos[1], 0, this->GetDefaultRenderer());

    std::cout << "Pick pos is: " << pos[0] << " " << pos[1]
              << " " << endl;

    double *worldPosition = picker->GetPickPosition();

    std::cout << "World position is: " << worldPosition[0] << " " << worldPosition[1]
              << " " << worldPosition[2] << endl;

    std::cout << "Cell id is: " << picker->GetCellId() << std::endl;

    if (picker->GetCellId() != -1) {

//            std::cout << "Pick position is: " << worldPosition[0] << " " << worldPosition[1]
//                      << " " << worldPosition[2] << endl;

        vtkSmartPointer<vtkIdTypeArray> ids =
                vtkSmartPointer<vtkIdTypeArray>::New();
        ids->SetNumberOfComponents(1);
        ids->InsertNextValue(picker->GetCellId());

        vtkSmartPointer<vtkSelectionNode> selectionNode =
                vtkSmartPointer<vtkSelectionNode>::New();
        selectionNode->SetFieldType(vtkSelectionNode::CELL);
        selectionNode->SetContentType(vtkSelectionNode::INDICES);
        selectionNode->SetSelectionList(ids);

        vtkSmartPointer<vtkSelection> selection =
                vtkSmartPointer<vtkSelection>::New();
        selection->AddNode(selectionNode);

        vtkSmartPointer<vtkExtractSelection> extractSelection =
                vtkSmartPointer<vtkExtractSelection>::New();
        extractSelection->SetInputData(0, (vtkDataObject *) this->Data.GetPointer());
        extractSelection->SetInputData(1, selection);
        extractSelection->Update();

        // In selection
        vtkSmartPointer<vtkUnstructuredGrid> selected =
                vtkSmartPointer<vtkUnstructuredGrid>::New();
        selected->ShallowCopy(extractSelection->GetOutput());
        cout << "selected" << endl;
//            selected->Print(cout);

        std::cout << "There are " << selected->GetNumberOfPoints()
                  << " points in the selection." << std::endl;
        vtkPoints *pPoints = selected->GetPoints();
//            cout << "points scalar: " <<  endl;
//            pPoints->Print(cout);

        // 各点的属性
        vtkPointData *pPointData = selected->GetPointData();
        vtkDataArray *scalars = pPointData->GetScalars("ImageScalars");

        for (int i = 0; i < selected->GetNumberOfPoints(); i++) {
            std:
            {
                double *pDouble = pPoints->GetPoint(i);
                cout << "point " << i << ": ";
                for (int j = 0; j < 3; j++) {
                    cout << " " << pDouble[j];
                }
                // 点的属性值
                cout << " " << scalars->GetComponent(i, 0);
                cout << endl;
            }
        }

        // update temperature text
        myRenderer->updateTemperatureTextWidget(myRenderer->scalarToTemperature(scalars->GetComponent(0, 0)));

        std::cout << "There are " << selected->GetNumberOfCells()
                  << " cells in the selection." << std::endl;

        selectedMapper->SetInputData(selected);

        selectedActor->SetMapper(selectedMapper);
        selectedActor->GetProperty()->EdgeVisibilityOn();
        selectedActor->GetProperty()->SetEdgeColor(1, 0, 0);
        selectedActor->GetProperty()->SetLineWidth(0.5);

        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(selectedActor);
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

vtkStandardNewMacro(MouseInteractorStyle);


int main() {

    string fileBaseDir = "/home/honhe/Downloads/volume_render/temperature_data/idw_penalty-result_20161024/";
    myRenderer = new MyRenderer();
    myRenderer->init(fileBaseDir);
    myRenderer->addScalarBarWidget();
    myRenderer->addFileNameTextWidget();
    myRenderer->addTemperatureTextWidget();
    myRenderer->listTemperatureFiles();
    myRenderer->readFile(myRenderer->fileNames[0].string());
    myRenderer->prepareVolume();
    myRenderer->addGrid();
    myRenderer->addOrientationMarkerWidget();
    myRenderer->setCamera();
    myRenderer->addVolumePicker();
    myRenderer->render();
}
