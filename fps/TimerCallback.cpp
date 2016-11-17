//
// Created by honhe on 11/3/16.
//

#include <vtkRenderWindowInteractor.h>
#include "TimerCallback.h"
// TimerCallback
//
TimerCallback *TimerCallback::New() {
    TimerCallback *cb = new TimerCallback;
    cb->TimerCount = 0;
    return cb;
}

void TimerCallback::Execute(vtkObject *caller, unsigned long eventId,
                               void *vtkNotUsed(callData)) {
    vtkRenderWindowInteractor *iren =
            static_cast<vtkRenderWindowInteractor *>(caller);

    if (vtkCommand::TimerEvent == eventId) {
//        cout << "TimerCount " << TimerCount << endl;
//        ++this->TimerCount;
    }
    if (fpsRenderer->volumeFileIndex >= fpsRenderer->fileNames.size() - 1) {
        return;
    }
    fpsRenderer->readFile(fpsRenderer->fileNames[fpsRenderer->volumeFileIndex].string());
    fpsRenderer->refreshRender();

    // save screenshot
    // TODO 2016.11.17 用异步线程截图，但有几个问题: 1. 如何保证对应文件名只截一张图
//    fpsRenderer->screenShot(fpsRenderer->volumeFileIndex);

    fpsRenderer->volumeFileIndex++;
}

void TimerCallback::init(FpsRenderer *fpsRenderer) {
    this->fpsRenderer = fpsRenderer;
}
