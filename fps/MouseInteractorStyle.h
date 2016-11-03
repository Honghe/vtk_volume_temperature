//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_MOUSEINTERACTORSTYLE_H
#define DEMO_VTK_TUTORIAL_MOUSEINTERACTORSTYLE_H


#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkStructuredPoints.h>

class FpsRenderer;

class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera {
public:
    static MouseInteractorStyle *New();

    MouseInteractorStyle();

    void init(FpsRenderer*);
    virtual void OnRightButtonDown();

    vtkSmartPointer<vtkStructuredPoints> Data;
    vtkSmartPointer<vtkDataSetMapper> selectedMapper;
    vtkSmartPointer<vtkActor> selectedActor;
    FpsRenderer *fpsRenderer;
};

#endif //DEMO_VTK_TUTORIAL_MOUSEINTERACTORSTYLE_H
