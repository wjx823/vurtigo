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
#include <climits>

#include <QInputDialog>
#include <QMessageBox>
#include <QThread>

#include "rtApplication.h"
#include "rtMessage.h"
#include "rt3dPointBufferDataObject.h"

//! Constructor
rt3DPointBufferDataObject::rt3DPointBufferDataObject()
: m_currentScale(1.0), m_pointZoom(1.0)
{
  removeAllPoints();
  setObjectType(rtConstants::OT_3DPointBuffer);
  setupGUI();

  connect( this, SIGNAL(pointListModifiedSignal()), this, SLOT(pointListModifiedSlot()), Qt::QueuedConnection );
}

//! Destructor
rt3DPointBufferDataObject::~rt3DPointBufferDataObject() {
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


//! Send the info to the GUI
/*!
  @todo Write the GUI
 */
void rt3DPointBufferDataObject::update() {
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
  // Must change the range from [10,50] to [1.0,5.0]
  m_pointZoom = ((double)zoom)/10.0f;
  Modified();
}

void rt3DPointBufferDataObject::transPlusX() {
  applyTranslateToPoints(1.0, 0.0, 0.0);
  Modified();
}

void rt3DPointBufferDataObject::transMinusX() {
  applyTranslateToPoints(-1.0, 0.0, 0.0);
  Modified();
}

void rt3DPointBufferDataObject::transPlusY() {
  applyTranslateToPoints(0.0, 1.0, 0.0);
  Modified();
}

void rt3DPointBufferDataObject::transMinusY() {
  applyTranslateToPoints(0.0, -1.0, 0.0);
  Modified();
}

void rt3DPointBufferDataObject::transPlusZ() {
  applyTranslateToPoints(0.0, 0.0, 1.0);
  Modified();
}

void rt3DPointBufferDataObject::transMinusZ() {
  applyTranslateToPoints(0.0, 0.0, -1.0);
  Modified();
}

void rt3DPointBufferDataObject::rotPlusX() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateX(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::rotMinusX() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateX(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::rotPlusY() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateY(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::rotMinusY() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateY(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::rotPlusZ() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateZ(5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::rotMinusZ() {
  vtkTransform * temp= vtkTransform::New();
  temp->RotateZ(-5.0);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::scaleChanged(double val) {
  double scaleChange = val / m_currentScale;
  m_currentScale = val;

  vtkTransform * temp= vtkTransform::New();
  temp->Scale(scaleChange, scaleChange, scaleChange);
  applyTransformToPoints(temp);
  if (temp) temp->Delete();
  Modified();
}

void rt3DPointBufferDataObject::clearPointDataPressed() {
  QMessageBox::StandardButton button;

  button = QMessageBox::question(getBaseWidget(), QString("Clear All Data"),
                        QString("This operation will delete all points and point data. The operation cannot be undone. Proceed?"),
                        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (button == QMessageBox::Yes) {
    removeAllPoints();
  }
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

  m_optionsWidget.pointsTable->setColumnCount(5+m_columnHeaderList.size());

  m_optionsWidget.pointsTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Point ID") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(1, new QTableWidgetItem("X") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Y") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(3, new QTableWidgetItem("Z") );
  m_optionsWidget.pointsTable->setHorizontalHeaderItem(4, new QTableWidgetItem("Timestamp (ms)") );

  // Add the custom headers (if any)
  for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++) {
    m_optionsWidget.pointsTable->setHorizontalHeaderItem(5+ix1, new QTableWidgetItem(m_columnHeaderList[ix1]) );
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

    // Do the custom headings as well.
    for (int ix1=0; ix1<m_columnHeaderList.size(); ix1++) {
      if (i.value().tagExists(m_columnHeaderList[ix1])) {
        m_optionsWidget.pointsTable->setItem( index, 5+ix1, new QTableWidgetItem(QString::number( i.value().getValue(m_columnHeaderList[ix1]) )) );
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
        m_namedInfoData[id].setNamedValue(m_columnHeaderList[col-5], newValue);
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
        if (m_namedInfoData[id].tagExists(m_columnHeaderList[col-5])) {
          item->setText(QString::number(m_namedInfoData[id].getValue(m_columnHeaderList[col-5])));
        } else {
          item->setText("");
        }
      }

    }
  }
}


void rt3DPointBufferDataObject::pointListModifiedSlot() {
  updateGuiPointList();
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
  connect( m_optionsWidget.newPointButton, SIGNAL(clicked()), this, SLOT(createNewPoint()) );
  connect( m_optionsWidget.addPropertyPushButton, SIGNAL(clicked()), this, SLOT(addNewTagButton()) );

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
