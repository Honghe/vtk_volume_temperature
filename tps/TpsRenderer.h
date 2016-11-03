//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_TPSRENDERER_H
#define DEMO_VTK_TUTORIAL_TPSRENDERER_H


#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <fps/FpsRenderer.h>

/**
 * TPS Render
 */
class TpsRenderer {
public:
    TpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin, vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor,
                    MyDirector *myDirector);

    vtkSmartPointer<vtkRenderer> renderer;

    TpsRenderer(vtkSmartPointer<vtkRenderWindow>);
    void setViewPort(double *_args);

    void update(FpsRenderer *);

    void init(FpsRenderer *pRenderer);
    void prepareVolume(FpsRenderer *fpsRenderer);
    void addGrid(FpsRenderer *fpsRenderer);

private:
    vtkSmartPointer<vtkRenderWindow> renderWin;
    MyDirector *myDirector;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper;
    vtkSmartPointer<vtkVolume> volume;

};


#endif //DEMO_VTK_TUTORIAL_TPSRENDERER_H
