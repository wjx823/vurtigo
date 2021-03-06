#include "CathHistoryUI.h"
#include "rtObjectManager.h"
#include "rtBaseHandle.h"
#include "rtApplication.h"

#include <QList>
#include <QMessageBox>

CathHistoryUI::CathHistoryUI() {
  setupUi(this);
  
  m_points = rtBaseHandle::instance().requestNewObject(rtConstants::OT_3DPointBuffer, "Catheter History");
  
  m_historyRecorder = NULL;

  m_numPoints = pointsSpin->value();
  m_interval = intervalSpin->value();
  m_timer = new QTimer(this);
  m_counter = 0;
  m_set = 0;

  markButton->setEnabled(false);
  pointsSpin->setEnabled(false);
  intervalSpin->setEnabled(false);
  autoSaveBox->setEnabled(false);
  resetButton->setEnabled(false);

  m_currCath = -1;
  
  populateLists();
  connectSignals();

  trackingPairChanged();
}

CathHistoryUI::~CathHistoryUI() {
  if (m_historyRecorder != NULL)
    delete m_historyRecorder;
  m_historyRecorder = NULL;   
    
  if (m_points >= 0)
    {
      // delete it somehow
    }

  if (m_timer != NULL)
      delete m_timer;
 
}

void CathHistoryUI::connectSignals() {
  connect( rtApplication::instance().getObjectManager(), SIGNAL(objectCreated(int)), this, SLOT(objectAdded(int)) );
  connect( rtApplication::instance().getObjectManager(), SIGNAL(objectRemoved(int)), this, SLOT(objectRemoved(int)) );
  connect( rtApplication::instance().getObjectManager(), SIGNAL(objectRenamed(int)), this, SLOT(objectRemoved(int)) );

  connect( pointSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(pointSizeChanged(int)) );

  connect( cathComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingPairChanged()) );
  
  connect( cathAutoTrackPushButton, SIGNAL(toggled(bool)), this, SLOT(cathAutoTrackChanged(bool)));
  connect( autoSaveBox, SIGNAL(toggled(bool)),this,SLOT(autoSaveChanged(bool)));

  connect( markButton, SIGNAL(clicked()), this, SLOT(mark()));
  connect( pointsSpin, SIGNAL(valueChanged(int)), this, SLOT(pointsChanged(int)));
  connect ( intervalSpin, SIGNAL(valueChanged(int)), this, SLOT(intervalChanged(int)));

  connect (resetButton, SIGNAL(clicked()), this, SLOT(resetPoints()));
}

void CathHistoryUI::updateCheckableStatus() {
  int cathPos;
  cathPos = cathComboBox->currentIndex();
  if (cathPos >= 0)
  {
      markButton->setEnabled(true);
      pointsSpin->setEnabled(true);
      intervalSpin->setEnabled(true);
      autoSaveBox->setEnabled(true);
      resetButton->setEnabled(true);
  }
  
  cathAutoTrackPushButton->setEnabled((cathPos < 0) ? false : true);
}

void CathHistoryUI::populateLists() {

  disconnect( cathComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingPairChanged()) );
  QList<int> cathObjs;
  QList<int> planeObjs;
  rtCathDataObject* cath=NULL;

  cathObjs = rtBaseHandle::instance().getObjectsOfType(rtConstants::OT_Cath);

  cathComboBox->clear();

  int index = 0;

  for (int ix1=0; ix1<cathObjs.size(); ix1++) {
    cath = static_cast<rtCathDataObject*>(rtBaseHandle::instance().getObjectWithID(cathObjs[ix1]));
    if (cath) {
      m_cathObjectMap.insert(ix1, cath);
      cathComboBox->insertItem(ix1, QString::number(cathObjs[ix1])+" "+cath->getObjName());
      if (m_currCath == cathObjs[ix1])
          index = ix1;
    }
  }

  cathComboBox->setCurrentIndex(index);

  updateCheckableStatus();
  connect( cathComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(trackingPairChanged()) );
}

void CathHistoryUI::objectAdded(int objID) {
  rtDataObject *data=NULL;
  rtCathDataObject *cath=NULL;

  data = rtBaseHandle::instance().getObjectWithID(objID);
  if (!data) return;

  if ( data->getObjectType() == "OT_Cath" ) {
    cath = static_cast<rtCathDataObject*>(data);
    if (!cath) return;
    m_cathObjectMap.insert(m_cathObjectMap.count(), cath);
    cathComboBox->insertItem(cathComboBox->count(), QString::number(objID)+cath->getObjName());
  }

  updateCheckableStatus();
}

//! The object manager has removed an object. It could be of a type we are interesed in.
void CathHistoryUI::objectRemoved(int objID) {
  int cathPos;

  populateLists();

  updateCheckableStatus();
}

void CathHistoryUI::pointSizeChanged(int size) {
  if (m_historyRecorder) {
    m_historyRecorder->setPointSize(size);
  }
}

