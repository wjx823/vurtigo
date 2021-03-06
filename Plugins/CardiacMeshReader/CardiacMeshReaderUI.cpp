#include "CardiacMeshReaderUI.h"

#include <QFileDialog>

#include <iostream>

#include <vtkErrorCode.h>

#include "rt3dVolumeDataObject.h"
#include "rtPolyDataObject.h"
#include "rtEPDataObject.h"
#include "rtBaseHandle.h"
#include "DICOMFileReader.h"
#include "rtBasic3DPointData.h"
#include "rtCardiacMeshPointData.h"

CardiacMeshReaderUI::CardiacMeshReaderUI() {
  setupUi(this);

  m_lastDir = "";
  m_lastMeshDir = "";

  // Buttons start disabled.
  p1FinishButton->setEnabled(false);
  p1NextButton->setEnabled(false);

  p2FinishButton->setEnabled(false);
  p2NextButton->setEnabled(false);

  connectSignals();
}

CardiacMeshReaderUI::~CardiacMeshReaderUI() {
}

void CardiacMeshReaderUI::connectSignals() {

  // Page 1
  connect(directoryEdit, SIGNAL(editingFinished()), this, SLOT(newDirectory()));
  connect(directoryChooser, SIGNAL(clicked()), this, SLOT(dirChooser()));
  connect(p1NextButton, SIGNAL(clicked()), this, SLOT(page1Next()));
  connect(p1FinishButton, SIGNAL(clicked()), this, SLOT(page1Finish()));

  // Page 2
  minSliceSlider->setMinimum(0);
  maxSliceSlider->setMinimum(0);

  connect(directoryEditMesh, SIGNAL(editingFinished()), this, SLOT(newDirectoryMesh()));
  connect(directoryChooserMesh, SIGNAL(clicked()), this, SLOT(dirChooserMesh()));
  connect(minSliceSlider, SIGNAL(valueChanged(int)), this, SLOT(minSliderChanged(int)));
  connect(maxSliceSlider, SIGNAL(valueChanged(int)), this, SLOT(maxSliderChanged(int)));
  connect(p2BackButton, SIGNAL(clicked()), this, SLOT(page2Back()));
  connect(p2NextButton, SIGNAL(clicked()), this, SLOT(page2Next()));
  connect(p2FinishButton, SIGNAL(clicked()), this, SLOT(page2Finish()));

  // Page 3
  connect(nameLineEdit, SIGNAL(editingFinished()), this, SLOT(namesChanged()));
  connect(meshNameLineEdit, SIGNAL(editingFinished()), this, SLOT(namesChanged()));
  connect(p3BackButton, SIGNAL(clicked()), this, SLOT(page3Back()));
  connect(p3FinishButton, SIGNAL(clicked()), this, SLOT(page3Finish()));
}

//! Slot called when the user changes the directory.
void CardiacMeshReaderUI::newDirectory() {

  if ( m_lastDir != directoryEdit->text() ) {
    m_lastDir = directoryEdit->text();

    // Use the custom reader too.
    bool ok = m_customReader.setDirectory(m_lastDir,DICOMFileReader::I_DICOM);
    infoBrowser->clear();
    if (ok) {
      infoBrowser->append(m_customReader.getComments());
      m_customReader.createVolume(m_customReader.getDICOMImageData());
      p1FinishButton->setEnabled(true);
      p1NextButton->setEnabled(true);
      m_meshReader.setNumPhases(m_customReader.getImageData(true)->GetNumberOfScalarComponents());
    } else {
      infoBrowser->append("Error!");
      p1FinishButton->setEnabled(false);
      p1NextButton->setEnabled(false);
    }

  }
}

//! Popup a dialog box to choose a directory
void CardiacMeshReaderUI::dirChooser() {
  QString dir;
  QDir lastDirUp = QDir(m_lastDir);
  lastDirUp.cdUp();
  dir = QFileDialog::getExistingDirectory(this, "Select DICOM Directory", lastDirUp.path(), QFileDialog::ShowDirsOnly);

  if (dir != "") {
    // The user did select a directory.
    directoryEdit->setText(dir);
    newDirectory();
  }
}

