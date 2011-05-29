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

//#define PROC_DEBUG

#include "InfusionMonitorUI.h"
#include "rtObjectManager.h"
#include "rtBaseHandle.h"
#include "rtApplication.h"
#include "rtRenderObject.h"

#include <QList>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkImageMathematics.h>
#include <vtkImageThreshold.h>

InfusionMonitorUI::InfusionMonitorUI() {
  setupUi(this);
  
 // setup "source" plane accessors
  m_nPlaneSrc = -1;
  m_pPlaneSrc = NULL;

 // create "destination" plane
  m_nPlaneDst = rtBaseHandle::instance().requestNewObject(rtConstants::OT_2DObject, "Infusion Monitoring Plane");
  m_pPlaneDst = static_cast<rt2DSliceDataObject *>(rtBaseHandle::instance().getObjectWithID(m_nPlaneDst));
  
 // create images
  m_pImageRef = vtkImageData::New();
  m_pImageCur = vtkImageData::New();
//  m_pImageOut = vtkImageData::New();
  
  if (!m_pImageRef || !m_pImageCur/* || !m_pImageOut*/)
    {
      // fail somehow here
    }
    
  m_fRefExists = false;
  
  threshold_DoubleSpinBox->setValue(50);
  
  radioButton_diff->setChecked(true);

  populateLists();
  connectSignals();
}

InfusionMonitorUI::~InfusionMonitorUI() {
}

void InfusionMonitorUI::connectSignals() {
  connect( rtApplication::instance().getObjectManager(), SIGNAL(objectCreated(int)), this, SLOT(objectAdded(int)) );
  connect( rtApplication::instance().getObjectManager(), SIGNAL(objectRemoved(int)), this, SLOT(objectRemoved(int)) );

  connect( threshold_DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(threshold_Changed(double)) );
  connect( takeRefButton,           SIGNAL(clicked()),            this, SLOT(takeReference_ButtonPushed()) );

  connect(sourcePlaneComboBox,      SIGNAL(currentIndexChanged(int)), this, SLOT(planeIndexChanged(int)) );
  
  connect(radioButton_ref,          SIGNAL(clicked(bool)),            this, SLOT(showChanged()) );
  connect(radioButton_cur,          SIGNAL(clicked(bool)),            this, SLOT(showChanged()) );
  connect(radioButton_diff,         SIGNAL(clicked(bool)),            this, SLOT(showChanged()) );
}

////////

int InfusionMonitorUI::populateLists() {
  QList<int> planeObjs;
  rt2DSliceDataObject* slice=NULL;

  planeObjs = rtBaseHandle::instance().getObjectsOfType(rtConstants::OT_2DObject);

  sourcePlaneComboBox->clear();

#ifdef PROC_DEBUG
  cout << "InfusionMonitorUI::populateLists" << endl;
#endif  

  for (int ix1=0; ix1<planeObjs.size(); ix1++) {
//   // skip our own plane object - do not put it in the list or allow it to be seleced
//    if (planeObjs[ix1] == m_nPlaneDst)
//      continue;
  
    slice = static_cast<rt2DSliceDataObject*>(rtBaseHandle::instance().getObjectWithID(planeObjs[ix1]));
    if (slice) {
      m_planeObjectMap.insert(ix1, slice);
      
      QString num = QString::number(planeObjs[ix1])+slice->getObjName();
      
      sourcePlaneComboBox->insertItem(ix1, num);
    }
  }
  
  return planeObjs.size();
}

void InfusionMonitorUI::objectAdded(int objID) {
  rtDataObject *data=NULL;
  rt2DSliceDataObject *slice=NULL;

// // skip our own plane object - do not put it in the list or allow it to be seleced
//  if (objID == m_nPlaneDst)
//    return;

#ifdef PROC_DEBUG
  cout << "InfusionMonitorUI::objectAdded" << endl;
#endif  

  data = rtBaseHandle::instance().getObjectWithID(objID);
  if (!data) return;

  if ( data->getObjectType() == rtConstants::OT_2DObject ) {
    slice = static_cast<rt2DSliceDataObject*>(data);
    if (!slice) return;
    m_planeObjectMap.insert(m_planeObjectMap.count(), slice);
    
    QString num = QString::number(objID)+slice->getObjName();

    sourcePlaneComboBox->insertItem(sourcePlaneComboBox->count(), num);
  }

}

//! The object manager has removed an object. It could be of a type we are interesed in.
void InfusionMonitorUI::objectRemoved(int objID) {
 // save previous selection indices
  int sourcePlanePos  = sourcePlaneComboBox->currentIndex();

 // update lists
  populateLists();

 // restore previous selection indices (though this might lead to an offset by one, depending on which was deleted)
  if ( sourcePlanePos >= 0 && sourcePlanePos < sourcePlaneComboBox->count() ) 
    sourcePlaneComboBox->setCurrentIndex(sourcePlanePos);
}

////////

