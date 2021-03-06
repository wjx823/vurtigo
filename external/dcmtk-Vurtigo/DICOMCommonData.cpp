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
// Qt
#include <QtGlobal>

//DCMTK
#ifdef Q_OS_UNIX
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

// for backwards compatibility with DCMTK
#define DCM_PATIENT_NAME DcmTagKey(0x0010, 0x0010)

#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofstring.h"
#include "dcmtk/dcmdata/dcdatset.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/ofstd/ofcond.h"
#include "dcmtk/dcmdata/dctagkey.h"

// Local
#include "DICOMCommonData.h"


DICOMCommonData::DICOMCommonData()
 : m_warning(false), m_error(false)
{

  // Set defaults for parameters
  m_patientsName = "ANONYMOUS";
  m_studyDate = QDate::fromString("19700101", "yyyymmdd");
  m_studyTime = QTime::fromString("010101", "hhmmss");
  m_patientPosition = "HFS";
  m_modality = "";
  m_manufacturer = "";
  m_studyID = "000000";
  m_seriesNumber = "0";

  m_numRows = 0;
  m_numCols = 0;

  m_pixSpace[0] = 1.0;
  m_pixSpace[1] = 1.0;

  m_imgPosition[0] = 0.0;
  m_imgPosition[1] = 0.0;
  m_imgPosition[2] = 0.0;

  m_imgOrient[0] = 1.0;
  m_imgOrient[1] = 0.0;
  m_imgOrient[2] = 0.0;
  m_imgOrient[3] = 0.0;
  m_imgOrient[4] = 1.0;
  m_imgOrient[5] = 0.0;
}


void DICOMCommonData::readData(DcmDataset* datSet) {
  OFString temp;

  // Get the patient name.
  if (!datSet->findAndGetOFString(DCM_PATIENT_NAME, temp).good()) {
    m_patientsName = "ANONYMOUS";
  } else {
    m_patientsName = temp.c_str();
  }

  // Get the study date.
  if (!datSet->findAndGetOFString(DCM_StudyDate, temp).good()) {
    m_warning = true;
    // yyyyMMdd
    m_studyDate = QDate::fromString("19700101", "yyyyMMdd");
  } else {
    m_studyDate = QDate::fromString(QString(temp.c_str()), "yyyyMMdd");
  }

  // Get the study time.
  if (!datSet->findAndGetOFString(DCM_StudyTime, temp).good()) {
    m_warning = true;
    // hhmmss
    m_studyTime = QTime::fromString("010101", "hhmmss");
  } else {
    m_studyTime = QTime::fromString(QString(temp.c_str()), "hhmmss");
  }

  if (!datSet->findAndGetOFString(DCM_PatientPosition, temp).good()) {
    m_warning = true;
    // Default to head-first supine.
    m_patientPosition = "HFS";
  } else {
    m_patientPosition = temp.c_str();
  }

  // Modality is required.
  if (!datSet->findAndGetOFString(DCM_Modality, temp).good()) {
    m_error = true;
  } else {
    m_modality = temp.c_str();
  }

  // Manufacturer is required.
  if(!datSet->findAndGetOFString(DCM_Manufacturer, temp).good()) {
    m_error = true;
  } else {
    m_manufacturer = temp.c_str();
  }

  // Study ID
  if (!datSet->findAndGetOFString(DCM_StudyID, temp).good()) {
    m_studyID = "000000";
    m_warning = true;
  } else {
    m_studyID = temp.c_str();
  }

  // Series Number
  if (!datSet->findAndGetOFString(DCM_SeriesNumber, temp).good()) {
    m_seriesNumber = "0";
    m_warning = true;
  } else {
    m_seriesNumber = temp.c_str();
  }

  // Number of rows is required.
  if ( !datSet->findAndGetUint16(DCM_Rows, m_numRows).good() ) {
    m_error = true;
  }

  // Number of columns is required
  if ( !datSet->findAndGetUint16(DCM_Columns, m_numCols).good() ) {
    m_error = true;
  }

  // Image pixel spacing.
  for (int ix1=0; ix1<2; ix1++) {
    if( !datSet->findAndGetFloat64(DCM_PixelSpacing, m_pixSpace[ix1], ix1).good() ) {
      m_pixSpace[ix1] = 1.0;
      m_warning = true;
    }
  }

  // Image position
  for (int ix1=0; ix1<3; ix1++) {
    if( !datSet->findAndGetFloat64(DCM_ImagePositionPatient, m_imgPosition[ix1], ix1).good() ) {
      m_error = true;
    }
  }

  // Image direction cosines.
  for (int ix1=0; ix1<6; ix1++) {
    if( !datSet->findAndGetFloat64(DCM_ImageOrientationPatient, m_imgOrient[ix1], ix1).good() ) {
      m_error = true;
    }
  }

}


void DICOMCommonData::deepCopy(DICOMCommonData* data) {
  m_warning = data->isWarning();
  m_error = data->isError();

  setPatientName(data->getPatientName());
  setStudyDate(data->getStudyDate());
  setStudyTime(data->getStudyTime());
  setPatientPosition(data->getPatientPosition());
  setModality(data->getModality());
  setManufacturer(data->getManufacturer());
  setStudyID(data->getStudyID());
  setSeriesNumber(data->getSeriesNumber());

  setNumRows(data->getNumRows());
  setNumCols(data->getNumCols());

  data->getPixelSpacing(m_pixSpace);
  data->getImagePosition(m_imgPosition);
  data->getImageOrientation(m_imgOrient);
}


void DICOMCommonData::getPixelSpacing(double pixSpace[2]) {
  pixSpace[0] = m_pixSpace[0];
  pixSpace[1] = m_pixSpace[1];
}

void DICOMCommonData::getImagePosition(double imgPosition[3]) {
  imgPosition[0] = m_imgPosition[0];
  imgPosition[1] = m_imgPosition[1];
  imgPosition[2] = m_imgPosition[2];
}

void DICOMCommonData::getImageOrientation(double imgOrient[6]) {
  imgOrient[0] = m_imgOrient[0];
  imgOrient[1] = m_imgOrient[1];
  imgOrient[2] = m_imgOrient[2];
  imgOrient[3] = m_imgOrient[3];
  imgOrient[4] = m_imgOrient[4];
  imgOrient[5] = m_imgOrient[5];
}

void DICOMCommonData::setPixelSpacing(double pixSpace[2]) {
  m_pixSpace[0] = pixSpace[0];
  m_pixSpace[1] = pixSpace[1];
}

void DICOMCommonData::setImagePosition(double imgPosition[3]) {
  m_imgPosition[0] = imgPosition[0];
  m_imgPosition[1] = imgPosition[1];
  m_imgPosition[2] = imgPosition[2];
}

void DICOMCommonData::setImageOrientation(double imgOrient[6]) {
  m_imgOrient[0] = imgOrient[0];
  m_imgOrient[1] = imgOrient[1];
  m_imgOrient[2] = imgOrient[2];
  m_imgOrient[3] = imgOrient[3];
  m_imgOrient[4] = imgOrient[4];
  m_imgOrient[5] = imgOrient[5];
}
