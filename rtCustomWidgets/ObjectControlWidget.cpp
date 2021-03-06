/***************************************************************************
Vurtigo: Visualization software for interventional applications in medicine


Copyright (c) 2011, Sunnybrook Research Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sunnybrook Research Institute nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Sunnybrook Research Institute BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "ObjectControlWidget.h"

#include "rtMainWindow.h"
#include "rtCameraControl.h"
#include "rtApplication.h"

#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkPropCollection.h>
#include <vtkPlane.h>
#include <vtkMath.h>
#include <vtkWorldPointPicker.h>

#include <algorithm>

ObjectControlWidget::ObjectControlWidget() {
  m_showing = false;
  m_vis.clear();
  m_xsize = 1.0;
  m_ysize = 1.0;
  m_transform = vtkTransform::New();
  m_transform->Identity();
  m_userTransform = vtkTransform::New();
  m_userTransform->Identity();

  // Setup how the points are located around the widget
  //////////////////////
  // 0 1 2
  // 3 4 5
  // 6 7 8
  ////////////////////
  for (int ix1=0; ix1<3; ix1++) {
    for (int ix2=0; ix2<3; ix2++) {
      m_pointLocations[ix1+3*ix2][0] = ix1;
      m_pointLocations[ix1+3*ix2][1] = ix2;
      m_pointLocations[ix1+3*ix2][2] = 0.0f;  // The z is centered.
    }
  }
  // And the z directions
  m_pointLocations[9][0] = 1;
  m_pointLocations[9][1] = 1;
  m_pointLocations[9][2] = 1; // Positive z

  m_pointLocations[10][0] = 1;
  m_pointLocations[10][1] = 1;
  m_pointLocations[10][2] = -1; // Negative z

  m_corners[0] = 0;
  m_corners[1] = 2;
  m_corners[2] = 8;
  m_corners[3] = 6;

  // There is no current prop yet
  m_currProp = NULL;

  // Opacity starts off as 100% opaque.
  setWidgetOpacity(1.0);

  // No object of interest just yet.
  m_objOfInterest = NULL;

}

ObjectControlWidget::~ObjectControlWidget() {
  m_transform->Delete();
  m_userTransform->Delete();
}

void ObjectControlWidget::setTransform(vtkTransform* input) {
  if (!input || m_currProp) return;

  m_transform->DeepCopy(input);
  if(m_showing) {
    updateWidgetPosition();
  }
}

void ObjectControlWidget::getTransform(vtkTransform* output) {
  output->Identity();
  output->Concatenate(m_transform);
}

void ObjectControlWidget::setSize(double xsize, double ysize, double zsize) {
  m_xsize = xsize;
  m_ysize = ysize;
  m_zsize = zsize;

  // When the size changes so do the point locations.
  for (int ix1=0; ix1<3; ix1++) {
    for (int ix2=0; ix2<3; ix2++) {
      m_pointLocations[ix1+3*ix2][0] = (ix1)*m_xsize*0.5;
      m_pointLocations[ix1+3*ix2][1] = (ix2)*m_ysize*0.5;
      m_pointLocations[ix1+3*ix2][2] = 0.0f;  // The z is centered.
    }
  }

  // And the z directions
  m_pointLocations[9][0] = 1*m_xsize*0.5;
  m_pointLocations[9][1] = 1*m_ysize*0.5;
  m_pointLocations[9][2] = 1*m_zsize*0.5; // Positive z

  m_pointLocations[10][0] = 1*m_xsize*0.5;
  m_pointLocations[10][1] = 1*m_ysize*0.5;
  m_pointLocations[10][2] = -1*m_zsize*0.5; // Negative z

  sizeChanged();
}

void ObjectControlWidget::sizeChanged() {
  // Empty in this class
}

void ObjectControlWidget::show() {
  if(m_showing) return;
  m_showing = true;
  visibilityChanged();
}

void ObjectControlWidget::hide() {
  if(!m_showing) return;
  m_showing = false;
  visibilityChanged();
}

bool ObjectControlWidget::isShowing() {
  return m_showing;
}

void ObjectControlWidget::visibilityChanged() {
  // Empty Here
}


void ObjectControlWidget::setUserTransform(vtkTransform* t) {
  if (!t) return;
  m_userTransform->Identity();
  m_userTransform->Concatenate(t);
  userTransformChanged();
}

void ObjectControlWidget::userTransformChanged() {
  // Empty here
}

void ObjectControlWidget::setWidgetOpacity(double op) {
  if (op > 1.0) op = 1.0;
  else if (op < 0.0) op = 0.0;
  m_widgetOpacity = op;
  widgetOpacityChanged();
}

double ObjectControlWidget::getWidgetOpacity() {
  return m_widgetOpacity;
}

void ObjectControlWidget::widgetOpacityChanged() {
  // Empty in the base class
}

/////////////////
// Public Slots
/////////////////
void ObjectControlWidget::mousePressEvent(QMouseEvent* event, int window) {
  if(!m_showing || !event) return;
}

void ObjectControlWidget::mouseMoveEvent(QMouseEvent* event, int window) {
  if(!m_showing || !event) return;
}

void ObjectControlWidget::mouseReleaseEvent(QMouseEvent* event, int window) {
  if(!m_showing || !event) return;
}

void ObjectControlWidget::mouseDoubleClickEvent(QMouseEvent* event, int window) {
  if(!m_showing || !event) return;

}

void ObjectControlWidget::keyPressEvent(QKeyEvent* event, int window) {
  if(!m_showing || !event) return;

}

void ObjectControlWidget::keyReleaseEvent(QKeyEvent* event, int window) {
  if(!m_showing || !event) return;

}

void ObjectControlWidget::wheelEvent(QWheelEvent* event, int window) {
  if(!m_showing || !event) return;
}

//////////////
// Protected
//////////////

void ObjectControlWidget::updateWidgetPosition() {
  // Empty in the base class.
}

void ObjectControlWidget::updateConvertedLocations() {
  for (int ix1=0; ix1<11; ix1++) {
    m_transform->TransformPoint(m_pointLocations[ix1], m_convertedLocations[ix1]);
  }
}

bool ObjectControlWidget::pickedObjectOfInterest(int x, int y) {
  if (!m_objOfInterest) return false;

  vtkActor* result;  

  result = NULL;
  for (int ix1=0; ix1<rtApplication::instance().getMainWinHandle()->getNumRenWins();ix1++)
  {
      vtkRenderer* ren = rtApplication::instance().getMainWinHandle()->getRenderer(ix1);
      vtkPropCollection* col = vtkPropCollection::New();
      vtkPropPicker* pick = vtkPropPicker::New();
      col->AddItem(m_objOfInterest);


      if (pick->PickProp(x, y, ren, col) ) {
          result = static_cast<vtkActor*>(pick->GetViewProp());
      }

      if(col) col->Delete();
      if(pick) pick->Delete();
  }

  return result == m_objOfInterest;
}
