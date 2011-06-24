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
#include "rtPieceFuncRenderObject.h"
#include "rtPieceFuncDataObject.h"

rtPieceFuncRenderObject::rtPieceFuncRenderObject() {
  setObjectType("OT_vtkPiecewiseFunction");
  setName("Piecewise Function Renderer");
  setupDataObject();
  setupPipeline();
}


rtPieceFuncRenderObject::~rtPieceFuncRenderObject() {
}


//! Take info from the data object. 
void rtPieceFuncRenderObject::update() {
}

//! Add this object to the given renderer.
bool rtPieceFuncRenderObject::addToRenderer(vtkRenderer* ren,int window) {
  if (!ren) return false;

  return true;
}

//! Remove this object from the given renderer.
bool rtPieceFuncRenderObject::removeFromRenderer(vtkRenderer* ren,int window) {
  if (!ren) return false;

  return true;
}

//! Create the correct data object.
void rtPieceFuncRenderObject::setupDataObject() {
  m_dataObj = new rtPieceFuncDataObject();
}


//! Create the part of the pipeline that is done first. 
void rtPieceFuncRenderObject::setupPipeline() {
}

//! This object has no valid 3D location.
bool rtPieceFuncRenderObject::getObjectLocation(double loc[6]) {
  loc[0] = 0.0;
  loc[1] = 0.0;
  loc[2] = 0.0;
  loc[3] = 0.0;
  loc[4] = 0.0;
  loc[5] = 0.0;

  return false;
}
