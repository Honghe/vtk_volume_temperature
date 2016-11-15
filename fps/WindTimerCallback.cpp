//
// Created by honhe on 11/11/16.
//

#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include "WindTimerCallback.h"

vtkStandardNewMacro(WindTimerCallback);

void WindTimerCallback::init(vtkSmartPointer<vtkRenderWindow> renderWindow, vtkSmartPointer<vtkPoints> points,
                             vtkSmartPointer<vtkUnsignedCharArray> scalars) {
    this->points = points;
    this->scalars = scalars;
    this->renderWindow = renderWindow;
}

void WindTimerCallback::Execute(vtkObject *vtkNotUsed(caller),
                                unsigned long event,
                                void *calldata) {
//    cout << "WindTimerCallback " << endl;
    int wind_axis_x = 30;
    int wind_axis_y = 20;
    int wind_axis_z = 20;

    for (int i = 0; i < wind_axis_x; ++i) {
        for (int j = 0; j < wind_axis_y; ++j) {
            for (int k = 0; k < wind_axis_z; ++k) {
                int id = wind_axis_y * wind_axis_z * i + wind_axis_z * j + k;
                double *point = points->GetPoint(id);
//                    point[0] = (int(point[0]) + 2) % wind_axis_x;
//                    point[0] += 0.2;
                double point0 = point[0] - 0.1;
                if (point0 > wind_axis_x - 1) {
                    point0 -= (wind_axis_x - 1);
                } else if (point0 < 0) {
                    point0 = wind_axis_x - 1;
                }
                points->SetPoint(id, point0, point[1], point[2]);
                scalars->SetValue(id, point0 * 2);
            }
        }
    }
    points->Modified();
    renderWindow->Render();
}

