//
// Created by honhe on 11/17/16.
//

#ifndef DEMO_VTK_TUTORIAL_RENDERENDEVENTCALLBACK_H
#define DEMO_VTK_TUTORIAL_RENDERENDEVENTCALLBACK_H


#include <vtkCommand.h>

class FpsRenderer;

class RenderEndEventCallback: public vtkCommand {
public:
vtkTypeMacro(RenderEndEventCallback, vtkCommand);
    static RenderEndEventCallback *New();
    void init(FpsRenderer*);

    virtual void Execute(vtkObject *caller, unsigned long eventId,
                         void *vtkNotUsed(callData));

    FpsRenderer *fpsRenderer;
};


#endif //DEMO_VTK_TUTORIAL_RENDERENDEVENTCALLBACK_H
