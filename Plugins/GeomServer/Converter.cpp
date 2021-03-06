#include "Converter.h"

#include <iostream>

#include "rtBaseHandle.h"

#include "vtkImageData.h"
#include "vtkImageFlip.h"
#include "vtkImageChangeInformation.h"
#include "vtkMatrix4x4.h"

#define CONVERTER_MAX_IMAGE_SIZE 1024 * 1024 * 4

//For debugging purposes
//#ifndef CONVERTER_DEBUG
//#define CONVERTER_DEBUG
//#endif

using namespace std;

//! Prints a header and footer before and after the function is called
void Converter::printHeaderFooter(ConverterFunction function) {
  cout << "-------------------------------- VURTIGO START PRINT" << endl;
  //call the function on this instance
  (this->*function)();
  cout << "-------------------------------- VURTIGO END PRINT" << endl;
}

//! Prints all information
void Converter::printAll() {
  printAllLocalImages();
  printAllLocalCath();
}

//! Prints all local Cathater information
void Converter::printAllLocalCath() {
  cout << "Number of Cathaters: " << remoteCathMap->size() << endl;

  int cathCounter = 0;
  int coilCounter;
  QHash<int, rtCathDataObject::CathCoilData> * coils;
  rtCathDataObject::CathCoilData currCoil;
  for (NodeMap::iterator cathIt = remoteCathMap->begin(); cathIt != remoteCathMap->end(); cathIt++) {
    cout << "Cath Number: " << cathCounter++ << endl;

    //cast check
    coils = ((rtCathDataObject *) rtBaseHandle::instance().getObjectWithID(cathIt->first))->getListHandle();
    coilCounter = 0;
    for (QHash<int, rtCathDataObject::CathCoilData>::iterator coilIt = coils->begin(); coilIt != coils->end(); coilIt++) {
      currCoil = coilIt.value();
      cout << "Coil Number: " << coilCounter++
              << " locationID: " << currCoil.locationID;
      QList<QString> props = currCoil.coilPropValues.keys();
      for (int ix1=0; ix1<props.size(); ix1++)
      {
          cout << props[ix1].toStdString().c_str() << currCoil.coilPropValues.value(props[ix1]) << endl;
      }
      cout << "Coords: [" << currCoil.cx << ", " << currCoil.cy << ", " << currCoil.cz << "]" << endl;
      cout << "Angles: [" << currCoil.angles[0] << ", " << currCoil.angles[1] << "]" << endl;
    }
  }
}

//! Prints all local image information
void Converter::printAllLocalImages() {
  cout << "Number of Images: " << remoteImageMap->size() << endl;

  int imageCounter = 0;
  rt2DSliceDataObject * currImage;
  for (IdMap::iterator imageIt = remoteImageMap->begin(); imageIt != remoteImageMap->end(); imageIt++) {
    cout << "Image Number: " << imageCounter++ << endl;

    //cast check
    currImage = (rt2DSliceDataObject *) rtBaseHandle::instance().getObjectWithID(imageIt->first);
    cout << "Raw Data: "; currImage->getRawData()->Print(cout); cout << endl;
    cout << "Transform: "; currImage->getTransform()->Print(cout); cout << endl;
  }
}

Converter::Converter() {
  localCathMap = new NodeMap();
  remoteCathMap = new NodeMap();
  localImageMap = new IdMap();
  remoteImageMap = new IdMap();
}

Converter::~Converter() {
  delete localCathMap;
  delete remoteCathMap;
  delete localImageMap;
  delete remoteImageMap;
}


//! Returns a local object node, and will create a new one in the local and remote maps if the local object node doesn't exist and the parameters are availible
/*!
  @param id remote id of object in "localMap"
  @param localMap map to  search in
  @param remoteMap map to add new local object node, must not be NULL to be valid
  @param objType object type for new local object node, must not be UNDEFINED_RT_OBJECT_TYPE to be valid
  @param name name for new local object node, must not be NULL to be valid
  @return the local object node for the id and localMap, or the new obect node, or NULL
 */
