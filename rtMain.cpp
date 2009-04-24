#include <QApplication>
#include <iostream> 
#include "rtMainWindow.h"
#include "rtObjectManager.h"
#include "rtLabelRenderObject.h"
#include "rtTimeManager.h"
#include "rtPluginLoader.h"

#include "rt3dPointBufferDataObject.h"
#include "rtCathDataObject.h"

//! Just a function to load some test datasets. 
void loadTestData() {
  rtRenderObject* label;
  rtRenderObject* label2;
  rtRenderObject* pointList;
  rtRenderObject* noneObj;
  rtRenderObject* cath;
  rtRenderObject* cath2;
  rtRenderObject* cath4;

  rt3DPointBufferDataObject::SimplePoint p1, p2, p3, p4;

  qDebug("Loading Test Data...");

  p1.px = 0.0;
  p1.py = 0.0;
  p1.pz = 0.0;
  p1.pSize = 0.25;
  p1.pProp->SetColor(1, 0, 0); //Red
  p1.pProp->SetOpacity(0.2);

  p2.px = 1.0;
  p2.py = 0.0;
  p2.pz = 0.0;
  p2.pSize = 0.25;
  p2.pProp->SetColor(1, 0, 0);

  p3.px = 0.0;
  p3.py = 1.0;
  p3.pz = 0.0;
  p3.pSize = 0.25;
  p3.pProp->SetColor(0, 1, 0);

  p4.px = 0.0;
  p4.py = 0.0;
  p4.pz = 1.0;
  p4.pSize = 0.25;
  p4.pProp->SetColor(0, 0, 1);

  int ix1=0, ix2=0;
  rt3DPointBufferDataObject::SimplePoint pp[10][10];

  label = rtObjectManager::instance().addObjectOfType(rtConstants::OT_TextLabel, "MyLabel");
  label2 = rtObjectManager::instance().addObjectOfType(rtConstants::OT_TextLabel, "MyLabel2");
  pointList = rtObjectManager::instance().addObjectOfType(rtConstants::OT_3DPointBuffer, "TestPointBuffer");

  static_cast<rt3DPointBufferDataObject*>(pointList->getDataObject())->addPoint(p1);
  static_cast<rt3DPointBufferDataObject*>(pointList->getDataObject())->addPoint(p2);
  static_cast<rt3DPointBufferDataObject*>(pointList->getDataObject())->addPoint(p3);
  static_cast<rt3DPointBufferDataObject*>(pointList->getDataObject())->addPoint(p4);

  // Add 100 semi-transparent spheres.
  for (ix1=2; ix1<12; ix1++) {
    for (ix2=2; ix2<12; ix2++) {
      pp[ix1-2][ix2-2].px = ix1;
      pp[ix1-2][ix2-2].py = ix2;
      pp[ix1-2][ix2-2].pz = 2;
      pp[ix1-2][ix2-2].pSize = 0.25;
      pp[ix1-2][ix2-2].pProp->SetColor(0,1,0);
      pp[ix1-2][ix2-2].pProp->SetOpacity(0.2);
      static_cast<rt3DPointBufferDataObject*>(pointList->getDataObject())->addPoint(pp[ix1-2][ix2-2]);
    }
  }

  pointList->update();

  cath = rtObjectManager::instance().addObjectOfType(rtConstants::OT_Cath, "Cath Object 1 Coil");
  cath2 = rtObjectManager::instance().addObjectOfType(rtConstants::OT_Cath, "Cath Object 2 Coil");
  cath4 = rtObjectManager::instance().addObjectOfType(rtConstants::OT_Cath, "Cath Object 4 Coil");

  int c11 = static_cast<rtCathDataObject*>(cath->getDataObject())->addCoil(0);
  static_cast<rtCathDataObject*>(cath->getDataObject())->setCoilCoords(c11, 1.5, 1.5, 1.5);
  static_cast<rtCathDataObject*>(cath->getDataObject())->setCoilSNR(c11, 50);
  static_cast<rtCathDataObject*>(cath->getDataObject())->Modified();

  int c21 = static_cast<rtCathDataObject*>(cath2->getDataObject())->addCoil(0);
  int c22 = static_cast<rtCathDataObject*>(cath2->getDataObject())->addCoil(1);
  static_cast<rtCathDataObject*>(cath2->getDataObject())->setCoilCoords(c21, 1.5, 1.5, 1.5);
  static_cast<rtCathDataObject*>(cath2->getDataObject())->setCoilSNR(c21, 50);
  static_cast<rtCathDataObject*>(cath2->getDataObject())->setCoilCoords(c22, 3.5, 2.5, 1.5);
  static_cast<rtCathDataObject*>(cath2->getDataObject())->setCoilSNR(c22, 25);
  static_cast<rtCathDataObject*>(cath2->getDataObject())->Modified();

  int c41 = static_cast<rtCathDataObject*>(cath4->getDataObject())->addCoil(0);
  int c42 = static_cast<rtCathDataObject*>(cath4->getDataObject())->addCoil(0);
  int c43 = static_cast<rtCathDataObject*>(cath4->getDataObject())->addCoil(1);
  int c44 = static_cast<rtCathDataObject*>(cath4->getDataObject())->addCoil(2);

  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilCoords(c41, 1.5, 1.5, 1.5);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilSNR(c41, 50);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilCoords(c42, 1.6, 1.4, 1.3);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilSNR(c42, 25);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilCoords(c43, 3.5, 2.5, 1.5);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilSNR(c43, 40);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilCoords(c44, 4.5, 2.0, 2.0);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->setCoilSNR(c44, 40);
  static_cast<rtCathDataObject*>(cath4->getDataObject())->Modified();

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    rtMainWindow mainWin;
    
    rtTimeManager::instance().startRenderTimer(&mainWin, 40);
    rtObjectManager::instance().setMainWinHandle(&mainWin);

    loadTestData();

    mainWin.show();
    return app.exec();
}
