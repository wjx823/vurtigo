#include "genericMode.h"
#include <iostream>

using namespace std;

GenericMode::GenericMode():m_sender(0), m_lastClientIndex(0) {
  int ix1=0; 

  m_imgDataArray.clear();
  m_cathDataArray.clear();

  // Set the translation to zero. 
  for (ix1=0;  ix1<TRANS_MATRIX_SIZE; ix1++) {
    m_volTransData[ix1] = 0;
  }

}

GenericMode::~GenericMode() {
}

void GenericMode::print() {
  int ix1;

  cout << "-------------------------------- START PRINT" << endl;
  cout << "Last client index: " << m_lastClientIndex << endl;

  cout << "PlaneID: " << m_planeID << endl;

  for (ix1=0; ix1<m_imgDataArray.size(); ix1++) {
    cout << "-- Plane with ID: "  << ix1 << endl;
    cout << "Respiratory phase: " <<  m_imgDataArray[ix1].resp << endl;
    cout << "Trigger delay: " << m_imgDataArray[ix1].trig << endl;
    cout << "FOV: " << m_imgDataArray[ix1].FOV << " cm"  << endl;
    cout << "Rotation: [" << m_imgDataArray[ix1].rotMatrix[0] << " " << m_imgDataArray[ix1].rotMatrix[1] << " " << m_imgDataArray[ix1].rotMatrix[2] << " ]" << endl;
    cout << "          [" << m_imgDataArray[ix1].rotMatrix[3] << " " << m_imgDataArray[ix1].rotMatrix[4] << " " << m_imgDataArray[ix1].rotMatrix[5] << " ]" << endl;
    cout << "          [" << m_imgDataArray[ix1].rotMatrix[6] << " " << m_imgDataArray[ix1].rotMatrix[7] << " " << m_imgDataArray[ix1].rotMatrix[8] << " ]" << endl;
    cout << "Translation: [" << m_imgDataArray[ix1].transMatrix[0] << " " << m_imgDataArray[ix1].transMatrix[1] << " " << m_imgDataArray[ix1].transMatrix[2] << " ]" << endl;
    cout << "Image size = " << m_imgDataArray[ix1].imgSize << endl;
    cout << "Number of image color channels = " << m_imgDataArray[ix1].numChannels << endl;
  }

  for (ix1=0; ix1<m_cathDataArray.size(); ix1++) {
    cout << "Number of catheter coils: " << m_cathDataArray[ix1].numCoils << endl;
    cout << "Catheter mode: " << m_cathDataArray[ix1].cathMode << endl;
    if (NO_CATHETERS != static_cast<CatheterMode>(m_cathDataArray[ix1].cathMode)) {
      for (int coil = 0; coil < m_cathDataArray[ix1].numCoils; coil++) {
	cout << "Cath coil = " << coil <<" locID = " << m_cathDataArray[ix1].coils[coil].locID << " SNR = " << m_cathDataArray[ix1].coils[coil].SNR <<  endl;
	cout << "Coords: [" <<  m_cathDataArray[ix1].coils[coil].coords[0] << ", " 
	     <<  m_cathDataArray[ix1].coils[coil].coords[1] << ", " 
	     <<  m_cathDataArray[ix1].coils[coil].coords[2] << "]" << endl;
	cout << "Angles: [" << m_cathDataArray[ix1].coils[coil].angles[0] << ", " 
	     <<  m_cathDataArray[ix1].coils[coil].angles[1] << "]" << endl;
      } 
    }
  }


  cout << "Volume translaton: [" << m_volTransData[0] << " " << m_volTransData[1] <<  " " <<  m_volTransData[2] << " ]" << endl;
  cout << "-------------------------------- END PRINT" << endl;
}


void GenericMode::getAllGeom() {
  IMAGEDATA* currImg;
  CATHDATA* currCath;
  COILDATA* coilDat;
  float *rotation, *translation;
  float* volTrans;

  // Catheter data temp arrays
  float *coords;
  int *angles;

  int ix1;

  // Always use the first catheter. 
  currCath = &m_cathDataArray[0];

  // The last client to write.
  m_lastClientIndex = m_sender->getLastClientWrite();

  // The gometry plane.
  m_planeID = m_sender->getPlaneID();
  
  currImg = &m_imgDataArray[m_planeID];

  currImg->trig = m_sender->getTrig();
  currImg->resp = m_sender->getResp();
  currImg->FOV = m_sender->getFOV();
  rotation = m_sender->getImgRotation();
  translation = m_sender->getImgTranslation();
  currImg->imgSize = m_sender->getImgSize();
  currImg->numChannels = m_sender->getNumChan();

  for (ix1=0; ix1<ROT_MATRIX_SIZE; ix1++) {
    currImg->rotMatrix[ix1] = rotation[ix1];
  }

  for (ix1=0; ix1<TRANS_MATRIX_SIZE; ix1++) {
    currImg->transMatrix[ix1] = translation[ix1];
  }
  
  // The volume translation.
  volTrans = m_sender->getVolTranslation();

  for (ix1=0; ix1<TRANS_MATRIX_SIZE; ix1++) {
    m_volTransData[ix1] = volTrans[ix1];
  }

  // Get catheter information.
  currCath->cathMode = m_sender->getCathModeAsInt();
  if (NO_CATHETERS != static_cast<CatheterMode>(currCath->cathMode)) {
    currCath->numCoils = m_sender->getNumCathCoils();
    currCath->coils.clear();
    for (ix1=0; ix1<currCath->numCoils; ix1++) {
      coilDat->locID = m_sender->getCathLocID(ix1);
      coilDat->SNR = m_sender->getCathSNR(ix1);
      coords = m_sender->getCathCoords(ix1);
      angles = m_sender->getCathAngles(ix1);

      memcpy(coilDat->coords, coords, COORDS_SIZE*sizeof(float));
      memcpy(coilDat->angles, angles, ANGLES_SIZE*sizeof(float));

      currCath->coils.push_back(*coilDat);      
    }
  }
}