Converter::Node * Converter::getLocalObjNode(int id, NodeMap * localMap, NodeMap * remoteMap, rtConstants::rtObjectType objType, const char * name) {
#ifdef CONVERTER_DEBUG
  cout << "Converter::getLocalObjNode()" << endl;
#endif
  Node * node = getNode(id, localMap);
  if (node == NULL && remoteMap != NULL && objType != UNDEFINED_RT_OBJECT_TYPE && name != NULL) {
      //if parameters are there and the object doesn't exist, add new object
      rtDataObject * obj = rtBaseHandle::instance().getObjectWithID(rtBaseHandle::instance().requestNewObject(objType, name));
      node = new Node(id);
      remoteMap->insert(NodePair(obj->getId(), node));
      node = new Node(obj->getId());
      localMap->insert(NodePair(id, node));
  }
  #ifdef CONVERTER_DEBUG
    cout << "Converter::getLocalObjNode() out" << endl;
  #endif
  return node;
}

//! Given the parameters, it searches in the node map with the id. Returns NULL if not found.
Converter::Node * Converter::getNode(int id, NodeMap * nodeMap) {
  NodeMap::iterator foundNode = nodeMap->find(id);
  if (foundNode != nodeMap->end()) {
#ifdef CONVERTER_DEBUG
        cout << "Converter::getNode() - found" << endl;
#endif
      return foundNode->second;
  }
#ifdef CONVERTER_DEBUG
      cout << "Converter::getNode() - not found" << endl;
#endif
  return NULL;
}

//! Return the remote cathater id, given the local id. Returns NULL_ID if not found
int Converter::getRemoteCathId(int localId) {
  Node * node = getNode(localId, remoteCathMap);
  if (node != NULL) {
    #ifdef CONVERTER_DEBUG
      cout << "Converter::getRemoteCathId() - found" << endl;
    #endif
    return node->currId;
  }
  else {
    #ifdef CONVERTER_DEBUG
      cout << "Converter::getRemoteCathId() - not found" << endl;
    #endif
    return NULL_ID;
  }
}

//! Given a local cathater id and local coil id, it returns the local coil id. Returns NULL_ID if not found
int Converter::getRemoteCoilId(int localCathId, int localCoilId) {
  //find the node for the remote cath
  Node * node = getNode(localCathId, remoteCathMap);
  if (node != NULL) {
    //if remote cath is found, search its NodeMap for the coil
    NodeMap::iterator foundNode = node->nodeMap->find(localCoilId);
    if (foundNode != node->nodeMap->end()) {
      return foundNode->second->currId;
    }
  }
  return NULL_ID;
}

//! Returns the local cathater object, geven a remote id. Returns NULL of not found.
rtCathDataObject * Converter::getLocalCath(int remoteId) {
  Node * node = getLocalObjNode(remoteId, localCathMap, remoteCathMap, rtConstants::OT_Cath, CATH_OBJ_NAME);
  rtDataObject * obj = NULL;
  if (node != NULL)
    obj = rtBaseHandle::instance().getObjectWithID(node->currId);

  //Cast Check?
  return (rtCathDataObject *) obj;
}

