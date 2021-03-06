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
#include "DICOMFileReader.h"

#include <QDir>
#include <QStringList>
#include <QInputDialog>

#include <iostream>
#include <string>
#include <math.h>

#include <vtkMath.h>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>
#include <vtkBMPReader.h>
#include <vtkStringArray.h>
#include <vtkImageActor.h>
#include <vtkRendererCollection.h>
#include <vtkImageFlip.h>

#include <rtBaseHandle.h>
#include <rt3dVolumeDataObject.h>

//DCMTK
#ifdef Q_OS_UNIX
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dctagkey.h"

DICOMFileReader::DICOMFileReader() {
  m_vtkImgData = vtkImageData::New();
  m_transform = vtkTransform::New();
  m_matrixToTransform = vtkMatrixToLinearTransform::New();
  m_matrix = vtkMatrix4x4::New();
  m_infoFix = vtkImageChangeInformation::New();

  // Set up the mini pipeline.
  m_matrix->Identity();
  m_matrixToTransform->SetInput(m_matrix);
  m_transform->SetInput(m_matrixToTransform);
  m_transform->Inverse();

  m_ddata = NULL;

  m_reader = NULL;
  m_spacingX = 1.0;
  m_spacingY = 1.0;
  m_spacingZ = 1.0;


  m_volName = "";
}


DICOMFileReader::~DICOMFileReader() {
  cleanupImageData();
  m_matrixToTransform->Delete();
  m_matrix->Delete();
  m_vtkImgData->Delete();
  m_transform->Delete();
  m_infoFix->Delete();

  if (m_ddata) delete m_ddata;
}

void DICOMFileReader::cleanupImageData() {

  //Clear the list
  while (!m_imgData.empty()) {
    DICOMImageData* temp;
    temp = m_imgData.takeFirst();
    if (temp == m_ddata) {
      m_ddata=NULL;
    }
    delete temp;
  }
  m_comments.clear();
  // Reset the trigger list.
  m_triggerList.clear();
}

//! Set the name of the directory to get the DICOM files from.
bool DICOMFileReader::setDirectory(QString dirPath, imageType type) {

  QDir fileDir(dirPath);
  QStringList files;
  QStringList filters;
  if (type == DICOMFileReader::I_DICOM)
  {

      if (!fileDir.exists()) {
          std::cout << "No such directory: " << dirPath.toStdString() << std::endl;
          return false;
      }

      fileDir.setFilter(QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable | QDir::Files);
      files = fileDir.entryList();

      if (files.count() <= 0) {
          std::cout << "Directory found, but no DICOM files could be found inside:  " << dirPath.toStdString() << std::endl;
          return false;
      }

      cleanupImageData();

      double maxTrig=-1.0;

      // Create the trigger list for the first set of images.
      for (int ix1=0; ix1<files.count(); ix1++) {
          double trig = DICOMImageData::getTriggerFromFile(fileDir.filePath(files.at(ix1)));
          // if ONE file is not a DICOM, fail out of the entire directory
          if (trig == -2) return false;
          if (!m_triggerList.contains(trig) && trig > maxTrig) {
              m_triggerList.append(trig);
              maxTrig = trig;
          } else {
              break;
          }
      }

      for (int ix1=0; ix1<files.count(); ix1++) {
          m_comments.append("Image Number: ===== " + QString::number(ix1+1) + " ===== \n");
          m_ddata = new DICOMImageData();
          // Some image types need a trigger list.
          m_ddata->setTrigList(&m_triggerList);
          if ( !m_ddata->readFile(fileDir.filePath(files.at(ix1))) ) {
              // Failed to read the file.
              std::cout << "Failed to Read DICOM File: " << fileDir.filePath(files.at(ix1)).toStdString() << std::endl;
              // if ONE file is not a DICOM, fail out of the entire directory
              return false;
          } else {
              // File was read correctly.
              m_imgData.append(m_ddata);
          }
      }

      // Give that volume a default name.
      m_volName = QString("E") + m_ddata->getStudyID() + QString("S") + m_ddata->getSeriesNumber();
      return true;
  }
  else
  {
      if (type == DICOMFileReader::I_JPEG)
          filters << "*.jpg";
      else if (type == DICOMFileReader::I_PNG)
          filters << "*.png";
      else if (type == DICOMFileReader::I_BMP)
          filters << "*.bmp";
      else
          return false;

      if (!fileDir.exists()) {
          std::cout << "No such directory: " << dirPath.toStdString() << std::endl;
          return false;
      }
      fileDir.setFilter(QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable | QDir::Files);
      fileDir.setNameFilters(filters);
      files = fileDir.entryList();

      if (files.count() <= 0) {
          std::cout << "Directory found, but no correct image files could be found inside:  " << dirPath.toStdString() << std::endl;
          return false;
      }

      m_comments.clear();

      vtkStringArray *nameList = vtkStringArray::New();
      QImage first(fileDir.filePath(files.at(0)));
      for (int ix1=0; ix1<files.count(); ix1++) {
          QImage img(fileDir.filePath(files.at(ix1)));
          if (img.size() == first.size())
          {
              m_comments.append("Image Number: ===== " + QString::number(ix1+1) + " ===== \n");
              nameList->InsertValue(ix1,fileDir.filePath(files.at(ix1)).toStdString().c_str());
          }
          else
          {
              m_comments.append("Image Number: ===== " + QString::number(ix1+1) + " =====   NOT LOADED\n");
          }
      }

      if (type == DICOMFileReader::I_JPEG)
          m_reader = vtkJPEGReader::New();
      else if (type == DICOMFileReader::I_PNG)
          m_reader = vtkPNGReader::New();
      else if (type == DICOMFileReader::I_BMP)
          m_reader = vtkBMPReader::New();
      else
          return false;

      m_reader->SetFileNames(nameList);
      m_volName = "Image Object";
      return true;
  }
}



