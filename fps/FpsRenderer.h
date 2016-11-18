//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_FPSRENDERER_H
#define DEMO_VTK_TUTORIAL_FPSRENDERER_H

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

#include "./jet256colormap.h"
#include "MouseInteractorStyle.h"
#include "BasePsRenderer.h"
#include "BasePsRenderer.h"
#include <boost/format.hpp>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace boost::filesystem;


class MyDirector;

/**
 * FPS Render
 */
class FpsRenderer : public BasePsRenderer {
public:

    FpsRenderer(vtkSmartPointer<vtkRenderWindow> pointer, vtkSmartPointer<vtkRenderWindowInteractor> smartPointer,
                MyDirector *pDirector);

    FpsRenderer();

    void init(std::string fileBaseDir, std::string screenShotDir);

    void initVolumeDataMemory();

    void refreshRender();

    void render();

    void screenShot(int number);

    void listTemperatureFiles();

    void readFile(std::string fileName);

    void addScalarBarWidget();

    void addGrid();

    void prepareVolume();

    void setCamera();

    void addFileNameTextWidget();

    void addTemperatureTextWidget();

    void updateTemperatureTextWidget(std::string tmp);

    void addOrientationMarkerWidget();

    virtual ~FpsRenderer() {

    }

    void addVolumePicker();

    int positionNormalize(float scalar);

    /**
     * 规一化
     * 若超过边界，则置相应边界值，截断
     * @param t
     * @return
     */
    int temperatureNormalize(float t);

    std::string scalarToTemperature(int scalar);

    /**
     * auto read the next file
     */
    void setTimeEventObserver(int interval);

    void addCameraEventCallback();

    void addWindFlow();

    vtkSmartPointer<vtkActor> windActor;

    void addAirCondition();

    /**
     * 计算 fps
     */
    void addRenderEndEventCallback();

    /**
     * 是否添加了模拟空调
     */
    bool isAddWindFlow;

    void *_readFile(std::string fileNamePtr);

    string lastFileName;

    static void *_readFiel_helper(void *context);

    double statusBarPositionY1;
    double statusBarPositionY2;

    void setStatusBarTextActor(vtkSmartPointer<vtkTextActor> textActor, vtkSmartPointer<vtkTextWidget> textWidget,
                               double positionX1, double positionX2);

    void addPickPositionTextWidget();

    vtkSmartPointer<vtkTextWidget> pickPositionTextWidget;
    vtkSmartPointer<vtkTextActor> pickPositionTextActor;

    void updatePickPositionWidget(float x, float y, float z);

    int lastPickPoint[3];
    bool isLastPickPoint;

    void updateLastPickPointTemperature();
};


#endif //DEMO_VTK_TUTORIAL_FPSRENDERER_H
