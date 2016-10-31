#define vtkRenderingCore_AUTOINIT 3(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingOpenGL2)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL2)

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
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkVolumeProperty.h>

#include <stdio.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCallbackCommand.h>

#include <vtkDataSetMapper.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkPolyData.h>

#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>

using namespace std;
using namespace boost::filesystem;

int temperature_min = 18;
int temperature_max = 30;

int positionNormalize(float scalar)
{
    return (scalar - 5) / 10;
}

int temperatureNormalize(float t)
{
    return (t - temperature_min) / (temperature_max - temperature_min) * 254 + 1;
}

class JLCMouseCallback : public vtkCommand
{
private:
    vtkCamera *mCamera;
    long mPressCount = 0;

public:
    static JLCMouseCallback *New()
    {
        return new JLCMouseCallback;
    }

    void SetObject(vtkCamera *camera)
    {
        mCamera = camera;
    }

    virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData)
    {
        double *cameraPos = new double[3];
        double *range = new double[2];
        mCamera->GetPosition(cameraPos);
        printf("Mouse Click Count = %ld:\n", ++mPressCount);
        printf("\tCamera Position = (%f, %f, %f)\n", cameraPos[0], cameraPos[1], cameraPos[2]);
        mCamera->GetViewUp(cameraPos);
        printf("\tCamera View up  = (%f, %f, %f)\n", cameraPos[0], cameraPos[1], cameraPos[2]);
        mCamera->GetFocalPoint(cameraPos);
        printf("\tCamera Focal Point  = (%f, %f, %f)\n", cameraPos[0], cameraPos[1], cameraPos[2]);
        mCamera->GetClippingRange(range);
        printf("\tCamera Clipping Range  = (%f, %f)\n", range[0], range[1]);
        printf("\tCamera View Angle  = %f\n", mCamera->GetViewAngle());
        delete cameraPos;
        delete range;
    }
};