void CathHistoryUI::trackingPairChanged() {
  int cathPos;

  cathPos = cathComboBox->currentIndex();

  if (cathPos < 0 || m_points < 0) {
   // no selection
    markButton->setEnabled(false);
    pointsSpin->setEnabled(false);
    intervalSpin->setEnabled(false);
  } else {
   // valid selection
    markButton->setEnabled(true);
    pointsSpin->setEnabled(true);
    intervalSpin->setEnabled(true);
    
    rtCathDataObject *cath = m_cathObjectMap.value(cathPos);
    m_currCath = cath->getId();
    rt3DPointBufferDataObject *points = static_cast<rt3DPointBufferDataObject*>(rtBaseHandle::instance().getObjectWithID(m_points));

    if (m_historyRecorder == NULL) {
      m_historyRecorder = new HistoryData(cath, points);
      m_historyRecorder->setPointSize(pointSizeSlider->value());
    } else {
      m_historyRecorder->setCathObject(cath);
    }
  }
  
  updateCheckableStatus();
}

void CathHistoryUI::mark()
{
    if (m_historyRecorder != NULL && m_counter == 0)
    {
        if (m_interval == 0)
        {
            for (int ix1=0; ix1<m_numPoints; ix1++)
            {
                 if (m_historyRecorder->saveSetPoint(m_set))
                     m_counter++;
            }
            if (m_counter > 0)
            {
                savePointObject();
                savedLabel->setText("Marked " + QString::number(m_counter) + " points in set # " + QString::number(m_set));
                m_set++;
                m_counter = 0;
            }
            else
                savedLabel->setText("Could not mark any points");

        }
        else
        {
            savedLabel->setText("Marking...");
            if (m_historyRecorder->saveSetPoint(m_set))
            {
                /*! /todo{
                //save to disk after every new point?
                savePointObject(); }
                */
                m_counter++;
            }
            m_numTries = 1;
            connect(m_timer, SIGNAL(timeout()), this, SLOT(markNow()));
            m_timer->start(m_interval);
        }

    }
}

void CathHistoryUI::markNow()
{
    if (m_numTries >= m_numPoints)
    {
        if (m_counter > 0)
        {
            savedLabel->setText("Marked " + QString::number(m_counter) + " points in set # " + QString::number(m_set));
            m_counter = 0;
            m_set++;
            savePointObject();
        }
        else
            savedLabel->setText("Could not mark any points");
        m_timer->stop();
        disconnect(m_timer, SIGNAL(timeout()), this, SLOT(markNow()));

    }
    else
    {
        savedLabel->setText("Marking...");
        if (m_historyRecorder->saveSetPoint(m_set))
        {
            /*! /todo{
            //save to disk after every new point?
            savePointObject(); }
            */
            m_counter++;
        }
        m_numTries++;
    }
}

void CathHistoryUI::cathAutoTrackChanged(bool value) {
  m_historyRecorder->setAutoTrack(value);
  if (value)
  {
      cathAutoTrackPushButton->setText("Auto Track On");
      savedLabel->setText("");
      filenameLabel->setText("");
  }
  else
      cathAutoTrackPushButton->setText("Auto Track Off");
  markButton->setEnabled(!value);
  pointsSpin->setEnabled(!value);
  intervalSpin->setEnabled(!value);
  autoSaveBox->setEnabled(!value);
}

void CathHistoryUI::autoSaveChanged(bool value)
{
    if (!value)
        filenameLabel->setText("");
}

void CathHistoryUI::savePointObject()
{
    if (autoSaveBox->isChecked())
    {
        if (m_historyRecorder->getPointBufferObject()->getFilename().isEmpty())
        {
            filenameLabel->setStyleSheet("QLabel { color : red; }");
            filenameLabel->setText("NO FILE SPECIFIED, NOT SAVING");
        }
        else
        {
            filenameLabel->setStyleSheet("");
            filenameLabel->setText("Saved Points to: " + QFileInfo(m_historyRecorder->getPointBufferObject()->getFilename()).fileName());
            m_historyRecorder->getPointBufferObject()->saveThisObject();
        }
    }
    QTimer::singleShot(2000, this, SLOT(savedObject()));
}

void CathHistoryUI::resetPoints()
{
    rt3DPointBufferDataObject *dObj = m_historyRecorder->getPointBufferObject();
    if (!dObj) return;
    QMessageBox::StandardButton button1,button2;

    button1 = QMessageBox::question(this, QString("Reset Points"),
                          QString("This operation will delete all points and point data to start a new session at set 0. \n The operation cannot be undone. Proceed?"),
                          QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (button1 == QMessageBox::Yes)
    {
        button2 = QMessageBox::question(this, QString("Save File"),
                              QString("Save points before reset? \n If you select No, any changes made to points before last save will be lost."),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (button2 == QMessageBox::Yes)
        {
            if (dObj->getPointListSize() > 0)
            {
                dObj->saveThisObject();
                if (dObj->getFilename().isEmpty())
                {
                    filenameLabel->setStyleSheet("QLabel { color : red; }");
                    filenameLabel->setText("File not saved");
                }
                else
                {
                    filenameLabel->setStyleSheet("");
                    filenameLabel->setText("Saved Points to: " + QFileInfo(m_historyRecorder->getPointBufferObject()->getFilename()).fileName());
                }
                savedLabel->setText("");
                QTimer::singleShot(2000, this, SLOT(savedObject()));


            }
        }
        dObj->removeAllPoints();
        dObj->setFilename("");
        m_set = 0;
    }
}

void CathHistoryUI::savedObject()
{
    filenameLabel->setText("");
}
