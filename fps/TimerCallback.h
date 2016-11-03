//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_VTKTIMERCALLBACK_H
#define DEMO_VTK_TUTORIAL_VTKTIMERCALLBACK_H

#include <vtkCommand.h>
#include <vtkObject.h>
#include "FpsRenderer.h"

class TimerCallback : public vtkCommand {
public:
    vtkTypeMacro(TimerCallback, vtkCommand);

    static TimerCallback *New();

    void init(FpsRenderer*);

    virtual void Execute(vtkObject *caller, unsigned long eventId,
                         void *vtkNotUsed(callData));

private:
    int TimerCount;
    FpsRenderer *fpsRenderer;
};

#endif //DEMO_VTK_TUTORIAL_VTKTIMERCALLBACK_H
