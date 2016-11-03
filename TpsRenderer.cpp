//
// Created by honhe on 11/3/16.
//

#include "TpsRenderer.h"

TpsRenderer::TpsRenderer(vtkSmartPointer<vtkRenderWindow> renderWin) {
    this->renderWin = renderWin;
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetViewport(0.5, 0, 1, 1);
    renderWin->AddRenderer(renderer2);
}