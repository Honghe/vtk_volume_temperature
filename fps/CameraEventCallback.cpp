//
// Created by honhe on 11/4/16.
//

#include <vtkObjectFactory.h>
#include "CameraEventCallback.h"
#include "../MyDirector.h"

vtkStandardNewMacro(CameraEventCallback);

void CameraEventCallback::Execute(vtkObject *caller, unsigned long eventId,
                                  void *vtkNotUsed(callData)) {
    fpsRenderer->myDirector->fpsRendererCameraUpdateEvent();
    fpsRenderer->updateWall();
}
void CameraEventCallback::init(FpsRenderer *fpsRenderer) {
    this->fpsRenderer = fpsRenderer;
}
