//
// Created by honhe on 11/11/16.
//

#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include "WindTimerCallback.h"
#include "FpsRenderer.h"

vtkStandardNewMacro(WindTimerCallback);

void WindTimerCallback::init(vtkSmartPointer<vtkRenderWindow> renderWindow,
                             FpsRenderer *fpsRenderer) {
    this->renderWindow = renderWindow;
    this->fpsRenderer = fpsRenderer;

}

void WindTimerCallback::Execute(vtkObject *vtkNotUsed(caller),
                                unsigned long event,
                                void *calldata) {
    fpsRenderer->refreshWindFlow();
    renderWindow->Render();
}

int WindTimerCallback::GetDebug() {
    return 0;
}

void WindTimerCallback::Modified() {

};