//! Given a remote coil id and a local catherater object, this method searchs for he local coil id.
/*!
  If it doesn't exist it uses the remote coil local to add a new one or returns NULL for failure.
*/
int Converter::getLocalCoilId(int remoteCoilId, rtCathDataObject * localCath, int remoteCoilLoc) {
#ifdef CONVERTER_DEBUG
  cout << "Converter::getLocalCoilId()" << endl;
#endif
  int remoteCathId = getRemoteCathId(localCath->getId());

  //remote cath not found
  if (remoteCathId == NULL_ID) {
    #ifdef CONVERTER_DEBUG
      cout << "Converter::getLocalCoilId() not remote cath, return null id: " << NULL_ID << endl;
    #endif
    return NULL_ID;
  }

  //get node for the local cath to get the corresponding local coil map
  Node * localCathNode = getNode(remoteCathId, localCathMap);
  if (localCathNode == NULL) {
    cerr << "Converter::getLocalCoilId() internal bug localCathMap and remoteCathMap don't match, return null id " << NULL_ID << endl;
    return NULL_ID;
  }

  //search local coil map to get the local coil
  NodeMap::iterator localCoilNode = localCathNode->nodeMap->find(remoteCoilId);
  if (localCoilNode != localCathNode->nodeMap->end()) {
    //found
    #ifdef CONVERTER_DEBUG
      cout << "Converter::getLocalCoilId() return found coil: " << localCoilNode->first << endl;
    #endif
    return localCoilNode->first;
  }
  else if (remoteCoilLoc != UNDEFINED_REMOTE_COIL_LOC) {
    //add a new local coil and return the coil id
    int localCoilId = localCath->addCoil(remoteCoilLoc);
    //add to localCathMap
    localCathNode->nodeMap->insert(NodePair(remoteCoilId, new Node(localCoilId)));
    //add to remote Cath Map
    getNode(localCath->getId(), remoteCathMap)->nodeMap->insert(NodePair(localCoilId, new Node(remoteCoilId)));

    #ifdef CONVERTER_DEBUG
      cout << "Converter::getLocalCoilId() return new coil: " << localCoilId << endl;
    #endif
    return localCoilId;
  }
  #ifdef CONVERTER_DEBUG
    cout << "Converter::getLocalCoilId() return null coil: " << NULL_ID << endl;
  #endif
  return NULL_ID;
}

//! Sets the value of the local cathater object, given a remote cathater object. Returns true of successful.
bool Converter::setLocalCath(CATHDATA & remote, rtCathDataObject * local) {
  #ifdef CONVERTER_DEBUG
    cout << "Converter::setLocalCath()" << endl;
  #endif
  int localCoilId;

  bool success = true;
  //for all remote coils
  int coilIndex = 0;
  
  local->addCathProperty("SNR");

  for (vector<COILDATA>::iterator currCoil = remote.coils.begin(); currCoil != remote.coils.end(); currCoil++) {
    #ifdef CONVERTER_DEBUG
      cout << "loop iteration: " << coilIndex << endl;
    #endif
    //get the local coil id to search with
    localCoilId = getLocalCoilId(coilIndex, local, currCoil->locID);

    //no local coil id found
    if (localCoilId == NULL_ID) continue;

    //set the local values
    local->lock();
    success = success && local->setCoilAngles(localCoilId, currCoil->angles[0], currCoil->angles[1]);
    success = success && local->setCoilCoords(localCoilId, currCoil->coords[0], -currCoil->coords[1], -currCoil->coords[2]);
    success = success && local->setCoilPropValue(localCoilId,"SNR",currCoil->SNR);
    local->unlock();

    coilIndex++;
  }
  #ifdef CONVERTER_DEBUG
    cout << "Converter::setLocalCath() return " << success << endl;
  #endif

  //if not successful, do not call local->Modified, and return
  if (!success)
      return success;

  local->Modified();
  return success;
}

//! Retruns a local image object with the matching remoteId, if one doesn't exist. One is a local image object is created.
rt2DSliceDataObject * Converter::getLocalImage(int remoteId, int imageSize) {
  IdMap::iterator foundId = localImageMap->find(remoteId);

  rtDataObject * obj = NULL;
  if (foundId != localImageMap->end()) {
    obj = rtBaseHandle::instance().getObjectWithID(foundId->second);
  }
  else if (imageSize <= CONVERTER_MAX_IMAGE_SIZE && imageSize > 0) {
    int localId = rtBaseHandle::instance().requestNewObject(rtConstants::OT_2DObject, IMAGE_OBJ_NAME);
    localImageMap->insert(IdPair(remoteId, localId));
    remoteImageMap->insert(IdPair(localId, remoteId));
    obj = rtBaseHandle::instance().getObjectWithID(localId);
  }
  return (rt2DSliceDataObject *) obj;
}

