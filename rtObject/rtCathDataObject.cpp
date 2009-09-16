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
#include "rtCathDataObject.h"


//! Constructor
rtCathDataObject::rtCathDataObject() {
  setObjectType(rtConstants::OT_Cath);

  // Allocate new objects
  m_splineProperty = vtkProperty::New();
  m_pointProperty = vtkProperty::New();
  m_tipProperty = vtkProperty::New();

  m_splinePropertyDlg = new rtPropertyChooserDialog(m_splineProperty);
  m_pointPropertyDlg = new rtPropertyChooserDialog(m_pointProperty);
  m_tipPropertyDlg = new rtPropertyChooserDialog(m_tipProperty);

  // Build the GUI
  setupGUI();

  // More than enough coils
  m_max_coils = 128;


  // Set the default point size.
  m_pointSize = 10;
  m_cathGuiSetup.pointSizeSlider->setValue(m_pointSize);
  pointSizeChanged(m_pointSize);

  m_eType = ET_MEAN;
}

//! Destructor
rtCathDataObject::~rtCathDataObject() {
  cleanupGUI();

  m_splineProperty->Delete();
  m_pointProperty->Delete();
  m_tipProperty->Delete();

  delete m_splinePropertyDlg;
  delete m_pointPropertyDlg;
  delete m_tipPropertyDlg;
}

//! Send the info to the GUI
void rtCathDataObject::update() {

}

// Coil specific
//! Add a coil to the list at a partiucular location.
/*!
  Each coil must have a location before it is created. Multiple coils can be at the same location.
  @param loc The location of the coil. Multiple coils can be at the same location.
  @return The coil ID for the new coil or -1 if the coil could not be added.
  */
int rtCathDataObject::addCoil(int loc) {
  int cID;
  rtCathDataObject::CathCoilData coil;

  cID = getNewCoilID();
  if (cID >= 0) {
    coil.locationID = loc;
    coil.cx = 0.0;
    coil.cy = 0.0;
    coil.cz = 0.0;
    coil.SNR = 1;
    m_coilList.insert(cID, coil);

    m_coilLocations.insert(loc, cID);
  }

  return cID;
}

//! Set the SNR for a coil
bool rtCathDataObject::setCoilSNR(int coilID, int SNR) {
  if (m_coilList.contains(coilID)) {
    m_coilList[coilID].SNR = SNR;
    return true;
  }
  return false;
}

//! Set the angles for a coil.
bool rtCathDataObject::setCoilAngles(int coilID, int a1, int a2) {
  if (m_coilList.contains(coilID)) {
    m_coilList[coilID].angles[0] = a1;
    m_coilList[coilID].angles[1] = a2;
    return true;
  }
  return false;
}

//! Set the coords for a coil.
bool rtCathDataObject::setCoilCoords(int coilID, double cx, double cy, double cz) {
  if (m_coilList.contains(coilID)) {
    m_coilList[coilID].cx = cx;
    m_coilList[coilID].cy = cy;
    m_coilList[coilID].cz = cz;
    return true;
  }
  return false;
}

//! Get a handle for a particular coil object.
rtCathDataObject::CathCoilData* rtCathDataObject::getCoilHandle(int coilID) {
  if (m_coilList.contains(coilID)) {
    return &m_coilList[coilID];
  }
  return NULL;
}

//! Remove a coil from the list.
bool rtCathDataObject::removeCoil(int coilID) {
  if (m_coilList.contains(coilID)){
    m_coilLocations.remove(m_coilList[coilID].locationID, coilID);
    m_coilList.remove(coilID);
    return true;
  }
  return false;
}

// List specific
//! Get the number of coils in the list.
int rtCathDataObject::getNumCoils() {
  return m_coilList.size();
}

//! Get the handle to the coil list.
/*!
  The list should not be modified by the caller.
  @returnTthe handle to the coil list.
  */
QHash<int, rtCathDataObject::CathCoilData>* rtCathDataObject::getListHandle() {
  return &m_coilList;
}

//! The the number of unique locations.
int rtCathDataObject::getNumLocations() {
  return m_coilLocations.uniqueKeys().size();
}

//! List of unique locations on the catheter.
QList<int> rtCathDataObject::getLocationList() {
  return m_coilLocations.uniqueKeys();
}

//! Get the calculated position at a certain location
/*!
  Locations may have multiple coils and the position must be estimated.
  @param loc The location ID
  @param out Resulting position (x,y,z) will be placed here.
  @return True if the resulting position is valid. False otherwise.
  */