//! Go to the next set in the process.
void CardiacMeshReaderUI::page1Next() {
  meshStackedWidget->setCurrentIndex(1);
}

//! Save the current set of DICOM images as a volume
void CardiacMeshReaderUI::page1Finish() {
  m_vol = rtBaseHandle::instance().requestNewObject(rtConstants::OT_3DObject, m_customReader.getDefaultName());
  if (m_vol >=0) {
    rt3DVolumeDataObject* ptObj = static_cast<rt3DVolumeDataObject*>(rtBaseHandle::instance().getObjectWithID(m_vol));

    if (ptObj) {
      ptObj->lock();
      ptObj->copyNewTransform(m_customReader.getTransform());
      ptObj->copyNewImageData(m_customReader.getImageData(true));
      ptObj->copyTriggerDelayList(m_customReader.getTriggerList());
      ptObj->Modified();
      ptObj->unlock();
    }
  }
}

void CardiacMeshReaderUI::newDirectoryMesh() {
  if ( m_lastMeshDir != directoryEditMesh->text() ) {
    m_lastMeshDir = directoryEditMesh->text();

    // Use the custom mesh reader.
    bool ok = m_meshReader.setDirectory(m_lastMeshDir);
    infoBrowserMesh->clear();
    if (ok) {

      int numPhases=m_customReader.getTriggerList()->size();
      int maxSlices=0;
      for (int ix1=0; ix1<=numPhases; ix1++) {
        MeshPointSet* currPhase=NULL;
        currPhase = m_meshReader.getPointSet(ix1);
        if(!currPhase) continue;
        if (currPhase->getMaxSlice() > maxSlices) {
          maxSlices = currPhase->getMaxSlice();
        }
      }
      minSliceSlider->setMaximum(maxSlices);
      maxSliceSlider->setMaximum(maxSlices);
      minSliceSlider->setValue(0);
      maxSliceSlider->setValue(maxSlices);

      infoBrowserMesh->append(m_meshReader.getComments());
      page2GroupBox->setEnabled(true);
      p2FinishButton->setEnabled(true);
      p2NextButton->setEnabled(true);
    } else {
      infoBrowserMesh->append("Error!");
      page2GroupBox->setEnabled(false);
      p2FinishButton->setEnabled(false);
      p2NextButton->setEnabled(false);
    }

  }
}

void CardiacMeshReaderUI::dirChooserMesh() {
  QString dir;
  // use the DICOM set location for the mesh, since they might be close together
  QDir lastDirUp = QDir(m_lastDir);
  lastDirUp.cdUp();
  dir = QFileDialog::getExistingDirectory(this, "Select Mesh Directory", lastDirUp.path(), QFileDialog::ShowDirsOnly);

  if (dir != "") {
    // The user did select a directory.
    directoryEditMesh->setText(dir);
    newDirectoryMesh();
  }
}

void CardiacMeshReaderUI::minSliderChanged(int val) {
  if ( val == minSliceSlider->maximum () ) {
    val = val-1;
    minSliceSlider->setValue(val);
  }
  if (maxSliceSlider->value() <= val) maxSliceSlider->setValue(val+1);
  minSliceLabel->setText(QString::number(val));
}

void CardiacMeshReaderUI::maxSliderChanged(int val) {
  if ( val == maxSliceSlider->minimum () ) {
    val = val+1;
    maxSliceSlider->setValue(val);
  }
  if (minSliceSlider->value() >= val) minSliceSlider->setValue(val-1);
  maxSliceLabel->setText(QString::number(val));
}

void CardiacMeshReaderUI::page2Next() {
  meshStackedWidget->setCurrentIndex(2);
  nameLineEdit->setText(m_customReader.getDefaultName());
  meshNameLineEdit->setText(m_meshReader.getDefaultName());
}


void CardiacMeshReaderUI::page2Back() {
  meshStackedWidget->setCurrentIndex(0);
}

void CardiacMeshReaderUI::page2Finish() {
  // Set the auto titles and finish the job.
  nameLineEdit->setText(m_customReader.getDefaultName());
  meshNameLineEdit->setText(m_meshReader.getDefaultName());

  page3Finish();
}

