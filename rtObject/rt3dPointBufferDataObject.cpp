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
#include <climits>

#include <QInputDialog>
#include <QMessageBox>
#include <QThread>
#include <QString>
#include <QFileDialog>

#include "rtApplication.h"
#include "rtMainWindow.h"
#include "rtRenderObject.h"
#include "rtMessage.h"
#include "rt3dPointBufferDataObject.h"
#include "rtObjectManager.h"
#include "FilterDialog.h"
#include "rt2DPlotDataObject.h"
#include "rt2DPlotRenderObject.h"
#include <vtkPlot.h>
#include <vtkAxis.h>

#include <vtkChartXY.h>
#include <vtkContextScene.h>

#include <vtkDataArray.h>
#include <vtkFieldData.h>
#include <vtkDataObject.h>
#include <vtkSphereSource.h>

//! Constructor
rt3DPointBufferDataObject::rt3DPointBufferDataObject()
: m_currentScale(1.0), m_pointZoom(1.0), m_saveFileName("")
{
  m_plot = vtkXYPlotActor::New();
  removeAllPoints();
  setObjectType("OT_3DPointBuffer");
  setupGUI();

  connect( this, SIGNAL(pointListModifiedSignal()), this, SLOT(pointListModifiedSlot()), Qt::QueuedConnection );
}

//! Destructor
rt3DPointBufferDataObject::~rt3DPointBufferDataObject() {
  m_plot->Delete();
  removeAllPoints();
  cleanupGUI();
}

rtNamedInfoPointData* rt3DPointBufferDataObject::getPointAt(double x, double y, double z) {
  QList<int> keyList = m_namedInfoData.keys();

  rtNamedInfoPointData* res = 0;
  for (int ix1=0; ix1<keyList.size(); ix1++) {
    if (m_namedInfoData[keyList[ix1]].getX() == x && m_namedInfoData[keyList[ix1]].getY() == y && m_namedInfoData[keyList[ix1]].getZ() == z) {
      res = &(m_namedInfoData[keyList[ix1]]);
    }
  }
  return res;
}

rtNamedInfoPointData* rt3DPointBufferDataObject::getPointWithId(int id) {
  rtNamedInfoPointData* res = 0;
  if ( m_namedInfoData.contains(id) ) {
    res = &(m_namedInfoData[id]);
  }
  return res;
}

void rt3DPointBufferDataObject::removePointWithId(int id) {
  if ( m_namedInfoData.contains(id) ) {
    m_namedInfoData.remove(id);
  }
}

rtNamedInfoPointData* rt3DPointBufferDataObject::getPointAtIndex(int index) {
  QList<int> keyList = m_namedInfoData.keys();

  if ((index < 0) || (index >= keyList.size()))
    return NULL;
      
  return &m_namedInfoData[keyList[index]];
}


//! Add a point to the list
void rt3DPointBufferDataObject::addPoint(rtBasic3DPointData sp) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();
  if (id != -1) {
    sp.setPointId(id);
    namedPt.fromBasic3DPoint(&sp);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}

void rt3DPointBufferDataObject::addTimePoint(rt3DTimePointData pt) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();
  if (id != -1) {
    pt.setPointId(id);
    // Append to the regular point list.
    namedPt.fromTimePoint(&pt);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}


void rt3DPointBufferDataObject::addCartoPoint(rtCartoPointData pt) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();

  if (id != -1) {
    pt.setPointId(id);
    // Append to the regular point list.
    namedPt.fromCartoPoint(&pt);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}

void rt3DPointBufferDataObject::addCustomPoint(rtBasic3DPointData pt, const QList<QPair<QString,double> > &tags) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();
  if (id != -1) {
    pt.setPointId(id);
    // Append to the regular point list.
    namedPt.fromBasic3DPoint(&pt);
    for (int ix1=0; ix1<tags.size(); ix1++)
        namedPt.setNamedValue(tags.at(ix1).first,tags.at(ix1).second);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}

void rt3DPointBufferDataObject::addCustomTimePoint(rt3DTimePointData pt, const QList<QPair<QString,double> > &tags) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();
  if (id != -1) {
    pt.setPointId(id);
    // Append to the regular point list.
    namedPt.fromTimePoint(&pt);
    for (int ix1=0; ix1<tags.size(); ix1++)
        namedPt.setNamedValue(tags.at(ix1).first,tags.at(ix1).second);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}

