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
#ifndef DICOMFILEREADER_H
#define DICOMFILEREADER_H

#include <QList>
#include <QString>

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkImageChangeInformation.h"
#include "vtkSmartPointer.h"
#include "vtkMatrixToLinearTransform.h"

#include "DICOMImageData.h"
#include "DICOMCommonData.h"

  const int NUM_ENTRIES = 8;
  const std::string ENTRY_STRINGS[NUM_ENTRIES] = {"HFS", "FFS","HFP","FFP","HFDR", "FFDR", "HFDL", "FFDL"};
  const int ENTRY_FLIPS[NUM_ENTRIES][3] = {
    {1,1,1}, /* Head First Supine to RCS*/
    {-1,1,-1}, /* Feet First Supine to RCS */
    {-1,-1,1}, /* Head First Prone to RCS */
    {1,-1,-1}, /* Feet First Prone to RCS */
    {1,1,1},   /* Head First Decubitus Right before rotate */
    {-1,1,-1}, /*Feet First Decubitus Right before rotate */
    {1,1,1}, /*Head First Decubitus Left before rotate */
    {-1,1,-1} /*Feet First Decubitus Left before rotate */
  };

  class vtkImageReader2;

class DICOMFileReader
{
public:
  DICOMFileReader();
  ~DICOMFileReader();

  enum imageType {
    I_DICOM,
    I_JPEG,
    I_PNG,
    I_BMP
  };

  bool setDirectory(QString dirPath,imageType type);
  bool createVolume(QList<DICOMImageData*>* imgData);
  DICOMCommonData* getCommonDataHandle() { return m_ddata->getCommonDataHandle(); }

  vtkImageData* getImageData(bool isDICOM);
  vtkTransform* getTransform() { return m_transform; }

  QString getComments() { return m_comments; }
  QString getDefaultName() { return m_volName; }

  void cleanupImageData();
  
  //! Get the list of trigger delays.
  QList<double>* getTriggerList() { return &m_triggerList; }

  //! Get the list as DICOM image data.
  QList<DICOMImageData*>* getDICOMImageData() { return &m_imgData; }

  //! Set the spacing
  void setSpacing(double x, double y, double z)
  {
          m_spacingX = x;
          m_spacingY = y;
          m_spacingZ = z;
  }

  //! Get the spacing
 void getSpacing(double x, double y, double z)
 {
     x = m_spacingX;
     y = m_spacingY;
     z = m_spacingZ;
 }

protected:
  QList<QString> m_fileList;
  QList<DICOMImageData*> m_imgData;

  QList<double> m_triggerList;

  //! The vtk pipeline ready version of the image data.
  vtkSmartPointer<vtkImageData> m_vtkImgData;

  //! Matrix for the transform.
  vtkSmartPointer<vtkMatrix4x4> m_matrix;

  //! The object the converts between the vtkMatrix4x4 and the vtkTransform.
  vtkSmartPointer<vtkMatrixToLinearTransform> m_matrixToTransform;

  //! The transform for the data.
  vtkSmartPointer<vtkTransform> m_transform;

  vtkSmartPointer<vtkImageChangeInformation> m_infoFix;

  //! The DICOM IMAGE header.
  DICOMImageData* m_ddata;

  //! The output comments generated by the dicom reader about the file(s).
  QString m_comments;

  //! The default name of the volume
  QString m_volName;

  //! The X spacing of the image object
  double m_spacingX;
  //! The Y spacing of the image object
  double m_spacingY;
  //! The Z spacing of the image object
  double m_spacingZ;

  //! The image reader
  vtkImageReader2 *m_reader;
};

#endif // DICOMFILEREADER_H
