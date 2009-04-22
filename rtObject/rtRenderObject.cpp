#include "rtRenderObject.h"
#include "rtTimeManager.h"
#include "rtDataObject.h"

rtRenderObject::rtRenderObject() {
  m_dataObj = NULL;
  m_renderName = "None";
  m_objType = rtConstants::OT_None;
  m_treeItem = new QTreeWidgetItem();
  m_mainWin = NULL;
  m_visible3D = false;
}

rtRenderObject::~rtRenderObject() {
  if (m_treeItem) delete m_treeItem;
}

//! Get a version of the 3D pipeline that cannot be modified
QList<vtkProp*> const * const rtRenderObject::get3DPipeline() {
  return &m_pipe3D;
}

//! Get a version of the 2D pipeline that cannot be modified.
QList<vtkProp*> const * const rtRenderObject::get2DPipeline() {
  return &m_pipe2D;
}

rtDataObject* rtRenderObject::getDataObject() {
  return m_dataObj;
}

QString rtRenderObject::getName() {
  return m_renderName;
}

rtConstants::rtObjectType rtRenderObject::getObjectType() {
  return m_objType;
}

void rtRenderObject::setDataObject(rtDataObject* dataObj) {
  m_dataObj = dataObj;
}

void rtRenderObject::setName(QString renName) {
  m_renderName = renName;
}

void rtRenderObject::setObjectType(rtConstants::rtObjectType objType) {
  m_objType = objType;
}

void rtRenderObject::updateTreeItem() {
  if (m_treeItem) {
    m_treeItem->setText(0, m_dataObj->getObjName());
    m_treeItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    m_treeItem->setText(1, QString::number(m_dataObj->getId()));
    m_treeItem->setTextAlignment(1, Qt::AlignHCenter);
    if (!m_pipe3D.empty()) {
      m_treeItem->setText(2, "3D");
      m_treeItem->setCheckState(2,Qt::Unchecked); 
    } else {
      m_treeItem->setText(2, "NA");
    }
  }
}

void rtRenderObject::setVisible3D(bool v) {
  m_visible3D = v;
  if (v) {
    update();
    rtTimeManager::instance().addToWatchList(this);
  } else {
    rtTimeManager::instance().removeFromWatchList(this);
  }
}

//! Set the update time to the current time.
void rtRenderObject::resetUpdateTime() {
  m_lastUpdate = QDateTime::currentDateTime();
}

//! Check if an update is needed.
bool rtRenderObject::updateNeeded() {
  if(m_dataObj->getModified() > m_lastUpdate) {
    // Modified after the last update.
    return true;
  }
  return false;
}
