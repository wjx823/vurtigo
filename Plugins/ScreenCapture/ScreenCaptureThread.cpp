#include "ScreenCaptureThread.h"

#include "rtBaseHandle.h"

ScreenCaptureThread::ScreenCaptureThread() {
  m_jpgWriter = NULL;
  m_bmpWriter = NULL;
  m_aviWriter = NULL;

  m_screen = vtkImageData::New();

  // Start with a single resource.
  m_screenLock.release();
}


ScreenCaptureThread:: ~ScreenCaptureThread() {
  m_screen->Delete();
}

//! Start the screen capture thread.
void ScreenCaptureThread::run() {
  m_jpgWriter = new JpgWriter(this);
  m_bmpWriter = new BmpWriter(this);
  m_aviWriter = new AviWriter(this);

  //Blocking Queued connection.
  connect(m_jpgWriter, SIGNAL(requestNewScreen()), this, SLOT(updateScreenCapture()), Qt::BlockingQueuedConnection);
  connect(m_bmpWriter, SIGNAL(requestNewScreen()), this, SLOT(updateScreenCapture()), Qt::BlockingQueuedConnection);
  connect(m_aviWriter, SIGNAL(requestNewScreen()), this, SLOT(updateScreenCapture()), Qt::BlockingQueuedConnection);

  m_objectsCreated.release();
  exec();

  if (m_jpgWriter) delete m_jpgWriter;
  if (m_bmpWriter) delete m_bmpWriter;
  if (m_aviWriter) delete m_aviWriter;
}

//! Send the file name to all the writers.
void ScreenCaptureThread::setFileName(QString fName) {
  m_jpgWriter->setFileName(fName);
  m_bmpWriter->setFileName(fName);
  m_aviWriter->setFileName(fName);
}

//! Wait for the objects to be created.
void ScreenCaptureThread::waitForObjectInit() {
  m_objectsCreated.acquire();
}

//! Get a new screen capture.
/*!
  This function should be executed in the main thread.
  */
void ScreenCaptureThread::updateScreenCapture() {
  m_screenLock.acquire();
  m_screen->DeepCopy(rtBaseHandle::instance().grabScreenshot());
  m_screenLock.release();
}

//! Get a copy of the last screenshot
void ScreenCaptureThread::getScreenCopy(vtkImageData* copy) {
  m_screenLock.acquire();
  copy->DeepCopy(m_screen);
  m_screenLock.release();
}