void CardiacMeshReaderUI::namesChanged() {
  if (nameLineEdit->text().trimmed() == "" || meshNameLineEdit->text().trimmed() == "" ) {
    p3FinishButton->setEnabled(false);
  } else {
    p3FinishButton->setEnabled(true);
  }
}


void CardiacMeshReaderUI::page3Back() {
  meshStackedWidget->setCurrentIndex(1);
}

void CardiacMeshReaderUI::page3Finish() {

  // Create first the DICOM volume.
  m_vol = rtBaseHandle::instance().requestNewObject( rtConstants::OT_3DObject, nameLineEdit->text() );
  if (m_vol >=0) {
    rt3DVolumeDataObject* ptObj = static_cast<rt3DVolumeDataObject*>(rtBaseHandle::instance().getObjectWithID(m_vol));

    if (ptObj) {
      ptObj->lock();
      ptObj->copyNewTransform(m_customReader.getTransform());
      ptObj->copyNewImageData(m_customReader.getImageData(true));
      ptObj->copyTriggerDelayList(m_customReader.getTriggerList());
      ptObj->copyDicomCommonData(m_customReader.getCommonDataHandle());
      ptObj->Modified();
      ptObj->unlock();
    }
  }

  m_icontourNoSmooth = rtBaseHandle::instance().requestNewObject(rtConstants::OT_vtkPolyData, meshNameLineEdit->text().append(" (No Smoothing) ") );
  m_ocontourNoSmooth = rtBaseHandle::instance().requestNewObject(rtConstants::OT_vtkPolyData, meshNameLineEdit->text().append(" (No Smoothing) ") );
  m_icontourEPMesh = rtBaseHandle::instance().requestNewObject(rtConstants::OT_EPMesh, meshNameLineEdit->text().append(" Inner Contour"));
  m_ocontourEPMesh = rtBaseHandle::instance().requestNewObject(rtConstants::OT_EPMesh, meshNameLineEdit->text().append(" Outer Contour"));

  rtPolyDataObject* iContourObj = static_cast<rtPolyDataObject*>(rtBaseHandle::instance().getObjectWithID(m_icontourNoSmooth));
  if(iContourObj) {
    loadPolyDataFromPoints(iContourObj, MeshPointSet::PT_ICONTOUR, 1.0);
  }

  rtPolyDataObject* oContourObj = static_cast<rtPolyDataObject*>(rtBaseHandle::instance().getObjectWithID(m_ocontourNoSmooth));
  if(oContourObj) {
    loadPolyDataFromPoints(oContourObj, MeshPointSet::PT_OCONTOUR, 1.0);
  }

  if(m_icontourEPMesh >= 0) {
    rtEPDataObject* epData = static_cast<rtEPDataObject*>(rtBaseHandle::instance().getObjectWithID(m_icontourEPMesh));
    if(epData) {
      // Load the points into the data.
      loadEPMeshFromPoints(epData, MeshPointSet::PT_ICONTOUR);
    }
  }

  if (m_ocontourEPMesh >= 0) {
    rtEPDataObject* epData = static_cast<rtEPDataObject*>(rtBaseHandle::instance().getObjectWithID(m_ocontourEPMesh));
    if(epData) {
      // Load the points into the data.
      loadEPMeshFromPoints(epData, MeshPointSet::PT_OCONTOUR);
    }
  }


}

