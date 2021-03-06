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
#ifndef ALIGNMENT_TOOL_UI_H
#define ALIGNMENT_TOOL_UI_H

#include <QWidget>
#include <QString>
#include <QMap>
#include <ui_AlignmentTool.h>

#include "Aligner.h"
#include "rtBasic3DPointData.h"
#include "rt2dSliceDataObject.h"
#include "rt3dPointBufferDataObject.h"

class AlignmentToolUI : public QWidget, public Ui::AlignmentTool
{
  Q_OBJECT

public:
  AlignmentToolUI();
  ~AlignmentToolUI();

public slots:
  void objectAdded(int);
  void objectRemoved(int);

  void aimingOffsetChanged(double);
  void monitoringOffsetChanged(double);
  
  void pointMoved();

  void update();
  
  void updatePlanesChanged(bool);
  void planeIndexChanged(int);

  void planeVisibilityCheckBoxChanged(bool);

protected:
  void connectSignals();
  int  populateLists();

  //! Map the combo box indices to the catheter objects.
  QMap<int, rt2DSliceDataObject*> m_planeObjectMap;

  int m_pointTarget;
  int m_pointEntry;
  int m_pointAiming;
  
  bool m_fAutoUpdate;

};

#endif
