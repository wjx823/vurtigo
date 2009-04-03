#include "rtLabelRenderObject.h"
#include "rtLabelRenderOptions.h"
#include "rtLabelDataObject.h"

rtLabelRenderObject::rtLabelRenderObject() {
  setObjectType(rtConstants::OT_TextLabel);
  setName("Simple Label Renderer");
  setupDataObject();
  setupRenderOptions();
  setupPipeline();
}

rtLabelRenderObject::~rtLabelRenderObject() {
  if (m_dataObj) delete m_dataObj;
  if (m_renderObj) delete m_renderObj;

  if (m_textActor2D) m_textActor2D->Delete();
}

void rtLabelRenderObject::setupDataObject() {
  m_dataObj = new rtLabelDataObject();
  
}

void rtLabelRenderObject::setupRenderOptions() {
  m_renderObj = new rtLabelRenderOptions();
  m_dataObj->setRenderOptions(m_renderObj);
}

void rtLabelRenderObject::setupPipeline() {
  rtLabelDataObject *dObj = dynamic_cast<rtLabelDataObject*>(m_dataObj);

  m_textActor2D = vtkTextActor::New();
  m_textActor2D->SetInput(dObj->getText().toStdString().c_str());
  m_textActor2D->SetTextProperty(dObj->getTextProperty());

  // This should work the same way in both the 3D and 2D render windows. 
  m_pipe3D = m_textActor2D;
  m_pipe2D = m_textActor2D;
}