void InfusionMonitorUI::planeIndexChanged(int index)
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::planeIndexChanged" << endl;
#endif    

    if (m_pPlaneSrc != NULL)
      {
        disconnect(m_pPlaneSrc, SIGNAL(objectChanged(int)), this, SLOT(sourcePlane_moved())); 
        disconnect(m_pPlaneSrc, SIGNAL(copyImageData2DSignal()), this, SLOT(sourcePlane_newImage())); 
      }
    
    m_nPlaneSrc = index;
    
    cout << "new id " << m_nPlaneSrc << endl;

    QList<int> planeObjs;
    planeObjs = rtBaseHandle::instance().getObjectsOfType(rtConstants::OT_2DObject);
  
    m_pPlaneSrc = static_cast<rt2DSliceDataObject *>(rtBaseHandle::instance().getObjectWithID(planeObjs[m_nPlaneSrc]));
    
    if (planeObjs[m_nPlaneSrc] == m_nPlaneDst)
      {
        cout << "WARNING: Can't select destination as source! " << endl;
        m_nPlaneSrc = -1;
        m_pPlaneSrc = NULL;
      }

    if (m_pPlaneSrc)
      {
        cout << "new ptr " << m_pPlaneSrc << endl;
        cout << "type " << (int)(m_pPlaneSrc->getObjectType()) << endl; 
        cout << "name " << m_pPlaneSrc->getObjName().toStdString() << endl;
      }
    
    if (m_pPlaneSrc)
      {
        connect(m_pPlaneSrc, SIGNAL(objectChanged(int)), this, SLOT(sourcePlane_moved())); 
        connect(m_pPlaneSrc, SIGNAL(copyImageData2DSignal()), this, SLOT(sourcePlane_newImage())); 
      }

    if (m_pPlaneSrc)
      {  
        m_fRefExists = false;

        updateGeometry();
        updateImage();
      }
  }

////

void InfusionMonitorUI::takeReference()
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::takeReference" << endl;
#endif
    
    m_pImageRef->Initialize();
    
    m_pImageRef->DeepCopy(m_pImageCur);

    m_fRefExists = true;
    
    updateImage();
  }

void InfusionMonitorUI::updateGeometry() 
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::updateGeometry" << endl;
#endif
    
   // copies src geometry to destination
    if (m_pPlaneSrc && m_pPlaneDst)
      {
        vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
        
        m_pPlaneSrc->getTransform()->GetMatrix(matrix);
      
        m_pPlaneDst->lock();
        m_pPlaneDst->setVtkMatrix(matrix, true);
        m_pPlaneDst->Modified();
        m_pPlaneDst->unlock();
      
        matrix->Delete();
      }
  }

void InfusionMonitorUI::updateImage()
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::updateImage" << endl;
#endif

    if (!m_pImageCur || !m_pImageRef)
      return;
      
    if (!m_fRefExists)
      return;
      
    vtkImageData *pImageOut = vtkImageData::New();
    
    if (!pImageOut)
      return;

#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::updateImage actually doing update" << endl;
#endif    

    if (radioButton_ref->isChecked())
      {
        pImageOut->DeepCopy(m_pImageRef);
      }
    else if (radioButton_cur->isChecked())
      {
        pImageOut->DeepCopy(m_pImageCur);
      }
    else if (radioButton_diff->isChecked())
      {
        vtkImageMathematics *diff = vtkImageMathematics::New();
        vtkImageMathematics *abs = vtkImageMathematics::New();
        vtkImageMathematics *scale = vtkImageMathematics::New();
        
        if (!diff || !abs || !scale)
          return;
          
        float thresh = threshold_DoubleSpinBox->value();
        if (thresh == 0)
          thresh = 1;
        
        diff->SetOperationToSubtract();
        diff->SetInput1(m_pImageCur);
        diff->SetInput2(m_pImageRef);
        
        abs->SetOperationToAbsoluteValue();
        abs->SetInput1(diff->GetOutput());
        
        scale->SetOperationToMultiplyByK();
        scale->SetInput1(abs->GetOutput());
        scale->SetConstantK(1000 / thresh);
             
        scale->Update();
        
        pImageOut->CopyStructure(m_pImageRef);
        pImageOut->DeepCopy(scale->GetOutput());
        pImageOut->SetScalarType(VTK_UNSIGNED_SHORT);
        
       // spacing gets lost for some reason - restore it
        double spacing[3];
        m_pImageRef->GetSpacing(spacing);
        pImageOut->SetSpacing(spacing);
        
        diff->Delete();
        abs->Delete();
        scale->Delete();
      }
    else
      {
        cout << "ERROR: No display mode selected in InfusionMonitor" << endl;
        return;
      }

    m_pPlaneDst->lock();
    m_pPlaneDst->copyImageData2D(pImageOut);
    m_pPlaneDst->Modified();
    m_pPlaneDst->unlock();
    
    pImageOut->Delete();
  }

void InfusionMonitorUI::sourcePlane_moved()
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::sourcePlane_moved" << endl;
#endif    
    updateGeometry();
  }
  
void InfusionMonitorUI::sourcePlane_newImage()
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::sourcePlane_newImage" << endl;
#endif    
    m_pImageCur->Initialize();

    if (m_pPlaneSrc)
      {
        vtkImageData *srcImage = m_pPlaneSrc->getRawData();
        m_pImageCur->DeepCopy(srcImage);

        if (!m_fRefExists)
          takeReference();    

        updateImage();
      }
  }

void InfusionMonitorUI::threshold_Changed(double value) 
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::threshold_Changed" << endl;
#endif
    
    updateImage();
  }

void InfusionMonitorUI::takeReference_ButtonPushed(void)
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::takeReference_ButtonPushed" << endl;
#endif
    
    takeReference();
  }
  
void InfusionMonitorUI::showChanged(void)
  {
#ifdef PROC_DEBUG
    cout << "InfusionMonitorUI::showChanged" << endl;
#endif
    
    updateImage();
  }