bool rtCathDataObject::getPositionAtLocation(int loc, double out[3]) {
  if(!m_coilLocations.contains(loc)) return false;

  QList<int> Ids = m_coilLocations.values(loc);
  int ix1=0;
  double sumSNR=0;
  double maxSNR=0;

  out[0] = 0; out[1] = 0; out[2] = 0;

  switch(m_eType) {
    case (ET_MEAN):
    for (ix1=0; ix1<Ids.size(); ix1++) {
      out[0] = out[0] + m_coilList[Ids[ix1]].cx;
      out[1] = out[1] + m_coilList[Ids[ix1]].cy;
      out[2] = out[2] + m_coilList[Ids[ix1]].cz;
    }
    // Then divide by the number of coils.
    out[0] = out[0] / (double)Ids.size();
    out[1] = out[1] / (double)Ids.size();
    out[2] = out[2] / (double)Ids.size();

    break;
    case (ET_WTMEAN):
    // Weighted mean based on SNR
    for (ix1=0; ix1<Ids.size(); ix1++) {
      sumSNR = sumSNR + m_coilList[Ids[ix1]].SNR;
      out[0] = out[0] + m_coilList[Ids[ix1]].cx*m_coilList[Ids[ix1]].SNR;
      out[1] = out[1] + m_coilList[Ids[ix1]].cy*m_coilList[Ids[ix1]].SNR;
      out[2] = out[2] + m_coilList[Ids[ix1]].cz*m_coilList[Ids[ix1]].SNR;
    }
    // Then divide by the number of coils.
    out[0] = out[0] / sumSNR;
    out[1] = out[1] / sumSNR;
    out[2] = out[2] / sumSNR;
    break;
    case(ET_BESTSNR):
    for (ix1=0; ix1<Ids.size(); ix1++) {
      if ( m_coilList[Ids[ix1]].SNR > maxSNR ) {
        out[0] = m_coilList[Ids[ix1]].cx;
        out[1] = m_coilList[Ids[ix1]].cy;
        out[2] = m_coilList[Ids[ix1]].cz;
        maxSNR = m_coilList[Ids[ix1]].SNR;
      }
    }
    break;
  }

  return true;
}

//! Get the SNR at a particular location
/*!
  The SNR at a location is also defined by the estimation method.
  */
bool rtCathDataObject::getSNRAtLocation(int loc, double &SNR) {
  if(!m_coilLocations.contains(loc)) return false;

  QList<int> Ids = m_coilLocations.values(loc);
  int ix1;
  double sumSNR;
  SNR = 0;

  switch(m_eType) {
    case (ET_MEAN):
    for (ix1=0; ix1<Ids.size(); ix1++) {
      SNR  = SNR + m_coilList[Ids[ix1]].SNR;
    }
    // Then divide by the number of coils.
    SNR = SNR / (double)Ids.size();

    break;
    case (ET_WTMEAN):
    // Weighted mean based on SNR
    for (ix1=0; ix1<Ids.size(); ix1++) {
      sumSNR = sumSNR + m_coilList[Ids[ix1]].SNR;
      SNR  = SNR + m_coilList[Ids[ix1]].SNR*m_coilList[Ids[ix1]].SNR;
    }
    SNR = SNR / sumSNR ;
    break;
    case(ET_BESTSNR):
    for (ix1=0; ix1<Ids.size(); ix1++) {
      if ( m_coilList[Ids[ix1]].SNR > SNR ) {
        SNR = m_coilList[Ids[ix1]].SNR;
      }
    }
    break;
  }

  if (SNR < 0.01) {
    // Safety test. All SNR of less than 0.01 is defaulted to the minimum.
    SNR = 0.01;
  }

  return true;
}


//! Set the GUI widgets.
void rtCathDataObject::setupGUI() {
  QWidget* wid = getBaseWidget();

  m_cathGuiSetup.setupUi(wid);

  m_useSNRSize = false;
  m_cathGuiSetup.snrSizeCheckBox->setChecked(m_useSNRSize);

  connect(m_cathGuiSetup.splinePropButton, SIGNAL(pressed()), this, SLOT(splinePropertyDialog()));
  connect(m_cathGuiSetup.pointPropButton, SIGNAL(pressed()), this, SLOT(pointPropertyDialog()));
  connect(m_cathGuiSetup.tipPropButton, SIGNAL(pressed()), this, SLOT(tipPropertyDialog()));

  connect(m_cathGuiSetup.pointSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(pointSizeChanged(int)));

  connect(m_splinePropertyDlg, SIGNAL(propertyChanged()), this, SLOT(Modified()));
  connect(m_pointPropertyDlg, SIGNAL(propertyChanged()), this, SLOT(Modified()));
  connect(m_tipPropertyDlg, SIGNAL(propertyChanged()), this, SLOT(Modified()));

  connect(m_cathGuiSetup.snrSizeCheckBox, SIGNAL(stateChanged(int)),  this, SLOT(useSNRSizeChanged(int)));
}

//! Clean the GUI widgets.
void rtCathDataObject::cleanupGUI() {
}

//! Return the next unused coil ID
int rtCathDataObject::getNewCoilID() {
  int ix1;
  for (ix1=0; ix1<m_max_coils; ix1++) {
    if (!m_coilList.contains(ix1)) return ix1;
  }
  return -1;
}

//! Called when the spline properties are requested.
void rtCathDataObject::splinePropertyDialog() {
  m_splinePropertyDlg->show();
}

//! Called when the point properties are requested.
void rtCathDataObject::pointPropertyDialog() {
  m_pointPropertyDlg->show();
}


//! Called when the tip properties are requested.
void rtCathDataObject::tipPropertyDialog() {
  m_tipPropertyDlg->show();
}

//! Called when the slider changes and the point sizes are changed.
void rtCathDataObject::pointSizeChanged(int size) {
  m_cathGuiSetup.pointSizeLabel->setText(QString::number(size) + " X.");
  m_pointSize = size;
  Modified();
}

//! The SNR check box was changed
void rtCathDataObject::useSNRSizeChanged(int status) {
  if (status == Qt::Unchecked) {
    m_useSNRSize = false;
  } else {
    m_useSNRSize = true;
  }
}
