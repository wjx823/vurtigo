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
#include "rtDataObject.h"
#include "rtPluginLoader.h"
#include "rtApplication.h"
#include "rtMessage.h"
#include "rtObjectManager.h"

rtDataObject::rtDataObject() {
  int nextID;

  m_readOnly = false;

  // Get a valid ID from the object manager.
  nextID = rtApplication::instance().getObjectManager()->getNextID();
  if (nextID == -1) {
    rtApplication::instance().getMessageHandle()->error(__LINE__, __FILE__, QString("Could not find a valid ID for a new object! "));
  }
  setId(nextID);

  // When the object changes we can update the GUI.
  connect(this, SIGNAL(objectChanged(int)), this, SLOT(updateGUI()));
}

rtDataObject::~rtDataObject() {
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

void rtDataObject::Modified() {
  m_modifyTime=QDateTime::currentDateTime();
  rtApplication::instance().getPluginLoader()->objectModified(m_objId);
  emit objectChanged(m_objId);
}

QDateTime rtDataObject::getModified() {
  return m_modifyTime;
}

void rtDataObject::updateGUI() {
  // Call the update function for each object.
  update();
}

void rtDataObject::saveHeader(QXmlStreamWriter *writer) {
  rtDataObject::saveHeader(writer, m_objType, m_objName);
}

void rtDataObject::saveHeader(QXmlStreamWriter *writer, rtConstants::rtObjectType type, QString name) {
  writer->writeStartElement("FileInfo");
  writer->writeTextElement( "type", QString::number( (int)type ) );
  writer->writeTextElement( "name", name );
  writer->writeEndElement();
}

void rtDataObject::loadHeader(QXmlStreamReader *reader, rtConstants::rtObjectType &type, QString &name) {
  bool intOK;

  if (!reader) {
    type = rtConstants::OT_None;
    name = "";
    return;
  }

  if ( !(reader->name()=="FileInfo" || reader->isStartElement()) ) {
    rtApplication::instance().getMessageHandle()->error(__LINE__, __FILE__, QString("Failed to load file header."));
    return;
  }

  name = "";
  while ( reader->name() != "FileInfo" || !reader->isEndElement() ) {
    if(reader->readNext() == QXmlStreamReader::StartElement) {

      if(reader->name() == "type") {
        int objType = reader->readElementText().toInt(&intOK);
        if (intOK) {
          type = rtConstants::intToObjectType(objType);
        } else {
          type = rtConstants::OT_None;
        }
      } else if (reader->name() == "name") {
        name = reader->readElementText();
      }

    }
  }
  rtApplication::instance().getMessageHandle()->log(QString("Finished reading file header."));
}
