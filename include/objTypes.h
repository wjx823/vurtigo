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
#ifndef OBJ_TYPES_H
#define OBJ_TYPES_H

#include <QString>

//! A vurtigo container class for constants such as object types.
class rtConstants {

  public:
  //! An enumeration of the different types of objects
  enum rtObjectType {
    OT_None,
    OT_3DObject,
    OT_2DObject,
    OT_Cath,
    OT_vtkMatrix4x4,
    OT_vtkPolyData,
    OT_vtkPiecewiseFunction,
    OT_vtkColorTransferFunction,
    OT_ImageBuffer,
    OT_2DPointBuffer,
    OT_3DPointBuffer,
    OT_TextLabel,
    OT_EPMesh,
    OT_2DPlot
  };

  //! Convert the object type into an int.
  /*!
    \param type The object type as an enum.
    \return The same value but as an integer.
    */
  static int objectTypeToInt(rtObjectType type) {
    return (int)type;
  }

  //! Convert the object type into a QString.
  /*!
    \param type The object type as an enum.
    \return The same value but as an integer.
    */
  static QString objectTypeToQString(rtObjectType type)
  {
      QString result;
      switch (type) {
        case OT_None:
          result = "OT_None";
          break;
        case OT_3DObject:
          result = "OT_3DObject";
          break;
        case OT_2DObject:
          result = "OT_2DObject";
          break;
        case OT_Cath:
          result = "OT_Cath";
          break;
        case OT_vtkMatrix4x4:
          result = "OT_vtkMatrix4x4";
          break;
        case OT_vtkPolyData:
          result = "OT_vtkPolyData";
          break;
        case OT_vtkPiecewiseFunction:
          result = "OT_vtkPiecewiseFunction";
          break;
        case OT_vtkColorTransferFunction:
          result = "OT_vtkColorTransferFunction";
          break;
        case OT_ImageBuffer:
          result = "OT_ImageBuffer";
          break;
        case OT_2DPointBuffer:
          result = "OT_2DPointBuffer";
          break;
        case OT_3DPointBuffer:
          result = "OT_3DPointBuffer";
          break;
        case OT_TextLabel:
          result = "OT_TextLabel";
          break;
        default:
          result = "OT_None";
          break;
      }
      return result;
  }

  //! Convert an int into a variable of type rtObjectType.
  /*!
    \param type The object type as an integer.
    \return The same value but cast as an object type. Will return OT_None if the integer value is not valid.
    */
  static rtObjectType intToObjectType(int type) {
    rtObjectType result;
    switch (type) {
      case OT_None:
      case OT_3DObject:
      case OT_2DObject:
      case OT_Cath:
      case OT_vtkMatrix4x4:
      case OT_vtkPolyData:
      case OT_vtkPiecewiseFunction:
      case OT_vtkColorTransferFunction:
      case OT_ImageBuffer:
      case OT_2DPointBuffer:
      case OT_3DPointBuffer:
      case OT_TextLabel:
      case OT_EPMesh:
      case OT_2DPlot:
      result = (rtObjectType)type;
      break;
      default:
      result = OT_None;
      break;
    }
    return result;
  }

};

#endif
