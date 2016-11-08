//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_BASEPSRENDERER_H
#define DEMO_VTK_TUTORIAL_BASEPSRENDERER_H

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
#include <vtkCallbackCommand.h>
#include <vtkTextActor.h>
#include <sstream>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include "../jet256colormap.h"
#include "MouseInteractorStyle.h"
#include <boost/format.hpp>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace boost::filesystem;


class MyDirector;

/**
 * FPS Render
 */
class BasePsRenderer {
public:

    BasePsRenderer(vtkSmartPointer<vtkRenderWindow> pointer, vtkSmartPointer<vtkRenderWindowInteractor> smartPointer,
                   MyDirector *pDirector);

    vector<vector<double>> myLookTable;
    std::string fileBaseDir;
    vector<path> fileNames;
    int volumeFileIndex = 0;
    vtkSmartPointer<vtkRenderer> renderer;

    BasePsRenderer();

    void setViewPort(double *_args);

    void init(std::string fileBaseDir, std::string screenShotDir);

    void initVolumeDataMemory();

    void refreshRender();

    void render();

    void screenShot(int number);

    void listTemperatureFiles();

    void readFile(std::string fileName);

    void addScalarBarWidget();

    void addGrid();

    void addOrientationMarkerWidget();

    void prepareVolume();

    void addFileNameTextWidget();

    void addTemperatureTextWidget();

    void updateTemperatureTextWidget(std::string tmp);

    virtual ~BasePsRenderer() {
    }

    int positionNormalize(float scalar);

    /**
     * 规一化
     * 若超过边界，则置相应边界值，截断
     * @param t
     * @return
     */
    int temperatureNormalize(float t);

    std::string scalarToTemperature(int scalar);

    float temperature_max;
    int data_axis_x;
    int data_axis_z;
    int data_axis_y;
    float temperature_min;
    vtkIdType colorNumber;
    int rgbaLength;
    vtkSmartPointer<vtkStructuredPoints> imgData;
    MyDirector *myDirector;

protected:
    vtkSmartPointer<vtkRenderWindow> renderWin;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationMarkerWidget;
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;
    vtkSmartPointer<vtkTextActor> text_actor;
    vtkSmartPointer<vtkTextWidget> text_widget;
    vtkSmartPointer<vtkTextActor> temperatureTextActor;
    vtkSmartPointer<vtkTextWidget> temperatureTextWidget;
    vtkSmartPointer<vtkVolume> volume;
    std::string screenShotDir;
    vtkSmartPointer<vtkLookupTable> lookupTable;
};


#endif //DEMO_VTK_TUTORIAL_BASEPSRENDERER_H
