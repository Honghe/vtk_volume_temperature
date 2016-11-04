//
// Created by honhe on 11/4/16.
//

#ifndef DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
#define DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
#include <vtkCommand.h>
#include <vtkObject.h>
#include "FpsRenderer.h"

class CameraEventCallback: public vtkCommand {
public:
    vtkTypeMacro(CameraEventCallback, vtkCommand);
    static CameraEventCallback *New();
    void init(FpsRenderer*);

    virtual void Execute(vtkObject *caller, unsigned long eventId,
                         void *vtkNotUsed(callData));

private:
    FpsRenderer *fpsRenderer;
};

#endif //DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