bool DICOMFileReader::createVolume(QList<DICOMImageData*>* imgData) {
  // Check for the image data pointer
  if (!imgData) {
    std::cout << "Error: No Image Pointer!" << std::endl;
    return false;
  }
  // Check that there are actually elements in the pointer.
  if (imgData->count() <= 0) {
    std::cout << "Error: Number of images read is: " << imgData->count() << std::endl;
    return false;
  }

  int numZSlices, numFrames, numRow, numCol, volZSlices;
  int locIdx = 0; // Default is HFS.

  // Set up the patient position for the first file. Should be the same for all the others too!
  for (int ix1=0; ix1<NUM_ENTRIES; ix1++) {
    if ( imgData->at(0)->getPatientPosition() == ENTRY_STRINGS[ix1].c_str() ) {
      locIdx = ix1;
    }
  }

  // Get the number of rows and columns
  numRow = imgData->at(0)->getNumRows();
  numCol = imgData->at(0)->getNumCols();

  // Chack for cine. If there are multiple images per cycle then this is a movie.
  if ( imgData->at(0)->isCineData() ) {
    // Check if the number of images is devisible by the number of images in one cycle.
    if (imgData->count() % imgData->at(0)->getImagesPerCycle() != 0) {
      std::cout << "Error: Number of images: " << imgData->count() << " Not divisible by: " << imgData->at(0)->getImagesPerCycle() << std::endl;
      return false;
    }

    numFrames = imgData->at(0)->getImagesPerCycle();
    numZSlices = (int) (imgData->count() / imgData->at(0)->getImagesPerCycle());
  } else {
    numFrames = 1;
    numZSlices = imgData->count();
  }

  // Number of Z slices in the volume is the same as in the DICOM files EXCEPT when there is only one slice.
  // To be a valid volume the thrid dimension must be at least 2.
  if (numZSlices == 1) {
    volZSlices = 2;
  } else {
    volZSlices = numZSlices;
  }

  // Create the image data.
  m_vtkImgData->SetScalarTypeToShort();
  m_vtkImgData->SetDimensions(numRow, numCol, volZSlices);
  m_vtkImgData->SetNumberOfScalarComponents(numFrames);
  m_vtkImgData->AllocateScalars();


  // Get the position of the first slice.
  double pos[3];
  pos[0] = ENTRY_FLIPS[locIdx][0]*imgData->at(0)->getImagePosition(0);
  pos[1] = ENTRY_FLIPS[locIdx][1]*imgData->at(0)->getImagePosition(1);
  pos[2] = ENTRY_FLIPS[locIdx][2]*imgData->at(0)->getImagePosition(2);

  // Get the row and column orientations.
  double rowOrient[3];
  double colOrient[3];

  rowOrient[0] = ENTRY_FLIPS[locIdx][0]*imgData->at(0)->getImageOrientation(0, 0);
  rowOrient[1] = ENTRY_FLIPS[locIdx][1]*imgData->at(0)->getImageOrientation(0, 1);
  rowOrient[2] = ENTRY_FLIPS[locIdx][2]*imgData->at(0)->getImageOrientation(0, 2);

  colOrient[0] = ENTRY_FLIPS[locIdx][0]*imgData->at(0)->getImageOrientation(1, 0);
  colOrient[1] = ENTRY_FLIPS[locIdx][1]*imgData->at(0)->getImageOrientation(1, 1);
  colOrient[2] = ENTRY_FLIPS[locIdx][2]*imgData->at(0)->getImageOrientation(1, 2);

  // Calclulate the z vector as the cross product of the row and column vectors.
  double zVec[3];
  vtkMath::Cross(rowOrient, colOrient, zVec);

  double zspacing = 0.0;
  double negZed = 1;

  if (numZSlices > 1) {
    double xd, yd, zd;

    // At least two Z Slices
    xd = ENTRY_FLIPS[locIdx][0]*(imgData->at(0)->getImagePosition(0)-imgData->at(numFrames)->getImagePosition(0));
    yd = ENTRY_FLIPS[locIdx][1]*(imgData->at(0)->getImagePosition(1)-imgData->at(numFrames)->getImagePosition(1));
    zd = ENTRY_FLIPS[locIdx][2]*(imgData->at(0)->getImagePosition(2)-imgData->at(numFrames)->getImagePosition(2));
    zspacing = sqrt(xd*xd + yd*yd + zd*zd);

    // Check for a sane spacing...
    if (zspacing < 0.001) zspacing = 0.5;

    // Check which direction to put the slices in.
    double flipFlag = zVec[0]*xd+zVec[1]*yd+zVec[2]*zd;

    if (flipFlag < 0) {
      negZed = -1;
    } else {
      negZed = 1;
    }
  } else {
    // Only one slice so there really is no z spacing or z direction. Default to 1.
    zspacing = 1.0f;
    negZed = 1.0f;
  }

  // Set the spacing for the image data.
  m_vtkImgData->SetSpacing(imgData->at(0)->getPixelSpace(0), -imgData->at(0)->getPixelSpace(1), negZed*zspacing);

  short* temp;
  short* imgPtr;
  // Copy and the data
  for (int ix1=0; ix1<volZSlices; ix1++) {
    for (int row=0; row<imgData->at(0)->getNumRows(); row++) {
      for (int col=0; col<imgData->at(0)->getNumCols(); col++) {
        temp = (short*)m_vtkImgData->GetScalarPointer(col, row, ix1);
        for (int frame = 0; frame<numFrames; frame++) {
          imgPtr = imgData->at( (ix1%numZSlices)*numFrames+frame )->getDataPtr();
          *temp =  imgPtr[row*imgData->at(0)->getNumCols()+col];
          temp++;
        }
      }
    }
  }

  // Reset the spacing.
  m_infoFix->SetInput(m_vtkImgData);
  m_infoFix->CenterImageOff();
  m_infoFix->SetOutputSpacing(imgData->at(0)->getPixelSpace(0), imgData->at(0)->getPixelSpace(1), -1*negZed*zspacing);
  m_infoFix->Update();

  // Set the orientation
  m_matrix->SetElement(0, 0, rowOrient[0]);
  m_matrix->SetElement(1, 0, rowOrient[1]);
  m_matrix->SetElement(2, 0, rowOrient[2]);
  m_matrix->SetElement(0, 1, colOrient[0]);
  m_matrix->SetElement(1, 1, colOrient[1]);
  m_matrix->SetElement(2, 1, colOrient[2]);
  m_matrix->SetElement(0, 2, zVec[0]);
  m_matrix->SetElement(1, 2, zVec[1]);
  m_matrix->SetElement(2, 2, zVec[2]);

  // Set the position
  m_matrix->SetElement(0, 3, pos[0]);
  m_matrix->SetElement(1, 3, pos[1]);
  m_matrix->SetElement(2, 3, pos[2]);

  double angle = 0;
  // if we need to rotate right
  if ((locIdx == 5) || (locIdx == 6))
      angle = 90;
  // if we need to rotate left
  else if ((locIdx == 4) || (locIdx == 7))
      angle = -90;

   vtkTransform *t = vtkTransform::New();
   t->Identity();
   t->RotateZ(angle);
   t->Concatenate(m_matrix);
   t->Update();
   m_matrix->DeepCopy(t->GetMatrix());
   t->Delete();



  return true;
}

vtkImageData* DICOMFileReader::getImageData(bool isDICOM)
{
    if (isDICOM)
        return m_infoFix->GetOutput();
    else
    {
        m_reader->SetDataSpacing(m_spacingX,m_spacingY,m_spacingZ);
        m_reader->Update();
        vtkImageFlip *flip = vtkImageFlip::New();
        flip->SetFilteredAxis(1);
        flip->SetInputConnection(m_reader->GetOutputPort());
        flip->Update();
        return flip->GetOutput();
    }

}
