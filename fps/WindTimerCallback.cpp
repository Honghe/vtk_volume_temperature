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

    for (int i = 0; i < wind_axis_x; ++i) {
        for (int j = 0; j < wind_axis_y; ++j) {
            for (int k = 0; k < wind_axis_z; ++k) {
                int id = wind_axis_y * wind_axis_z * i + wind_axis_z * j + k;
                double *point = points->GetPoint(id);
                // 动动方向上随机加速
                double point0 = point[0] - rand() % 20 * 0.05;
                if (point0 > wind_axis_x - 1) {
                    point0 -= (wind_axis_x - 1);
                } else if (point0 < 0) {
                    point0 = wind_axis_x - 1;
                }
                points->SetPoint(id, point0, point[1], point[2]);
                scalars->SetValue(id, point0);
            }
        }
    }
    points->Modified();
    renderWindow->Render();
}

int WindTimerCallback::GetDebug() {
    return 0;
}

void WindTimerCallback::Modified() {

};

