//
// Created by honhe on 11/11/16.
//

#ifndef DEMO_VTK_TUTORIAL_WINDTIMERCALLBACK_H
#define DEMO_VTK_TUTORIAL_WINDTIMERCALLBACK_H

#include <vtkRenderWindow.h>
#include <vtkPoints.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>

class WindTimerCallback : public vtkCommand {
public:
vtkTypeMacro(WindTimerCallback, vtkCommand);

    static WindTimerCallback *New();

    vtkSmartPointer<vtkRenderWindow> renderWindow;

    void
    init(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkPoints> points,
         vtkSmartPointer<vtkUnsignedCharArray> scalars);

    virtual void Execute(vtkObject *caller, unsigned long eventId,
                         void *vtkNotUsed(callData));

    vtkSmartPointer<vtkUnsignedCharArray> scalars;
    vtkSmartPointer<vtkPoints> points;
    int wind_axis_x;
    int wind_axis_y;
    int wind_axis_z;

    void Modified();

    int GetDebug();

    vtkSetMacro(wind_axis_x, int);
    vtkSetMacro(wind_axis_y, int);
    vtkSetMacro(wind_axis_z, int);
};


#endif //DEMO_VTK_TUTORIAL_WINDTIMERCALLBACK_H
