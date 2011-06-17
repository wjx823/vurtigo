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
#ifndef RTREGISTRATION_H
#define RTREGISTRATION_H

#include "ui_landmarkRegistration.h"

#include <QMouseEvent>
#include <QColor>
#include <QPalette>

class rt3DPointBufferDataObject;
class rtRenderObject;
//! A Dialog box for LandMark Registration
class rtRegistration : public QDialog, private Ui::landmarkRegistration {
Q_OBJECT

 public:


  rtRegistration(QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~rtRegistration();


 public slots:
  void registerButtonPressed();

  //! set the names of the 3D point objects in the combo boxes
  void setupPointCombos();
  //! set the names of the 3D volume objects in the combo boxes
  void setupVolumeCombos();
  void setupAllCombos();
  //! setup the source table values with points
  void setupSourceTable();
  //! setup the target table values with points
  void setupTargetTable();

  void volumeChanged();

  void sourceRadioChosen();
  void targetRadioChosen();
  //! add a new point to the active set
  void addActivePoint(QMouseEvent*,int);
  //! create a new 3D point object
  void addNewPoints();

  void placementOff();
  void placementOn();

  void mouseMoved(QMouseEvent*,int);
  void doubleClicked(QMouseEvent*,int);

  void syncToggled(bool flag);

  void colorMatchPressed();

  void tableOneChanged(int,int);
  void tableTwoChanged(int,int);

  void numberLabels();

  void setSourceChanged();
  void setTargetChanged();

protected:

 //! The list of all 3D point objects
 QList<int> m_points;

 //! The list of all 3D volume objects
 QList<int> m_volumes;

 //! The currently selected source points
 rtRenderObject *m_currSetSource;
 //! The currently selected target points
 rtRenderObject *m_currSetTarget;
 //! The currently selected source volume
 rtRenderObject *m_currVolSource;
 //! The currently selected target volume
 rtRenderObject *m_currVolTarget;


 //! the active point set
 int m_activeSet;

 //! The current color
 QColor m_color;
 QStringList m_colorList;

 //! A flag to check whether the mouse moved
 bool m_moved;

 //! The error measure
 double m_error;

 double calculateError(rt3DPointBufferDataObject *one, rt3DPointBufferDataObject *two);


};


#endif // RTREGISTRATION_H
