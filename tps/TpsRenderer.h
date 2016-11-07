//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_TPSRENDERER_H
#define DEMO_VTK_TUTORIAL_TPSRENDERER_H


#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <fps/BasePsRenderer.h>
#include <fps/FpsRenderer.h>

/**
 * TPS Render
 */
class TpsRenderer : public BasePsRenderer{
public:
    TpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin, vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor,
                    MyDirector *myDirector);

    TpsRenderer(vtkSmartPointer<vtkRenderWindow>);

    void init(FpsRenderer *fpsRenderer);

    void updateImgData();

    void prepareVolume();
    FpsRenderer *fpsRenderer;

    void initVolumeDataMemory();

    void setCamera();

    void updateTpsCamera();

    void addFpsCameraPoly();

    vtkSmartPointer<vtkActor> fpsCameraActor;
    vtkSmartPointer<vtkConeSource> CameraConeSource;
};


#endif //DEMO_VTK_TUTORIAL_TPSRENDERER_H