void rt3DPointBufferDataObject::addCustomCartoPoint(rtCartoPointData pt, const QList<QPair<QString,double> > &tags) {
  rtNamedInfoPointData namedPt;
  int id = getNextId();
  if (id != -1) {
    pt.setPointId(id);
    // Append to the regular point list.
    namedPt.fromCartoPoint(&pt);
    for (int ix1=0; ix1<tags.size(); ix1++)
        namedPt.setNamedValue(tags.at(ix1).first,tags.at(ix1).second);
    m_namedInfoData.insert(id, namedPt);
    emit pointListModifiedSignal();
  }
}


//! Send the info to the GUI
/*!
  @todo Write the GUI
 */
void rt3DPointBufferDataObject::update() {
}

bool rt3DPointBufferDataObject::saveFile(QFile *file) {
  if (!file->open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QXmlStreamWriter writer(file);
  writer.setAutoFormatting(true);
  writer.writeStartDocument();
  writer.writeStartElement("VurtigoFile");
  rtDataObject::saveHeader(&writer, getObjectType(), getObjName());
  rtRenderObject *rObj = rtApplication::instance().getObjectManager()->getObjectWithID(this->getId());
  rObj->saveVisibilities(&writer);

  QList<int> idList = m_namedInfoData.keys();

  writer.writeStartElement("BufferData");
  writer.writeTextElement( "numPoints", QString::number(m_namedInfoData.size()) );
  writer.writeTextElement( "scaling", QString::number(m_currentScale) );
  writer.writeTextElement( "zoom", QString::number(m_pointZoom) );
  writer.writeEndElement(); // BufferData

  for (int ix1=0; ix1<idList.size(); ix1++) {
    rtNamedInfoPointData temp = m_namedInfoData[idList[ix1]];
    QList<QString> tagList = temp.getTagList();

    writer.writeStartElement("InfoPoint");
    writer.writeTextElement( "id", QString::number(idList[ix1]) );
    writer.writeTextElement( "timestamp", QString::number(temp.getCreationTime()) );
    writer.writeTextElement( "size", QString::number(temp.getPointSize()) );
    writer.writeTextElement( "x", QString::number(temp.getX()) );
    writer.writeTextElement( "y", QString::number(temp.getY()) );
    writer.writeTextElement( "z", QString::number(temp.getZ()) );
    writer.writeTextElement("label",temp.getLabel());
    saveVtkProperty(&writer, temp.getProperty(), "Property");
    saveVtkProperty(&writer, temp.getSelectedProperty(), "SelectedProperty");

    for (int ix2=0; ix2<tagList.size(); ix2++) {
      writer.writeStartElement( "CustomTag" );
      writer.writeAttribute( "tag", tagList[ix2] );
      writer.writeAttribute( "value", QString::number(temp.getValue(tagList[ix2])) );
      writer.writeEndElement(); // CustomTag
    }

    writer.writeEndElement(); //InfoPoint
  }


  writer.writeEndElement(); // VurtigoFile
  writer.writeEndDocument();

  file->close();
  return true;
}

bool rt3DPointBufferDataObject::loadFile(QFile *file) {
  if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QXmlStreamReader reader(file);
  QString objType;
  QString objName = "";

  // Remove previous points before adding the new ones.
  removeAllPoints();

  while (!reader.atEnd()) {
    if(reader.readNext() == QXmlStreamReader::StartElement) {
      if (reader.name() == "FileInfo") {
        rtDataObject::loadHeader(&reader, objType, objName);

        if ( objType != this->getObjectType() ) {
          rtApplication::instance().getMessageHandle()->error(__LINE__, __FILE__, QString("Object Type for data file is wrong."));
          break;
        }
      } else if (reader.name() == "Visibilities")
      {
          rtRenderObject *rObj = rtApplication::instance().getObjectManager()->getObjectWithID(this->getId());
          rObj->loadVisibilities(&reader);
      } else if (reader.name() == "BufferData") {
        loadBufferData(&reader);
      } else if (reader.name() == "InfoPoint") {
        loadInfoPoint(&reader);
      }
    }
  }

  if (reader.hasError()) {
    file->close();
    return false;
  }

  file->close();
  emit pointListModifiedSignal();
  return true;
}


void rt3DPointBufferDataObject::applyTransformToPoints(vtkTransform * t) {
  if (!t) return;

  QList<int> keyList = m_namedInfoData.keys();

  for (int ix1=0; ix1<keyList.size(); ix1++) {
    m_namedInfoData[keyList[ix1]].applyTransform(t);
  }
  emit pointListModifiedSignal();
}

void rt3DPointBufferDataObject::applyTranslateToPoints(double x, double y, double z) {
  QList<int> keyList = m_namedInfoData.keys();
  for (int ix1=0; ix1<keyList.size(); ix1++) {
    m_namedInfoData[keyList[ix1]].translate(x,y,z);
  }
  emit pointListModifiedSignal();
}


void rt3DPointBufferDataObject::getPointListCenter(double center[3]) {
  double tempPt[3];
  double total = 0.0;
  QList<int> keyList = m_namedInfoData.keys();

  for (int ix1=0; ix1<keyList.size(); ix1++) {
    m_namedInfoData[keyList[ix1]].getPoint(tempPt);
    total = total + 1.0;
    for (int ix2=0; ix2<3; ix2++) {
      center[ix2] = center[ix2]+tempPt[ix2];
    }
  }

  for (int ix2=0; ix2<3; ix2++) {
    center[ix2] = center[ix2]/total;
  }
}

void rt3DPointBufferDataObject::getPointListExtents(double extents[6]) {
  QList<int> keyList = m_namedInfoData.keys();
  if (keyList.size() <= 0) return;

  double tempPt[3];

  // Get the value for the first point.
  m_namedInfoData[keyList[0]].getPoint(tempPt);

  extents[0] = tempPt[0]; // minx
  extents[1] = tempPt[0]; // maxx
  extents[2] = tempPt[1];
  extents[3] = tempPt[1];
  extents[4] = tempPt[2];
  extents[5] = tempPt[2];

  for (int ix1=1; ix1<keyList.size(); ix1++) {
    m_namedInfoData[keyList[ix1]].getPoint(tempPt);
    if ( tempPt[0] < extents[0] ) extents[0] = tempPt[0];
    if ( tempPt[0] > extents[1] ) extents[1] = tempPt[0];
    if ( tempPt[1] < extents[2] ) extents[2] = tempPt[1];
    if ( tempPt[1] > extents[3] ) extents[3] = tempPt[1];
    if ( tempPt[2] < extents[4] ) extents[4] = tempPt[2];
    if ( tempPt[2] > extents[5] ) extents[5] = tempPt[2];
  }
}


/////////////
// Public slots
/////////////

void rt3DPointBufferDataObject::pointZoomChanged(int zoom) {
  // Must change the range from [1,40] to [0.25,10.0]
  m_pointZoom = ((double)zoom)/4.0f;
  Modified();
}

void rt3DPointBufferDataObject::transPlusX() {
  applyTranslateToPoints(1.0, 0.0, 0.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::transMinusX() {
  applyTranslateToPoints(-1.0, 0.0, 0.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::transPlusY() {
  applyTranslateToPoints(0.0, 1.0, 0.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::transMinusY() {
  applyTranslateToPoints(0.0, -1.0, 0.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::transPlusZ() {
  applyTranslateToPoints(0.0, 0.0, 1.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::transMinusZ() {
  applyTranslateToPoints(0.0, 0.0, -1.0);
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotPlusX() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateX(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotMinusX() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateX(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotPlusY() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateY(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotMinusY() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateY(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotPlusZ() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateZ(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::rotMinusZ() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateZ(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::scaleChanged(double val) {
  double scaleChange = val / m_currentScale;
  m_currentScale = val;

  vtkTransform * temp= vtkTransform::New();
  temp->Scale(scaleChange, scaleChange, scaleChange);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
  // shouldn't have both manual and compass controls on at the same time
  rtApplication::instance().getObjectManager()->getObjectWithID(this->getId())->deselect();
}

void rt3DPointBufferDataObject::clearPointDataPressed() {
  QMessageBox::StandardButton button;

  button = QMessageBox::question(getBaseWidget(), QString("Clear All Data"),
                        QString("This operation will delete all points and point data. The operation cannot be undone. Proceed?"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (button == QMessageBox::Yes) {
    removeAllPoints();
    setFilename("");
  }
}


void rt3DPointBufferDataObject::loadThisObject() {
  QFile file;
  QString name;

  name = QFileDialog::getOpenFileName(getBaseWidget(), "Select Files To Open:");
  file.setFileName(name);

  if (file.exists()) {
    this->loadFile(&file);
  }
}


void rt3DPointBufferDataObject::saveThisObject() {
  QFile file;

  if (m_saveFileName.isEmpty()) {
    // Find the file name
    m_saveFileName = QFileDialog::getSaveFileName(getBaseWidget(), "Save As...");
  }
  if (m_saveFileName.isEmpty())
  {
      m_optionsWidget.filename->setText("File not saved");
  }
  else
  {
      file.setFileName(m_saveFileName);
      m_optionsWidget.filename->setText(QString("Saved to: " + QFileInfo(m_saveFileName).fileName()));
      this->saveFile(&file);
  }
  QTimer::singleShot(2000, this, SLOT(savedObject()));
}

void rt3DPointBufferDataObject::saveAsThisObject() {
  QFile file;
  m_saveFileName = QFileDialog::getSaveFileName(getBaseWidget(), "Save As...",QFileInfo(m_saveFileName).dir().path());

  if (m_saveFileName.isEmpty())
  {
      m_optionsWidget.filename->setText("File not saved");
  }
  else
  {
      file.setFileName(m_saveFileName);
      m_optionsWidget.filename->setText(QString("Saved to: " + QFileInfo(m_saveFileName).fileName()));
      this->saveFile(&file);
  }
  QTimer::singleShot(2000, this, SLOT(savedObject()));
}

void rt3DPointBufferDataObject::savedObject()
{
    m_optionsWidget.filename->setText(QString(""));
}

void rt3DPointBufferDataObject::removeSelectedPoints() {
  if (m_selectedItems.isEmpty()) return;

  for (int ix1=0; ix1<m_selectedItems.size(); ix1++) {
    m_namedInfoData.remove( m_selectedItems[ix1] );
    removePointWithId( m_selectedItems[ix1] );
  }
  m_selectedItems.clear();
  emit pointListModifiedSignal();
}

void rt3DPointBufferDataObject::filterPoints() {
  FilterDialog filterD(getBaseWidget());

  filterD.setPointList(&m_namedInfoData);

  if ( filterD.exec() == QDialog::Accepted) {
    emit pointListModifiedSignal();
  }
}

void rt3DPointBufferDataObject::createNewPoint() {
  rtBasic3DPointData newPt;
  newPt.setPoint(0.0, 0.0, 0.0);
  newPt.setColor(1.0, 0.0, 0.0);
  addPoint(newPt);
}

void rt3DPointBufferDataObject::addNewTagButton() {
  bool ok;
  QString tagName = QInputDialog::getText(getBaseWidget(),
                                          QString("New Property Name"),
                                          QString("Enter the name of the new property:"),
                                          QLineEdit::Normal, QString("NewProperty"), &ok);
  tagName = tagName.trimmed();

  if (ok && !tagName.isEmpty()) {
    if (!m_columnHeaderList.contains(tagName)) {
      m_columnHeaderList.append(tagName);
      emit pointListModifiedSignal();
    } else {
      // Report as a warning.
      rtApplication::instance().getMessageHandle()->warning( __LINE__, QString(__FILE__), QString("Property Name Already Exists. No New Property Created.") );
    }
  }
}

void rt3DPointBufferDataObject::plotProperties()
{
    if (m_optionsWidget.xAxisProperty->currentIndex() == -1) return;
    if (m_optionsWidget.yAxisProperty->currentIndex() == -1) return;

    vtkDataArray *xdata = vtkDataArray::CreateDataArray(VTK_DOUBLE);
    vtkDataArray *ydata = vtkDataArray::CreateDataArray(VTK_DOUBLE);


    double x,y;


    if (m_optionsWidget.selectedRadio->isChecked())
    {
        if (m_selectedItems.size() == 0)
            return;

        xdata->SetNumberOfTuples(m_selectedItems.size());
        ydata->SetNumberOfTuples(m_selectedItems.size());

        for (int ix1=0; ix1<m_selectedItems.size(); ix1++)
        {
            x = getPointWithId(m_selectedItems[ix1])->getValue(m_optionsWidget.xAxisProperty->currentText());
            xdata->SetTuple(ix1,&x);
            y = getPointWithId(m_selectedItems[ix1])->getValue(m_optionsWidget.yAxisProperty->currentText());
            ydata->SetTuple(ix1,&y);
        }
    }
    else if (m_optionsWidget.allRadio->isChecked())
    {
        xdata->SetNumberOfTuples(getPointListSize());
        ydata->SetNumberOfTuples(getPointListSize());

        for (int ix1=0; ix1<getPointListSize(); ix1++)
        {
            x = getPointAtIndex(ix1)->getValue(m_optionsWidget.xAxisProperty->currentText());
            xdata->SetTuple(ix1,&x);
            y = getPointAtIndex(ix1)->getValue(m_optionsWidget.yAxisProperty->currentText());
            ydata->SetTuple(ix1,&y);
        }
    }
/*  VTK Legacy Method of plotting
    /////////////////////////////

    vtkFieldData *fData = vtkFieldData::New();
    fData->AllocateArrays(2);
    fData->AddArray(xdata);
    fData->AddArray(ydata);

    vtkDataObject *dObj = vtkDataObject::New();
    dObj->SetFieldData(fData);

    m_plot->Delete();
    m_plot = vtkXYPlotActor::New();

    m_plot->AddDataObjectInput(dObj);

    m_plot->SetXValuesToValue();
    m_plot->SetDataObjectXComponent(0,0);
    m_plot->SetDataObjectYComponent(0,1);

    m_plot->SetPosition(0.1,0.7);
    m_plot->SetWidth(0.7);
    m_plot->SetHeight(0.3);

    m_plot->SetTitle(QString(m_optionsWidget.yAxisProperty->currentText() + " vs. " + m_optionsWidget.xAxisProperty->currentText()).toStdString().c_str());
    m_plot->SetXTitle(m_optionsWidget.xAxisProperty->currentText().toStdString().c_str());
    m_plot->SetYTitle(m_optionsWidget.yAxisProperty->currentText().toStdString().c_str());

    //vtkSphereSource *symb = vtkSphereSource::New();
    //m_plot->SetPlotSymbol(0,symb->GetOutput());
    //symb->Delete();
    m_plot->GetProperty()->SetPointSize(5);
    m_plot->PlotPointsOn();

    // which render window do we put it in? for now just put in window 0
    rtApplication::instance().getMainWinHandle()->getRenderer(0)->AddActor(m_plot);
    rtApplication::instance().getMainWinHandle()->setRenderFlag3D(true);
*/
    rt2DPlotRenderObject *rObj;
    rObj = static_cast<rt2DPlotRenderObject *>(rtApplication::instance().getObjectManager()->addObjectOfType("OT_2DPlot",QString(this->getObjName() + ": " + m_optionsWidget.yAxisProperty->currentText() + " vs. " + m_optionsWidget.xAxisProperty->currentText())));
    rt2DPlotDataObject *plot = static_cast<rt2DPlotDataObject *>(rObj->getDataObject());
    xdata->SetName(m_optionsWidget.xAxisProperty->currentText().toStdString().c_str());
    ydata->SetName(m_optionsWidget.yAxisProperty->currentText().toStdString().c_str());
    plot->addDataArray(xdata);
    plot->addDataArray(ydata);

    rObj->createPlot(0,1);
    rObj->getPlotAtIndex(0)->SetLabel(plot->getObjName().toStdString().c_str());
    rObj->getPlotAtIndex(0)->GetXAxis()->SetTitle(xdata->GetName());
    rObj->getPlotAtIndex(0)->GetYAxis()->SetTitle(ydata->GetName());
}

void rt3DPointBufferDataObject::updateGuiPointList() {
  QHash<int, rtNamedInfoPointData>::iterator i;
  QList<QString> tagList;
  int index=0;

  // Disconnect the table so that we don't get all the changed events.
  disconnect( m_optionsWidget.pointsTable, SIGNAL( cellChanged(int,int) ), this, SLOT( tableCellChanged(int,int) ) );

  // Lock to ensure that the list is not currently reserved by another thread.
  this->lock();

  // Update the list of custom column headers.
  updateColumnHeaders();

  m_optionsWidget.pointsTable->clear();

  m_optionsWidget.pointsTable->setColumnCount(6+m_columnHeaderList.size());

  m_optionsWidget.pointsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Point ID") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("X") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Y") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Z") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(4, new QTableWidgetItem("Timestamp (ms)") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(5, new QTableWidgetItem("Point Label") );

  // Add the custom headers (if any)
  for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++) {
    m_optionsWidget.pointsTable->setHorizontalHeaderItem(6+ix1, new QTableWidgetItem(m_columnHeaderList[ix1]) );
  }
  m_optionsWidget.pointsTable->setRowCount(m_namedInfoData.size());
  index = 0;
  for (i = m_namedInfoData.begin(); i != m_namedInfoData.end(); ++i) {
    m_optionsWidget.pointsTable->setItem( index, 0, new QTableWidgetItem(QString::number(i.key())) );
    m_optionsWidget.pointsTable->item( index, 0 )->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    m_optionsWidget.pointsTable->setItem( index, 1, new QTableWidgetItem(QString::number(i.value().getX())) );
    m_optionsWidget.pointsTable->setItem( index, 2, new QTableWidgetItem(QString::number(i.value().getY())) );
    m_optionsWidget.pointsTable->setItem( index, 3, new QTableWidgetItem(QString::number(i.value().getZ())) );
    m_optionsWidget.pointsTable->setItem( index, 4, new QTableWidgetItem(QString::number(i.value().getCreationTime())) );
    m_optionsWidget.pointsTable->item( index, 4 )->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    m_optionsWidget.pointsTable->setItem( index, 5, new QTableWidgetItem(i.value().getLabel()));

    // Do the custom headings as well.
    for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++) {
      if (i.value().tagExists(m_columnHeaderList[ix1])) {
        m_optionsWidget.pointsTable->setItem( index, 6+ix1, new QTableWidgetItem(QString::number( i.value().getValue(m_columnHeaderList[ix1]) )) );
      }
    }

    index++;
  }

  // Done modifications so we can unlock.
  this->unlock();

  // Connect the table again at the end.
  connect( m_optionsWidget.pointsTable, SIGNAL( cellChanged(int,int) ), this, SLOT( tableCellChanged(int,int) ) );

}

void rt3DPointBufferDataObject::tableSelectionChanged() {
  QList<QTableWidgetItem *> selected;

  selected = m_optionsWidget.pointsTable->selectedItems();
  m_selectedItems.clear();
  for (int ix1=0; ix1<selected.size(); ix1++) {
    if (selected[ix1]->column() == 0) {
      m_selectedItems.append( selected[ix1]->text().toInt() );
    }
  }
  Modified();
}

void rt3DPointBufferDataObject::tableCellChanged(int row, int col) {
  // Make sure that the IDs and timestamps are not being changed.
  if (col == 0 || col == 4) return;

  QTableWidgetItem* item;
  QTableWidgetItem* itemId;
  int id;
  double newValue;
  bool ok;
  rtBasic3DPointData* pointHandle;

  item = m_optionsWidget.pointsTable->item(row, col);
  itemId = m_optionsWidget.pointsTable->item(row, 0);
  id = itemId->text().toInt(&ok);

  if (ok && m_namedInfoData.contains(id)) {
      // if the label changed
      if (col == 5)
      {
          char text[100] = "";
          sprintf(text,"%s",qPrintable(item->text()));
          m_namedInfoData[id].setLabel(text);
          Modified();
      }
      else
      {
          newValue = item->text().toDouble(&ok);
          pointHandle = getPointWithId(id);

          if (ok) {
            // Value is a valid double. Must update data structures.
            if (col == 1) {
              m_namedInfoData[id].setX(newValue);
              pointHandle->setX(newValue);
            } else if (col == 2) {
              m_namedInfoData[id].setY(newValue);
              pointHandle->setY(newValue);
            } else if (col == 3) {
              m_namedInfoData[id].setZ(newValue);
              pointHandle->setZ(newValue);
            } else {
              // A custom tag.
              m_namedInfoData[id].setNamedValue(m_columnHeaderList[col-6], newValue);
            }

            // Both numbers are OK so we can call modified.
            Modified();

          } else {
            // Value is not a valid double.
            // Must replace string with the old value.
            if (col == 1) {
              item->setText(QString::number(m_namedInfoData[id].getX()));
            } else if (col == 2) {
              item->setText(QString::number(m_namedInfoData[id].getY()));
            } else if (col == 3) {
              item->setText(QString::number(m_namedInfoData[id].getZ()));
            } else {
              // A custom tag.
              if (m_namedInfoData[id].tagExists(m_columnHeaderList[col-6])) {
                item->setText(QString::number(m_namedInfoData[id].getValue(m_columnHeaderList[col-6])));
              } else {
                item->setText("");
              }
            }

          }
        }
      }



}


void rt3DPointBufferDataObject::pointListModifiedSlot() {
  updateGuiPointList();
  updatePlotProperties();
  Modified();
}

////////////
// Protected
////////////

void rt3DPointBufferDataObject::setupGUI() {
  m_optionsWidget.setupUi(getBaseWidget());


  // Buttons above the points table
  connect( m_optionsWidget.pointZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(pointZoomChanged(int)) );
  connect( m_optionsWidget.clearDataPushButton, SIGNAL(clicked()), this, SLOT(clearPointDataPressed()) );
  connect( m_optionsWidget.deleteSelectedButton, SIGNAL(clicked()), this, SLOT(removeSelectedPoints()) );
  connect( m_optionsWidget.filterPushButton, SIGNAL(clicked()), this, SLOT(filterPoints()) );
  connect( m_optionsWidget.newPointButton, SIGNAL(clicked()), this, SLOT(createNewPoint()) );
  connect( m_optionsWidget.addPropertyPushButton, SIGNAL(clicked()), this, SLOT(addNewTagButton()) );

  // Load and save buttons.
  connect( m_optionsWidget.loadButton, SIGNAL(clicked()), this, SLOT(loadThisObject()) );
  connect( m_optionsWidget.saveButton, SIGNAL(clicked()), this, SLOT(saveThisObject()) );
  connect( m_optionsWidget.saveasButton,SIGNAL(clicked()), this, SLOT(saveAsThisObject()));

  // Setup the points table

  m_optionsWidget.pointsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_optionsWidget.pointsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect( m_optionsWidget.pointsTable, SIGNAL( itemSelectionChanged() ), this, SLOT( tableSelectionChanged() ) );
  connect( m_optionsWidget.pointsTable, SIGNAL( cellChanged(int,int) ), this, SLOT( tableCellChanged(int,int) ) );

  // Connect the buttons
  connect( m_optionsWidget.pushXPlus, SIGNAL(clicked()), this, SLOT(transPlusX()) );
  connect( m_optionsWidget.pushYPlus, SIGNAL(clicked()), this, SLOT(transPlusY()) );
  connect( m_optionsWidget.pushZPlus, SIGNAL(clicked()), this, SLOT(transPlusZ()) );
  connect( m_optionsWidget.pushXMinus, SIGNAL(clicked()), this, SLOT(transMinusX()) );
  connect( m_optionsWidget.pushYMinus, SIGNAL(clicked()), this, SLOT(transMinusY()) );
  connect( m_optionsWidget.pushZMinus, SIGNAL(clicked()), this, SLOT(transMinusZ()) );
  connect( m_optionsWidget.rotateXPlus, SIGNAL(clicked()), this, SLOT(rotPlusX()) );
  connect( m_optionsWidget.rotateYPlus, SIGNAL(clicked()), this, SLOT(rotPlusY()) );
  connect( m_optionsWidget.rotateZPlus, SIGNAL(clicked()), this, SLOT(rotPlusZ()) );
  connect( m_optionsWidget.rotateXMinus, SIGNAL(clicked()), this, SLOT(rotMinusX()) );
  connect( m_optionsWidget.rotateYMinus, SIGNAL(clicked()), this, SLOT(rotMinusY()) );
  connect( m_optionsWidget.rotateZMinus, SIGNAL(clicked()), this, SLOT(rotMinusZ()) );
  connect( m_optionsWidget.scaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(scaleChanged(double)) );

  //connect the plot buttons
  connect( m_optionsWidget.plotButton, SIGNAL(clicked()), this, SLOT(plotProperties()));
}

void rt3DPointBufferDataObject::cleanupGUI() {

}

int rt3DPointBufferDataObject::getNextId() {
  // Try to find the next available ID.
  for (int ix1=0; ix1<INT_MAX; ix1++) {
    if (!m_namedInfoData.contains(ix1)) {
      return ix1;
    }
  }
  return -1;
}

void rt3DPointBufferDataObject::updateColumnHeaders() {
  QHash<int, rtNamedInfoPointData>::iterator i;
  QList<QString> tagList;

  for (i = m_namedInfoData.begin(); i != m_namedInfoData.end(); ++i) {
    tagList = i.value().getTagList();
    for ( int ix1=0; ix1<tagList.size(); ix1++ ) {
      if ( !m_columnHeaderList.contains(tagList[ix1]) )
        m_columnHeaderList.append(tagList[ix1]);
    }
  }
}

void rt3DPointBufferDataObject::updatePlotProperties()
{

    m_optionsWidget.xAxisProperty->clear();
    m_optionsWidget.yAxisProperty->clear();
    /*
    m_optionsWidget.xAxisProperty->insertItem(0,"Point ID" );
    m_optionsWidget.xAxisProperty->insertItem(1, "X" );
    m_optionsWidget.xAxisProperty->insertItem(2, "Y" );
    m_optionsWidget.xAxisProperty->insertItem(3, "Z" );
    m_optionsWidget.xAxisProperty->insertItem(4, "Timestamp (ms)" );

    m_optionsWidget.yAxisProperty->insertItem(0,"Point ID" );
    m_optionsWidget.yAxisProperty->insertItem(1, "X" );
    m_optionsWidget.yAxisProperty->insertItem(2, "Y" );
    m_optionsWidget.yAxisProperty->insertItem(3, "Z" );
    m_optionsWidget.yAxisProperty->insertItem(4, "Timestamp (ms)" );

    for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++)
    {
        m_optionsWidget.xAxisProperty->insertItem(6+ix1,m_columnHeaderList[ix1]);
        m_optionsWidget.yAxisProperty->insertItem(6+ix1,m_columnHeaderList[ix1]);
    }
    */
    for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++)
    {
        m_optionsWidget.xAxisProperty->insertItem(ix1,m_columnHeaderList[ix1]);
        m_optionsWidget.yAxisProperty->insertItem(ix1,m_columnHeaderList[ix1]);
    }
}


int rt3DPointBufferDataObject::loadBufferData(QXmlStreamReader* reader) {
  if (!reader) return 0;

  if ( !(reader->name()=="BufferData" || reader->isStartElement()) ) {
    rtApplication::instance().getMessageHandle()->error(__LINE__, __FILE__, QString("Failed to load Buffer Data from file."));
    return 0;
  }

  int numPoints = 0;
  while ( reader->name() != "BufferData" || !reader->isEndElement() ) {
    if(reader->readNext() == QXmlStreamReader::StartElement) {
      if (reader->name() == "numPoints") {
        numPoints = getIntFromString(reader->readElementText(), 0);
      } else if (reader->name() == "scalng") {
        m_currentScale = getDoubleFromString(reader->readElementText(), 1.0);
        m_optionsWidget.scaleSpinBox->setValue(m_currentScale);
      } else if (reader->name() == "zoom") {
        m_pointZoom = getDoubleFromString(reader->readElementText(), 1.0);
        m_optionsWidget.pointZoomSlider->setValue(m_pointZoom * 10.0);
      }
    }
  }
  return numPoints;
}

void rt3DPointBufferDataObject::loadInfoPoint(QXmlStreamReader* reader) {
  if (!reader) return;

  if ( !(reader->name()=="InfoPoint" || reader->isStartElement()) ) {
    rtApplication::instance().getMessageHandle()->error(__LINE__, __FILE__, QString("Failed to load Info Point from file."));
    return;
  }

  // Create an empty point.
  rtNamedInfoPointData temp;
  vtkProperty* inputProp = vtkProperty::New();
  QString propName;
  while ( reader->name() != "InfoPoint" || !reader->isEndElement() ) {
    if(reader->readNext() == QXmlStreamReader::StartElement) {
      if (reader->name() == "id") {
        temp.setPointId( getIntFromString( reader->readElementText(), getNextId() ) );
      } else if (reader->name() == "timestamp") {
        temp.setCreationTime( getDoubleFromString(reader->readElementText(), 0.0) );
      } else if (reader->name() == "size") {
        temp.setPointSize( getDoubleFromString(reader->readElementText(), 1.0) );
      } else if (reader->name() == "x") {
        temp.setX( getDoubleFromString(reader->readElementText(), 0.0) );
      } else if (reader->name() == "y") {
        temp.setY( getDoubleFromString(reader->readElementText(), 0.0) );
      } else if (reader->name() == "z") {
        temp.setZ( getDoubleFromString(reader->readElementText(), 0.0) );
      } else if (reader->name() == "label") {
        temp.setLabel(reader->readElementText());
      } else if (reader->name() == "VtkProperty") {
        // Read each of the two properties.
        loadVtkProperty(reader, inputProp, propName);
        if (propName == "Property") {
          temp.getProperty()->DeepCopy(inputProp);
        } else if (propName == "SelectedProperty") {
          temp.getSelectedProperty()->DeepCopy(inputProp);
        }
      } else if (reader->name() == "CustomTag") {
        QXmlStreamAttributes attribs = reader->attributes();
        temp.setNamedValue(attribs.value("tag").toString(), getDoubleFromString(attribs.value("value").toString(), 0.0));
      }
    }
  }
  inputProp->Delete();
  m_namedInfoData.insert(temp.getPointId(), temp);

}
