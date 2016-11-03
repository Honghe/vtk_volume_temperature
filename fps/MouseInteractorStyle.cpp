//
// Created by honhe on 11/3/16.
//

#include <vtkDataSetMapper.h>
#include <vtkVolumePicker.h>
#include <vtkIdTypeArray.h>
#include <vtkSelectionNode.h>
#include <vtkSelection.h>
#include <vtkRenderWindow.h>
#include <vtkExtractSelection.h>
#include <vtkUnstructuredGrid.h>
#include <vtkProperty.h>
#include "MouseInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPointData.h"
#include "vtkActor.h"
#include "vtkRendererCollection.h"
#include "FpsRenderer.h"

// MouseInteractorStyle
// Catch mouse events
MouseInteractorStyle::MouseInteractorStyle() {
    selectedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    selectedActor = vtkSmartPointer<vtkActor>::New();
}

void MouseInteractorStyle::init(FpsRenderer *fpsRenderer) {
    this->fpsRenderer = fpsRenderer;
};

void MouseInteractorStyle::OnRightButtonDown() {
    // Get the location of the click (in window coordinates)
    // 支持5个点目前
    int *pos = this->GetInteractor()->GetEventPosition();

    std::cout << "Pick pos is: " << pos[0] << " " << pos[1]
              << " " << endl;

    vtkSmartPointer<vtkVolumePicker> picker =
            vtkSmartPointer<vtkVolumePicker>::New();
    picker->PickClippingPlanesOn();
    picker->SetPickClippingPlanes(100);
    picker->GetPickClippingPlanes();
    picker->SetTolerance(0.0005);
    picker->SetVolumeOpacityIsovalue(0.001);    // threshold for select volume
    // Pick from this location.

    vtkRenderer *renderer = this->GetInteractor()->FindPokedRenderer(pos[0], pos[1]);
    // 不是我们目标的 render 就不进行元素选取
    if (renderer != this->fpsRenderer->renderer) {
        return;
    }
    // 只有 x,y 没有 z,因为是平面
    picker->Pick(pos[0], pos[1], 0, renderer);


    double *worldPosition = picker->GetPickPosition();

    std::cout << "World position is: " << worldPosition[0] << " " << worldPosition[1]
              << " " << worldPosition[2] << endl;

    std::cout << "Cell id is: " << picker->GetCellId() << std::endl;

    if (picker->GetCellId() != -1) {

//            std::cout << "Pick position is: " << worldPosition[0] << " " << worldPosition[1]
//                      << " " << worldPosition[2] << endl;

        vtkSmartPointer<vtkIdTypeArray> ids =
                vtkSmartPointer<vtkIdTypeArray>::New();
        ids->SetNumberOfComponents(1);
        ids->InsertNextValue(picker->GetCellId());

        vtkSmartPointer<vtkSelectionNode> selectionNode =
                vtkSmartPointer<vtkSelectionNode>::New();
        selectionNode->SetFieldType(vtkSelectionNode::CELL);
        selectionNode->SetContentType(vtkSelectionNode::INDICES);
        selectionNode->SetSelectionList(ids);

        vtkSmartPointer<vtkSelection> selection =
                vtkSmartPointer<vtkSelection>::New();
        selection->AddNode(selectionNode);

        vtkSmartPointer<vtkExtractSelection> extractSelection =
                vtkSmartPointer<vtkExtractSelection>::New();
        extractSelection->SetInputData(0, (vtkDataObject *) this->Data.GetPointer());
        extractSelection->SetInputData(1, selection);
        extractSelection->Update();

        // In selection
        vtkSmartPointer<vtkUnstructuredGrid> selected =
                vtkSmartPointer<vtkUnstructuredGrid>::New();
        selected->ShallowCopy(extractSelection->GetOutput());
        cout << "selected" << endl;
//            selected->Print(cout);

        std::cout << "There are " << selected->GetNumberOfPoints()
                  << " points in the selection." << std::endl;
        vtkPoints *pPoints = selected->GetPoints();
//            cout << "points scalar: " <<  endl;
//            pPoints->Print(cout);

        // 各点的属性
        vtkPointData *pPointData = selected->GetPointData();
        vtkDataArray *scalars = pPointData->GetScalars("ImageScalars");

        for (int i = 0; i < selected->GetNumberOfPoints(); i++) {
            std:
            {
                double *pDouble = pPoints->GetPoint(i);
                cout << "point " << i << ": ";
                for (int j = 0; j < 3; j++) {
                    cout << " " << pDouble[j];
                }
                // 点的属性值
                cout << " " << scalars->GetComponent(i, 0);
                cout << endl;
            }
        }

        // update temperature text
        this->fpsRenderer->updateTemperatureTextWidget(this->fpsRenderer->scalarToTemperature(scalars->GetComponent(0, 0)));

        std::cout << "There are " << selected->GetNumberOfCells()
                  << " cells in the selection." << std::endl;

        selectedMapper->SetInputData(selected);

        selectedActor->SetMapper(selectedMapper);
        selectedActor->GetProperty()->EdgeVisibilityOn();
        selectedActor->GetProperty()->SetEdgeColor(1, 0, 0);
        selectedActor->GetProperty()->SetLineWidth(0.5);

        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(selectedActor);
    }

    // Forward events
    vtkInteractorStyleTrackballCamera::OnRightButtonDown();
}

vtkStandardNewMacro(MouseInteractorStyle)