bool CardiacMeshReaderUI::loadPolyDataFromPoints(rtPolyDataObject* data,  MeshPointSet::PointType type, double smoothStep) {
  if (!data) return false;

  rtBasic3DPointData temp;
  rtPolyDataObject::PolyPointLink tempLink;
  QList<rtBasic3DPointData> pointList;
  QList<rtPolyDataObject::PolyPointLink> pointListLink;

  if (type == MeshPointSet::PT_ICONTOUR) {
    temp.setColor(1.0, 0.0, 0.0);
  } else if (type == MeshPointSet::PT_OCONTOUR) {
    temp.setColor(0.0, 0.0, 1.0);
  } else {
    temp.setColor(1.0, 1.0, 1.0);
  }

  pointList.clear();
  pointListLink.clear();

  double space[3];
  m_customReader.getImageData(true)->GetSpacing(space);

  vtkTransform* trans = m_customReader.getTransform();
  //vtkTransform* trans = vtkTransform::New();
  trans->Inverse();

  // Number of phases is the same as the size of the trigger delay list.
  int numPhases=m_customReader.getTriggerList()->size();
  int maxSlices=0;
  for (int ix1=0; ix1<=numPhases; ix1++) {
    MeshPointSet* currPhase=NULL;
    currPhase = m_meshReader.getPointSet(ix1);
    if(!currPhase) continue;
    if (currPhase->getMaxSlice() > maxSlices) {
      maxSlices = currPhase->getMaxSlice();
    }
  }

  double tempPoint[3];
  for (int ix1=0; ix1<=numPhases; ix1++) {
    MeshPointSet* currPhase=NULL;
    currPhase = m_meshReader.getPointSet(ix1);
    if(!currPhase) continue;

    int numVertices=0;
    for (double curvePos=minSliceSlider->value(); curvePos<=maxSliceSlider->value(); curvePos+=smoothStep) {
      for (int ix2=0; ix2<=currPhase->getMaxPtNum(); ix2++) {
        tempPoint[0] = (currPhase->getInterpolateXValue(type, curvePos, ix2)*space[0]);
        tempPoint[1] = (currPhase->getInterpolateYValue(type, curvePos, ix2)*space[1]);
        tempPoint[2] = (curvePos*space[2]+0.0f*space[2]);

        trans->TransformPoint(tempPoint, tempPoint);

        temp.setX(tempPoint[0]);
        temp.setY(tempPoint[1]);
        temp.setZ(tempPoint[2]);

        pointList.append(temp);
        numVertices++;
      }
    }

    for (int ix2=0; ix2<(numVertices-202); ix2++) {
      tempLink.threeVertex[0] = ix2;
      tempLink.threeVertex[1] = ix2+1;
      tempLink.threeVertex[2] = ix2+201;
      pointListLink.append(tempLink);

      tempLink.threeVertex[0] = ix2+1;
      tempLink.threeVertex[1] = ix2+201;
      tempLink.threeVertex[2] = ix2+202;
      pointListLink.append(tempLink);
    }

    data->lock();
    data->setNewGeometry( &pointList, &pointListLink, m_customReader.getTriggerList()->at(ix1) );
    data->Modified();
    data->unlock();

    // Reset for the next round.
    pointList.clear();
    pointListLink.clear();
  }
trans->Inverse();
return true;
}

bool CardiacMeshReaderUI::loadEPMeshFromPoints(rtEPDataObject* data, MeshPointSet::PointType type) {
  if(!data) return false;

  int numPhases=m_customReader.getTriggerList()->size();
  rtCardiacMeshPointData tempPT;

  double space[3];
  double pt[3];
  m_customReader.getImageData(true)->GetSpacing(space);

  vtkTransform* trans = m_customReader.getTransform();
  data->lock();
  data->setTransform(trans);
  data->unlock();

  trans->Inverse();

  data->lock();
  for (int ix1=0; ix1<numPhases; ix1++) {
    MeshPointSet* currPhase=NULL;
    currPhase = m_meshReader.getPointSet(ix1);
    if(!currPhase) continue;

    data->setTriggerDelay(ix1, m_customReader.getTriggerList()->at(ix1));

    tempPT.setPhase(ix1);
    tempPT.setTriggerDelay(m_customReader.getTriggerList()->at(ix1));

    for (double curvePos=minSliceSlider->value(); curvePos<=maxSliceSlider->value(); curvePos++) {
      tempPT.setSlice(curvePos);

      for (int ix2=0; ix2<=currPhase->getMaxPtNum(); ix2++) {
        tempPT.setLocation(ix2);

        pt[0] = (currPhase->getInterpolateXValue(type, curvePos, ix2)-1)*space[0];
        pt[1] = (currPhase->getInterpolateYValue(type, curvePos, ix2)-1)*space[1];
        pt[2] = (curvePos*space[2]+0.0f*space[2]);

        trans->TransformPoint(pt, pt);
        tempPT.setX(pt[0]);
        tempPT.setY(pt[1]);
        tempPT.setZ(pt[2]);

        data->addPoint(tempPT);
      }
    }
  }
  data->unlock();
  trans->Inverse();
  return true;
}
