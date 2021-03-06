//
// Created by honhe on 11/7/16.
//

#ifndef DEMO_VTK_TUTORIAL_DIFFERENCERENDERER_H
#define DEMO_VTK_TUTORIAL_DIFFERENCERENDERER_H

#include <fps/BasePsRenderer.h>
#include <fps/FpsRenderer.h>

/**
 * Temperature Difference Renderer
 */
class DifferenceRenderer : public BasePsRenderer {
public:
    void init(FpsRenderer *fpsRenderer);

    FpsRenderer *fpsRenderer;

    DifferenceRenderer(vtkSmartPointer<vtkRenderWindow> renderWin,
                       vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor,
                       MyDirector *myDirector);
    void addTitleTextWidget();

    void setCamera();

    void initVolumeDataMemory();

    void updateImgData();
    vector<vtkDataArray*> imgDataArrayVector;

    unsigned char calTemperatureDifference(int i, int j, int k);

    void prepareVolume();

    void addScalarBarWidget();

    unsigned char accumulateTemperatureDifference(int i, int j, int k, unsigned char i1);
};


#endif //DEMO_VTK_TUTORIAL_DIFFERENCERENDERER_H