vtkImageData * Converter::CopyImage(IMAGEDATA &remote)
  {
    vtkImageData *img = vtkImageData::New();
    
    if (img == NULL)
      return NULL;

    double spacing = ((double)remote.FOV*10)/((double)remote.imgSize);

    img->Initialize();

    if (remote.numChannels == 1)
      {
        if (remote.numBytesPerPixel == 1)
          img->SetScalarTypeToUnsignedChar();
        else
          img->SetScalarTypeToUnsignedShort();
      }
    else
      img->SetScalarTypeToUnsignedChar();


    img->SetSpacing(spacing, spacing, 1.0);
    img->SetOrigin(0.0,0.0,0.0);
    img->SetDimensions(remote.imgSize,remote.imgSize,1);
    if (remote.numChannels >= 3) {
      img->SetNumberOfScalarComponents(3);
    } else {
      img->SetNumberOfScalarComponents(1);
    }
    img->SetWholeExtent(0, remote.FOV*10, 0, remote.FOV*10,0,1);
    img->AllocateScalars();

    // Copy the data
    if (remote.numChannels == 1)
      {
        if (remote.numBytesPerPixel == 1)
          { // gray8 
            int srcPos = 0;
            for (int ix1 = 0; ix1 < remote.imgSize; ix1++) {
              for (int ix2 = 0; ix2 < remote.imgSize; ix2++) {  
                unsigned char *dstPixel = static_cast<unsigned char*>(img->GetScalarPointer( remote.imgSize-ix2-1,remote.imgSize-ix1-1, 0));
                *dstPixel = remote.img[srcPos++];
              }
            }
          }
        else
          { // gray16 (new code)
            int srcPos = 0;
            for (int ix1 = 0; ix1 < remote.imgSize; ix1++) {
              for (int ix2 = 0; ix2 < remote.imgSize; ix2++) {  
                short *dstPixel = static_cast<short*>(img->GetScalarPointer( remote.imgSize-ix2-1, remote.imgSize-ix1-1, 0));
                *dstPixel = ((short *)remote.img)[srcPos++];
                }
              }
          }
      }
    else
      { // rgba8 
        int srcPos = 0;
        for (int ix1 = 0; ix1 < remote.imgSize; ix1++) {
          for (int ix2 = 0; ix2 < remote.imgSize; ix2++) {  
            unsigned char *dstPixel = static_cast<unsigned char*>(img->GetScalarPointer( remote.imgSize-ix2-1,remote.imgSize-ix1-1, 0));

           // rearrange color data
           //   QT QImage RGBA8 format is actually 0xxAARRGGBB, or BB GG RR AA
           //   vtkImageData 3-channel format is RR GG BB
            dstPixel[0] = remote.img[srcPos + 2]; // r
            dstPixel[1] = remote.img[srcPos + 1]; // g
            dstPixel[2] = remote.img[srcPos + 0]; // b

            srcPos += 4;
          }
        }
      }

    img->Update();
    
    return img;
  }

//! Sets the value of the local image object, given a remote image object. Returns true of successful.
bool Converter::setLocalImage(IMAGEDATA & remote, rt2DSliceDataObject * local) {
  if (remote.FOV <= 0 || remote.imgSize <= 0 || remote.numChannels <= 0 || remote.numBytesPerPixel <= 0) return false;

  bool success = true;

  vtkImageData *img = CopyImage(remote);
  
  vtkImageChangeInformation *imgChangeInfo = vtkImageChangeInformation::New();

  imgChangeInfo->SetInput(img);
  imgChangeInfo->Update();

  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

  for (int ix1=0; ix1<9; ix1++) {
    matrix->SetElement(ix1/3, ix1%3, remote.rotMatrix[ix1]);

  }

  double ptIn[4], ptOut[4];
  ptIn[0] = (((double)remote.FOV*10)/2.0f);
  ptIn[1] = (((double)remote.FOV*10)/2.0f);
  ptIn[2] = 0.0f;
  ptIn[3] = 1.0f;

  matrix->MultiplyPoint(ptIn, ptOut);

  for (int ix1=0; ix1<9; ix1++) {
    if (ix1/3>0)  matrix->SetElement(ix1/3, ix1%3, -remote.rotMatrix[ix1]);
  }

  matrix->SetElement(0, 3, -(ptOut[0]-remote.transMatrix[0]));
  matrix->SetElement(1, 3, ptOut[1]-remote.transMatrix[1]);
  matrix->SetElement(2, 3, ptOut[2]-remote.transMatrix[2]);

  //imgChangeInfo->GetOutput()->Print(std::cout);
  //matrix->Print(std::cout);

  local->lock();
  // Translate the FOV into milimeters
  success = success && local->copyImageData2D(imgChangeInfo->GetOutput());
  success = success && local->setVtkMatrix(matrix);
  local->unlock();

  //if successful, call local->Modified
  if (success) local->Modified();

  img->Delete();
  imgChangeInfo->Delete();
  matrix->Delete();

  return success;
}


