/*******************************************************************************
    Vurtigo: Visualization software for interventional applications in medicine
    Copyright (C) 2009 Sunnybrook Health Sciences Centre

    This file is part of Vurtigo.

    Vurtigo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include "rtMatrixRenderObject.h"
#include "rtMatrixDataObject.h"
#include "rtApplication.h"
#include "rtMainWindow.h"
#include "customQVTKWidget.h"

rtMatrixRenderObject::rtMatrixRenderObject() {
  setObjectType("OT_vtkMatrix4x4");
  setName("Matrix Renderer");
  setupDataObject();
  setupPipeline();
}


rtMatrixRenderObject::~rtMatrixRenderObject() {
  if (m_planeSource.GetPointer()) m_planeSource->Delete();
  if (m_mapper) m_mapper->Delete();
  if (m_actor) m_actor->Delete();
  if (m_property) m_property->Delete();
}


//! Take info from the data object. 
void rtMatrixRenderObject::update() {

  rtMatrixDataObject* dObj = static_cast<rtMatrixDataObject*>(m_dataObj);
  if (!dObj) return;

  double orig[3], pt1[3], pt2[3];

  orig[0]=0.0; orig[1]=0.0; orig[2]=0.0;
  pt1[0]=1.0; pt1[1]=0.0; pt1[2]=0.0;
  pt2[0]=0.0; pt2[1]=1.0; pt2[2]=0.0;

  dObj->getTransform()->TransformPoint(orig, orig);
  dObj->getTransform()->TransformPoint(pt1, pt1);
  dObj->getTransform()->TransformPoint(pt2, pt2);

  m_planeSource->SetOrigin(orig);
  m_planeSource->SetPoint1(pt1);
  m_planeSource->SetPoint2(pt2);
}

//! Add this object to the given renderer.
bool rtMatrixRenderObject::addToRenderer(vtkRenderer* ren,int window) {
  if (!ren) return false;
  setVisible3D(window,true);

  if(!ren->HasViewProp(m_actor)) {
    ren->AddViewProp(m_actor);
  }

  // Connect signals and slots for interaction.
  customQVTKWidget* renWid;
  renWid = rtApplication::instance().getMainWinHandle()->getRenderWidget(window);
  // Connect mouse actions
  connect(renWid, SIGNAL(interMousePress(QMouseEvent*,int)), this, SLOT(mousePressEvent(QMouseEvent*,int)));
  connect(renWid, SIGNAL(interMouseMove(QMouseEvent*,int)), this, SLOT(mouseMoveEvent(QMouseEvent*,int)));
  connect(renWid, SIGNAL(interMouseRelease(QMouseEvent*,int)), this, SLOT(mouseReleaseEvent(QMouseEvent*,int)));
  connect(renWid, SIGNAL(interMouseDoubleClick(QMouseEvent*,int)), this, SLOT(mouseDoubleClickEvent(QMouseEvent*,int)));
  connect(renWid, SIGNAL(interKeyPress(QKeyEvent*,int)), this, SLOT(keyPressEvent(QKeyEvent*,int)));
  connect(renWid, SIGNAL(interKeyRelease(QKeyEvent*,int)), this, SLOT(keyReleaseEvent(QKeyEvent*,int)));
  connect(renWid, SIGNAL(interWheel(QWheelEvent*,int)), this, SLOT(wheelEvent(QWheelEvent*,int)));

  return true;
}

//! Remove this object from the given renderer.
bool rtMatrixRenderObject::removeFromRenderer(vtkRenderer* ren,int window) {
  if (!ren) return false;
  setVisible3D(window,false);

  if(ren->HasViewProp(m_actor)) {
    ren->RemoveViewProp(m_actor);
  }

  customQVTKWidget* renWid;
  renWid = rtApplication::instance().getMainWinHandle()->getRenderWidget(window);

  // Disconnect mouse actions
  disconnect(renWid, SIGNAL(interMousePress(QMouseEvent*,int)), this, SLOT(mousePressEvent(QMouseEvent*,int)));
  disconnect(renWid, SIGNAL(interMouseMove(QMouseEvent*,int)), this, SLOT(mouseMoveEvent(QMouseEvent*,int)));
  disconnect(renWid, SIGNAL(interMouseRelease(QMouseEvent*,int)), this, SLOT(mouseReleaseEvent(QMouseEvent*,int)));
  disconnect(renWid, SIGNAL(interMouseDoubleClick(QMouseEvent*,int)), this, SLOT(mouseDoubleClickEvent(QMouseEvent*,int)));
  disconnect(renWid, SIGNAL(interKeyPress(QKeyEvent*,int)), this, SLOT(keyPressEvent(QKeyEvent*,int)));
  disconnect(renWid, SIGNAL(interKeyRelease(QKeyEvent*,int)), this, SLOT(keyReleaseEvent(QKeyEvent*,int)));
  disconnect(renWid, SIGNAL(interWheel(QWheelEvent*,int)), this, SLOT(wheelEvent(QWheelEvent*,int)));

  return true;
}

//! Create the correct data object.
void rtMatrixRenderObject::setupDataObject() {
  m_dataObj = new rtMatrixDataObject();
}


//! Create the part of the pipeline that is done first. 
void rtMatrixRenderObject::setupPipeline() {
  m_planeSource = vtkPlaneSource::New();
  m_mapper = vtkPolyDataMapper::New();
  m_actor = vtkActor::New();
  m_property = vtkProperty::New();

  m_planeSource->SetOrigin(0,0,0);
  m_planeSource->SetPoint1(1,0,0);
  m_planeSource->SetPoint2(0,1,0);

  m_mapper->SetInput(m_planeSource->GetOutput());
  m_actor->SetMapper(m_mapper);
  m_actor->SetProperty(m_property);

  m_canRender3D = true;
}

//! The bounds for the plane in 3D space.
bool rtMatrixRenderObject::getObjectLocation(double loc[6]) {
  if (!m_actor) return false;

  m_actor->GetBounds(loc);

  return true;
}

