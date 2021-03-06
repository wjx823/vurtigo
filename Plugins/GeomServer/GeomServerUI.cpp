#include "GeomServerUI.h"
#include <QFileDialog>
#include <QComboBox>
#include <iostream>
#include <sstream>
#include "arguments.h"
#include "SenderSimp.h"
#include "rtApplication.h"
#include "rtTimeManager.h"

using namespace std;

//! Setup slots for UI actions
void GeomServerUI::setupSlots() {
  connect(connectButton, SIGNAL(clicked()), this, SLOT(serverConnect()));
  connect(disconnectButton, SIGNAL(clicked()), this, SLOT(serverDisconnect()));
  connect(numPlanesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(numPlanesChanged(int)));
  connect(numCathSpinBox, SIGNAL(valueChanged(int)), this, SLOT(numCathChanged(int)));
  connect(cathZeroCheckBox, SIGNAL(toggled(bool)), this, SLOT(cathZeroOnlyChanged(bool)));
  connect(senderThread.getSender(), SIGNAL(isNowConnected(char *, int)), this, SLOT(serverConnectDo(char *, int)));
}

//! Set defaults of the UI state
void GeomServerUI::setupDefaults() {
  arguments * args = senderThread.getArgs();
  std::stringstream sstream;

  hostLineEdit->insert(args->hostname);
  sstream << args->port;
  portLineEdit->insert(sstream.str().c_str());
}

//! Actions on startup
void GeomServerUI::init() {
}

GeomServerUI::GeomServerUI(SenderThread & senderThread) : senderThread(senderThread) {
  setupUi(this);
  setupDefaults();
  setupSlots();
  init();

  m_planeList.clear();
  m_cathList.clear();
  m_numPlanes = 0;
  m_numCath = 0;
}

GeomServerUI::~GeomServerUI() {
}

void GeomServerUI::updateObjectList() {
  SenderThread::ListObject obj;

  for (int ix1=0; ix1<m_numPlanes; ix1++) {
    if (ix1 < m_planeList.count()) {
      obj = m_planeList.takeAt(ix1);
    } else {
      obj.id = ix1;
      obj.type = SenderThread::OBJ_TYPE_PLANE;
      obj.act = SenderThread::OBJ_READ;
    }
    m_planeList.insert(ix1, obj);
  }

  for (int ix1=0; ix1<m_numCath; ix1++) {
    if (ix1 < m_cathList.count()) {
      obj = m_cathList.takeAt(ix1);
    } else {
      obj.id = ix1;
      obj.type = SenderThread::OBJ_TYPE_CATH;
      obj.act = SenderThread::OBJ_READ;
    }
    m_cathList.insert(ix1, obj);
  }

  // Remove elements if the lists are too long.
  while ( m_planeList.count() > m_numPlanes ) m_planeList.takeAt(m_numPlanes);
  while ( m_cathList.count() > m_numCath ) m_cathList.takeAt(m_numCath);

  updateListGUI();
}

void GeomServerUI::updateListGUI() {

  objectTable->setColumnCount(2);
  objectTable->setRowCount(m_numPlanes+m_numCath);
  for (int ix1=0; ix1<m_numPlanes; ix1++) {
    QString text;
    text = QString("Plane ")+QString::number(ix1);
    QTableWidgetItem *temp = new QTableWidgetItem();
    temp->setText(text);
    temp->setFlags(Qt::NoItemFlags);
    objectTable->setItem(ix1, 0, temp);

    QComboBox *tempCombo = new QComboBox();
    objectTable->setCellWidget(ix1, 1, tempCombo);
    tempCombo->addItem(QString("Ignore"));
    tempCombo->addItem(QString("Read"));
    tempCombo->addItem(QString("Write"));
    tempCombo->addItem(QString("Error"));

    tempCombo->setCurrentIndex(m_planeList.at(ix1).act);
    connect(tempCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(itemStateChanged(int)));
  }

  for (int ix1=0; ix1<m_numCath; ix1++) {
    QString text;
    text += QString("Catheter ");
    text += QString::number(ix1);
    QTableWidgetItem *temp = new QTableWidgetItem();
    temp->setText(text);
    temp->setFlags(Qt::NoItemFlags);
    objectTable->setItem(ix1+m_numPlanes, 0, temp);

    QComboBox *tempCombo = new QComboBox();
    objectTable->setCellWidget(ix1+m_numPlanes, 1, tempCombo);
    tempCombo->addItem(QString("Ignore"));
    tempCombo->addItem(QString("Read"));
    tempCombo->addItem(QString("Write"));
    tempCombo->addItem(QString("Error"));
    tempCombo->setCurrentIndex(m_cathList.at(ix1).act);
    connect(tempCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(itemStateChanged(int)));
  }

}


//! Connect to server
void GeomServerUI::serverConnect() {
  bool ok;
  arguments * args = senderThread.getArgs();

  SenderSimp::copyString(&args->hostname, hostLineEdit->text().toAscii().data());
  
  portLineEdit->text().toInt(&ok);
  if (ok)
      args->port = portLineEdit->text().toInt();

  senderThread.serverConnect();
}

//! Disonnect from server
void GeomServerUI::serverDisconnect() {
  senderThread.serverDisconnect();
  // If we are disconnecting from the server we should return to state it was before
  rtApplication::instance().getTimeManager()->triggerTimeSourceChanged(m_triggerState);
  connectstat->setText(QString("Connection Closed"));
}

void GeomServerUI::serverConnectDo(char *host, int port) {
  m_triggerState = rtApplication::instance().getTimeManager()->getSimulatedTrigger();
  rtApplication::instance().getTimeManager()->triggerTimeSourceChanged(false);
  connectstat->setText(QString("Established " + QString(host) + ":" + QString::number(port)));
}


void GeomServerUI::numPlanesChanged(int i) {
  m_numPlanes = i;
  updateObjectList();
  updateReaderObject();
}

void GeomServerUI::numCathChanged(int i) {
  m_numCath = i;
  updateObjectList();
  updateReaderObject();
}

//! The state of one of the combo boxes has changed.
void GeomServerUI::itemStateChanged(int state) {
  if (state == -1) return;

  // Extract plane info.
  for (int ix1=0; ix1<m_numPlanes; ix1++) {
    QComboBox* temp = static_cast<QComboBox*>(objectTable->cellWidget(ix1, 1));
    m_planeList[ix1].act = static_cast<SenderThread::ObjectActions>(temp->currentIndex());
  }

  // Extract catheter info.
  for (int ix1=0; ix1<m_numCath; ix1++) {
    QComboBox* temp = static_cast<QComboBox*>(objectTable->cellWidget(ix1+m_numPlanes, 1));
    m_cathList[ix1].act = static_cast<SenderThread::ObjectActions>(temp->currentIndex());
  }
  updateReaderObject();
}

void GeomServerUI::cathZeroOnlyChanged (bool status) {
  senderThread.getSender()->getReaderHandle()->setZeroCathOnly(status);
}

void GeomServerUI::updateReaderObject() {
  senderThread.getSender()->getReaderHandle()->setPlaneList(m_planeList);
  senderThread.getSender()->getReaderHandle()->setCathList(m_cathList);
}
