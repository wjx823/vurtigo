/*******************************************************************************
    Vurtigo: Visualization software for interventional applications in medicine
    Copyright (C) 2009 Sunnybrook Health Sciences Centre

    This file is part of Vurtigo.

    Vurtigo is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#ifndef RT_IMAGE_BUFFER_OBJECT_H
#define RT_IMAGE_BUFFER_OBJECT_H

#include "rtDataObject.h"

//! Object that represents a set of 2D images
/*!
  @todo Implement this class
  */
class rtImageBufferDataObject : public rtDataObject
{
Q_OBJECT

public:
  rtImageBufferDataObject();
  ~rtImageBufferDataObject();

  void update();

 protected:
  // Functions
  void setupGUI();
  void cleanupGUI();
  
};

#endif 