void addGrid(vtkSmartPointer<vtkRenderer> renderer, int data_axis_x, int data_axis_y, int data_axis_z)
{
      float padding = 0.5;
      const vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      for (float k : vector<float>{0 + padding, data_axis_z - padding})
      {
          for (float j : vector<float>{0 + padding, data_axis_y - padding})
          {
              for (float i  : vector<float>{0 + padding, data_axis_x - padding})
              {
                  points->InsertNextPoint(i, j, k);
              }
          }
      }
      const vtkSmartPointer<vtkCellArray> &lines = vtkSmartPointer<vtkCellArray>::New();
      for (int i: vector<int>{0, 4})
      {
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

      for (int l = 0; l < 4; ++l)
      {
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

int main()
{
    int data_axis_x = 80;
    int data_axis_z = 99;
    int data_axis_y = 36;

    string datasetDir = "/home/honhe/Downloads/volume_render/temperature_data/idw_penalty-result_20161024/";
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    window->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);

    vtkSmartPointer<vtkImageImport> dataImporter = vtkSmartPointer<vtkImageImport>::New();
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
    vtkIdType colorNumber = 256;
    int rgbaLength = 4;

    vtkSmartPointer<vtkInteractorStyleTrackballCamera> mStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    interactor->SetInteractorStyle(mStyle);


    // Initialization
    vector<vector<double>> myLookTable;
    vector<vector<double>> ppp(colorNumber, vector<double>(4));
    myLookTable = ppp;
    renderer->SetBackground(0.0, 0.0, 0.0);
    window->SetSize(900, 900);

    // Mouse Lisener
    vtkSmartPointer<JLCMouseCallback> mouseCallback = vtkSmartPointer<JLCMouseCallback>::New();
    mouseCallback->SetObject(renderer->GetActiveCamera());
    interactor->AddObserver(vtkCommand::LeftButtonPressEvent, mouseCallback);

    addGrid(renderer, data_axis_x, data_axis_y, data_axis_z);

    // Add scalar bar
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


    for (int i = 0; i < colorNumber; ++i)   // 使用库内置好的颜色值，Build 再获取，不然是无效值
    {
        vector<double> &row = myLookTable[i];
        double *tableValue = lookupTable->GetTableValue(i);
        for (int j = 0; j < rgbaLength; ++j)
        {
            row[j] = tableValue[j];
        }
    }

    for (int k = 0; k < colorNumber; ++k) // 倒过来赋值
    {
        vector<double> &row = myLookTable[colorNumber - 1 - k];
        lookupTable->SetTableValue(k, row[0], row[1], row[2], row[3]);
    }
    scalarBarActor->SetLookupTable(lookupTable);
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
    scalarBarWidget->SetInteractor(interactor);
    scalarBarWidget->SetScalarBarActor(scalarBarActor);
    vtkScalarBarRepresentation *pRepresentation = scalarBarWidget->GetScalarBarRepresentation();
    pRepresentation->SetPosition(0.9, 0.053796);
    pRepresentation->SetMoving(false);
    pRepresentation->SetShowBorderToOff();
    scalarBarWidget->On();

    // List Temperature Files
    vector<boost::filesystem::path> mFilesName;
    boost::filesystem::path dirPath(datasetDir);
    vector<boost::filesystem::path> filesName;
    if (boost::filesystem::is_directory(dirPath))
    {
        copy(directory_iterator(dirPath), directory_iterator(), back_inserter(filesName));
        sort(filesName.begin(), filesName.end());
        for (vector<boost::filesystem::path>::const_iterator it(filesName.begin()), it_end(filesName.end()); it != it_end; ++it)
        {
            cout << "   " << *it << '\n';
        }
        mFilesName = filesName;
    }
    else
    {
        printf("Error Read Files!\n");
        exit(-1);
    }


    // Import specified one temperature file
    string fileName = mFilesName[190].string();
    cout << "read file " << fileName << endl;
    FILE *f = fopen(fileName.c_str(), "r");
    // pass 2 lines
    const size_t line_size = 300;
    char *line = (char *) malloc(line_size);
    fgets(line, line_size, f);
    fgets(line, line_size, f);
    //
    float x, y, z, t;
    int volume_size = data_axis_x * data_axis_y * data_axis_z;
//        float temperatures[volume_size] = {0};
    vector<vector<vector<int>>> xyzs(data_axis_x, vector<vector<int>>(data_axis_z, vector<int>(data_axis_y)));

    for (int i = 0; i < volume_size; ++i)
    {
        fscanf(f, "%f %f %f %f", &x, &z, &y, &t);
//            printf("%d %f %f %f %f\n", i, x, z, y, t);
        xyzs[positionNormalize(x)][positionNormalize(z)][positionNormalize(y)] =
                temperatureNormalize(t);
    }
    fclose(f);
    vtkSmartPointer<vtkStructuredPoints> imgData = vtkSmartPointer<vtkStructuredPoints>::New();
    imgData->SetExtent(0, data_axis_x - 1, 0, data_axis_y - 1, 0, data_axis_z - 1);
    imgData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    int *dims = imgData->GetDimensions();

    for (int z = 0; z < dims[2]; z++)
    {
        for (int y = 0; y < dims[1]; y++)
        {
            for (int x = 0; x < dims[0]; x++)
            {
                unsigned char *pixel = static_cast<unsigned char *>(imgData->GetScalarPointer(x, y, z));
                pixel[0] = xyzs[x][z][y];
            }
        }
    }

    // Prepare Volume
    const vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i = 2; i < colorNumber; ++i)
    {
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

    // Coordinate system marker
    const vtkSmartPointer<vtkAxesActor> &axesActor1 = vtkSmartPointer<vtkAxesActor>::New();
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientationMarkerWidget->SetOutlineColor(1, 1, 0);
    orientationMarkerWidget->SetOrientationMarker(axesActor1);
    orientationMarkerWidget->SetInteractor(interactor);
    orientationMarkerWidget->SetViewport(0, 0, 0.2, 0.2);;
    orientationMarkerWidget->SetEnabled(true);
    orientationMarkerWidget->SetInteractive(false);

    // Coordinate system marker
    const vtkSmartPointer<vtkAxesActor> &axesActor2 = vtkSmartPointer<vtkAxesActor>::New();
    axesActor2->SetTotalLength(6, 6, 6);
    axesActor2->SetConeRadius(0.1);
    axesActor2->GetXAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axesActor2->GetXAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(20);
    axesActor2->GetYAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axesActor2->GetYAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(20);
    axesActor2->GetZAxisCaptionActor2D()->GetTextActor()->SetTextScaleModeToNone();
    axesActor2->GetZAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(20);
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget2 = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    orientationMarkerWidget2->SetOutlineColor(0, 1, 1);
    orientationMarkerWidget2->SetOrientationMarker(axesActor2);
    orientationMarkerWidget2->SetInteractor(interactor);
    orientationMarkerWidget2->SetViewport(0, 0, 1, 1);;
    orientationMarkerWidget2->SetEnabled(true);
    orientationMarkerWidget2->SetInteractive(false);

    /************************************************************************************************************************
     *
    Camera Position = (186.149431, 128.788384, -113.565990)
    Camera View up  = (-0.335415, 0.884417, 0.324506)
    Camera Focal Point  = (34.898176, 11.488353, 49.790376)
    Camera Clipping Range  = (115.362081, 409.800006)
    Camera View Angle  = 30.000000
     *
    ************************************************************************************************************************/

    renderer->GetActiveCamera()->SetPosition(186.149431, 128.788384, -113.565990);
    renderer->GetActiveCamera()->SetViewUp(-0.335415, 0.884417, 0.324506);
    renderer->GetActiveCamera()->SetFocalPoint(34.898176, 11.488353, 49.790376);
    renderer->GetActiveCamera()->SetClippingRange(115.362081, 409.800006);
    renderer->GetActiveCamera()->SetViewAngle(30.0);

    // Information Display
    vtkSmartPointer<vtkTextActor> title_actor = vtkSmartPointer<vtkTextActor>::New();
    title_actor->SetInput("Indoor Temperature Distribution Visualization");
    title_actor->GetTextProperty()->SetColor(1, 1, 1);
    title_actor->GetTextProperty()->SetBold(true);
    title_actor->GetTextProperty()->SetFontSize(128);
    const vtkSmartPointer<vtkTextRepresentation> &title_representation = vtkSmartPointer<vtkTextRepresentation>::New();
    /**
     * Specify opposite corners of the box defining the boundary of the
     * widget. By default, these coordinates are in the normalized viewport
     * coordinate system, with Position the lower left of the outline, and
     * Position2 relative to Position. Note that using these methods are
     * affected by the ProportionalResize flag. That is, if the aspect ratio of
     * the representation is to be preserved (e.g., ProportionalResize is on),
     * then the rectangle (Position,Position2) is a bounding rectangle.
     */
    title_representation->GetPositionCoordinate()->SetValue(0.35, 0.8);
    title_representation->GetPosition2Coordinate()->SetValue(0.35, 0.2);
    title_representation->SetMoving(false);
    vtkSmartPointer<vtkTextWidget> text_widget = vtkSmartPointer<vtkTextWidget>::New();
    text_widget->SetRepresentation(title_representation);
    text_widget->SetInteractor(interactor);
    text_widget->SetTextActor(title_actor);
    text_widget->SelectableOff();
    text_widget->GetBorderRepresentation()->SetShowBorderToOff();
    text_widget->On();

    // Render
    window->Render();
    interactor->Initialize();
    interactor->Start();
}
