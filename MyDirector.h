//
// Created by honhe on 11/3/16.
//

#ifndef DEMO_VTK_TUTORIAL_MYDIRECTOR_H
#define DEMO_VTK_TUTORIAL_MYDIRECTOR_H


#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <fps/FpsRenderer.h>
#include <tps/TpsRenderer.h>

class MyDirector {
public:
    MyDirector();

    void init();

    void startInteractor();

    std::string getDate() {
        char text[16];
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(text, sizeof(text) - 1, "%Y%m%d_%H%M", t);
        std::string str(text);
        return str;
    }

    void fpsRenderUpdateTpsRenderer(FpsRenderer *);

    void fpsRendererInitTps(FpsRenderer *);
    void fpsRendererPrepareVolume(FpsRenderer *fpsRenderer);
    void fpsRendereraddGrid(FpsRenderer *fpsRenderer);

private:
    vtkSmartPointer<vtkRenderWindow> renderWin;
    vtkSmartPointer<vtkRenderWindowInteractor> renderInteractor;
    FpsRenderer *fpsRenderer;
    TpsRenderer *tpsRenderer;

};

#endif //DEMO_VTK_TUTORIAL_MYDIRECTOR_H
