//
// Created by honhe on 12/14/16.
//

#ifndef DEMO_AIRCONDITION_WINDACTORWRAPPER_H
#define DEMO_AIRCONDITION_WINDACTORWRAPPER_H

#include <vtkObject.h>
#include <vtkSmartPointer.h>

class vtkRenderWindow;
class vtkPolyData;

class vtkRenderWindowInteractor;
class vtkPoints;
class vtkCellArray;
class vtkUnsignedCharArray;
class vtkActor;

class WindActorWrapper {

public:
    WindActorWrapper(vtkSmartPointer<vtkRenderWindow> renderWin,
                         vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor);

    vtkSmartPointer<vtkPolyData> polyData;
    vtkSmartPointer<vtkPoints> points;
    vtkSmartPointer<vtkCellArray> vertices;
    vtkSmartPointer<vtkUnsignedCharArray> scalars;
    int data_axis_x = 30;
    int data_axis_y = 20;
    int data_axis_z = 4;
    int colorNumber = 256;

    void init();

    void createData();

    vtkSmartPointer<vtkActor> actor;

    void refreshWind();

    vtkSmartPointer<vtkRenderWindow> renderWin;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;

    vtkSmartPointer<vtkPoints> pointsBase;

    bool useParabola = true;
};


#endif //DEMO_AIRCONDITION_WINDACTORWRAPPER_H
