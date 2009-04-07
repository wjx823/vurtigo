#include "rtDataObject.h"

rtDataObject::rtDataObject() {
  m_renderOptions = NULL;
  m_readOnly = false;
}

rtDataObject::~rtDataObject() {
}


rtRenderOptions* rtDataObject::getRenderOptions() {
  return m_renderOptions;
}

void rtDataObject::setRenderOptions(rtRenderOptions* renOpt) {
  if (m_renderOptions != renOpt) {
    m_renderOptions = renOpt;
  }
}


int rtDataObject::getId() {
  return m_objId;
}

void rtDataObject::setId(int id) {
  m_objId = id;
}

  
QString rtDataObject::getObjName(){
  return m_objName;
}

void rtDataObject::setObjName(QString name) {
  m_objName = name;
}


rtConstants::rtObjectType rtDataObject::getObjectType() {
  return m_objType;
}

void rtDataObject::setObjectType(rtConstants::rtObjectType ot) {
  m_objType = ot;
}
