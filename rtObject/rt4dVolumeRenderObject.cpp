#include "rt4dVolumeRenderObject.h"
#include "rt4dVolumeDataObject.h"

rt4DVolumeRenderObject::rt4DVolumeRenderObject() {
  setObjectType(rtConstants::OT_4DObject);
  setName(" Renderer");
  setupDataObject();
  setupPipeline();
}


rt4DVolumeRenderObject::~rt4DVolumeRenderObject() {
}


//! Take info from the data object. 
void rt4DVolumeRenderObject::update() {
}

//! Add this object to the given renderer.
bool rt4DVolumeRenderObject::addToRenderer(vtkRenderer* ren) {
  if (!ren) return false;
  setVisible3D(true);

}

//! Remove this object from the given renderer.
bool rt4DVolumeRenderObject::removeFromRenderer(vtkRenderer* ren) {
  if (!ren) return false;
  setVisible3D(false);

}

//! Create the correct data object.
void rt4DVolumeRenderObject::setupDataObject() {
  m_dataObj = new rt4DVolumeDataObject();
}


//! Create the part of the pipeline that is done first. 
void rt4DVolumeRenderObject::setupPipeline() {
}

//! When this object is implemented it will have a valid location. For now it does not.
bool rt4DVolumeRenderObject::getObjectLocation(double loc[6]) {
  loc[0] = 0.0;
  loc[1] = 0.0;
  loc[2] = 0.0;
  loc[3] = 0.0;
  loc[4] = 0.0;
  loc[5] = 0.0;

  return false;
}
