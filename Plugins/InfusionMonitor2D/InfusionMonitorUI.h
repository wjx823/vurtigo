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

/* 
    Infusion Monitor
    Copyright 2011 University of Wisconsin - Madison
*/


#ifndef INFUSION_MONITOR_UI_H
#define INFUSION_MONITOR_UI_H

#include <QWidget>
#include <QString>
#include <QMap>

#include "ui_InfusionMonitor.h" // xxxx fix weird build problem

#include "rtBasic3DPointData.h"
#include "rt2dSliceDataObject.h"
#include "rt3dPointBufferDataObject.h"

class InfusionMonitorUI : public QWidget, public Ui::InfusionMonitor
{
  Q_OBJECT

public:
  InfusionMonitorUI();
  ~InfusionMonitorUI();

public slots:
  void objectAdded(int);
  void objectRemoved(int);

  void planeIndexChanged(int);

  void threshold_Changed(double);
  
  void takeReference_ButtonPushed(void);
  void showChanged(void);
  
  void sourcePlane_moved();
  void sourcePlane_newImage();

  void updateGeometry();
  void updateImage();
  
  void takeReference();

protected:
  void connectSignals();
  int  populateLists();

  //! Map the combo box indices to the catheter objects.
  QMap<int, rt2DSliceDataObject*> m_planeObjectMap;
  
  int m_fColor;
  int m_fRefExists;

  int m_nPlaneSrc;
  int m_nPlaneDst;
  
  rt2DSliceDataObject *m_pPlaneSrc;
  rt2DSliceDataObject *m_pPlaneDst;
  
  vtkImageData *m_pImageRef;
  vtkImageData *m_pImageCur;
//  vtkImageData *m_pImageOut;
  
};

#endif

