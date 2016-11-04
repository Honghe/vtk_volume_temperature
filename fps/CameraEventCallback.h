//
// Created by honhe on 11/4/16.
//

#ifndef DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
#define DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
#include <vtkCommand.h>
#include <vtkObject.h>

class CameraEventCallback: public vtkCommand {
public:
    vtkTypeMacro(CameraEventCallback, vtkCommand);
    static CameraEventCallback *New();
    virtual void Execute(vtkObject *caller, unsigned long eventId,
                         void *vtkNotUsed(callData));
};

#endif //DEMO_VTK_TUTORIAL_CAMERAEVENTCALLBACK_H