//! Sets the value of the local image object, BUT NOT THE TRANSFORM. Returns true of successful.
bool Converter::setLocalImageOnly(IMAGEDATA & remote, rt2DSliceDataObject * local) {
  if (remote.FOV <= 0 || remote.imgSize <= 0 || remote.numChannels <= 0) return false;

  bool success = true;

  vtkImageData *img = CopyImage(remote);

  vtkImageChangeInformation *imgChangeInfo = vtkImageChangeInformation::New();

  imgChangeInfo->SetInput(img);
  imgChangeInfo->Update();

  local->lock();
  // Translate the FOV into milimeters
  success = success && local->copyImageData2D(imgChangeInfo->GetOutput());
  local->unlock();

  //if successful, call local->Modified
  if (success) local->Modified();

  img->Delete();
  imgChangeInfo->Delete();

  return success;
}


bool Converter::setRemoteImageTransform(std::vector<IMAGEDATA> & remoteImages) {
  rt2DSliceDataObject * localImage;
  bool success = true;

  if (remoteImages.size() == 0) return false;

  int imageIndex = 0;
  for (vector<IMAGEDATA>::iterator it = remoteImages.begin(); it != remoteImages.end(); it++) {
    localImage = getLocalImage(imageIndex, (*it).imgSize * (*it).imgSize * (*it).numChannels);
    imageIndex++;
    //if image is invalid, continue
    if (localImage == NULL) continue;

    vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

    localImage->lock();
    matrix->DeepCopy(localImage->getTransform()->GetMatrix());
    localImage->unlock();

    // First isolate the translation
    double translate[3];

    // Negate the X value immediately
    translate[0] = -matrix->GetElement(0, 3);
    translate[1] = matrix->GetElement(1, 3);
    translate[2] = matrix->GetElement(2, 3);

    matrix->SetElement(0, 3, 0.0f);
    matrix->SetElement(1, 3, 0.0f);
    matrix->SetElement(2, 3, 0.0f);

    // Negate the bottom rows.
    for (int ix1=3; ix1<9; ix1++) {
      matrix->SetElement(ix1/3, ix1%3, -matrix->GetElement(ix1/3, ix1%3));
    }

    // Copy the transformation matrix.
    for (int ix1=0; ix1<9; ix1++) {
      (*it).rotMatrix[ix1] = matrix->GetElement(ix1/3, ix1%3);
    }

    double ptIn[4], ptOut[4];
    ptIn[0] = (((double)(*it).FOV*10)/2.0f);
    ptIn[1] = (((double)(*it).FOV*10)/2.0f);
    ptIn[2] = 0.0f;
    ptIn[3] = 1.0f;

    matrix->MultiplyPoint(ptIn, ptOut);

    (*it).transMatrix[0] = -(translate[0]-ptOut[0]);
    (*it).transMatrix[1] = -(translate[1]-ptOut[1]);
    (*it).transMatrix[2] = -(translate[2]-ptOut[2]);
    //matrix->Print(std::cout);
    matrix->Delete();
  }

  return success;
}
