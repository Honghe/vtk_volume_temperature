//
// Created by honhe on 11/17/16.
//

#include "RenderEndEventCallback.h"
#include "fps/FpsRenderer.h"


vtkStandardNewMacro(RenderEndEventCallback);

void RenderEndEventCallback::Execute(vtkObject *caller, unsigned long eventId, void *) {
    double timeInSeconds = fpsRenderer->renderer->GetLastRenderTimeInSeconds();
    double fps = 1.0 / timeInSeconds;
    std::cout << "fps FPS: " << fps << std::endl;
}

void RenderEndEventCallback::init(FpsRenderer *fpsRenderer) {
    this->fpsRenderer = fpsRenderer;
}
