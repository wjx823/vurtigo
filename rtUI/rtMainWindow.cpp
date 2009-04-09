#include <QtGui>
#include <QApplication>
#include "rtMainWindow.h"
#include <iostream>
#include "rtObjectManager.h"
#include "rtRenderObject.h"
#include "rtDataObject.h"

rtMainWindow::rtMainWindow(QWidget *parent, Qt::WindowFlags flags) {
  setupUi(this);

  m_currentObjectWidget = NULL;
  m_renderFlag3D = false;

  m_render3DVTKWidget = new QVTKWidget(this->frame3DRender);
  m_render3DLayout = new QHBoxLayout();

  m_inter3D = m_render3DVTKWidget->GetInteractor();
  m_renWin3D = m_inter3D->GetRenderWindow();
  m_renderer3D = vtkRenderer::New();
  m_renWin3D->AddRenderer(m_renderer3D);

  m_render3DLayout->addWidget(m_render3DVTKWidget);
  this->frame3DRender->setLayout(m_render3DLayout);

  // The GUI must be stretched more than the tabs. 
  mainRLSplitter->setStretchFactor(0, 1);
  mainRLSplitter->setStretchFactor(1, 25);

  // The 3D view must be larger than the 2D view. 
  mainUDSplitter->setStretchFactor(0, 5);
  mainUDSplitter->setStretchFactor(1, 1);

  m_objectBrowseLayout = new QHBoxLayout();
  objBrowseFrame->setLayout(m_objectBrowseLayout);

  populateObjectTypeNames();
  connectSignals();
  setupObjectTree();
}

//! Destructor
/*!
  Removes the current widget from the viewer at the bottom.
  Also deletes objects that were created in this class. 
 */
rtMainWindow::~rtMainWindow() {
  m_renderFlag3D = false;

  // Remove any remaining widget from the dialog. 
  if (m_currentObjectWidget) {
    m_objectBrowseLayout->removeWidget(m_currentObjectWidget);
    m_currentObjectWidget->setParent(NULL);
    m_currentObjectWidget = NULL;
  }

  if (m_render3DVTKWidget) delete m_render3DVTKWidget;
  if (m_render3DLayout) delete m_render3DLayout;
  if (m_objectBrowseLayout) delete m_objectBrowseLayout;

  if (m_renderer3D) m_renderer3D->Delete();
}

vtkRenderWindow* rtMainWindow::getRenderWindow() {
  return m_renWin3D;
}

vtkRenderWindowInteractor* rtMainWindow::getInteractor() {
  return m_inter3D;
}

//! Get the vtkRenderer object for the 3D view. 
vtkRenderer* rtMainWindow::getRenderer() {
  return m_renderer3D;
}

//! Get the object tree
QTreeWidget* rtMainWindow::getObjectTree() {
  return objectTree;
}

//! Called with the newest set of render objects. 
void rtMainWindow::updateObjectList(QHash<int, rtRenderObject*>* hash) {
  QHash<int, rtRenderObject*>::iterator i;
  QHash<rtConstants::rtObjectType, QTreeWidgetItem *>::iterator wIter;

  // Remove all the items first from the GUI.
  while (objectTree->takeTopLevelItem(0));

  for (wIter = m_topItems.begin(); wIter!=m_topItems.end(); ++wIter){
    objectTree->addTopLevelItem(wIter.value());
  }

  for (i = hash->begin(); i != hash->end(); ++i) {
    i.value()->updateTreeItem();
    m_topItems.value(i.value()->getObjectType())->addChild(i.value()->getTreeItem());
  }
}

void rtMainWindow::setObjectManager(rtObjectManager* man){
  m_objMan = man;
}

//! This slot is called every time a new object is selected in the tree. 
void rtMainWindow::currItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous) {
  rtRenderObject *temp;
  QWidget * baseWid;

  if (!current) return;
  
  // Check for a heading. 
  if (current->columnCount() == 1) {
    if (m_currentObjectWidget) {
      m_objectBrowseLayout->removeWidget(m_currentObjectWidget);
      m_currentObjectWidget->setParent(NULL);
      m_currentObjectWidget = NULL;
    }
    return;
  }

  if (m_currentObjectWidget) {
    m_objectBrowseLayout->removeWidget(m_currentObjectWidget);
    m_currentObjectWidget->setParent(NULL);
  }

  temp = m_objMan->getObjectWithID(current->text(1).toInt());
  baseWid = temp->getDataObject()->getBaseWidget();

  m_objectBrowseLayout->addWidget(baseWid);
  m_currentObjectWidget = baseWid;
}

//! Called when the content of a tree item has changed.
void rtMainWindow::itemChanged(QTreeWidgetItem * current, int column) {
  rtRenderObject *temp;
  vtkProp* propTemp;

  if (!current) return;

  // Check for a heading. 
  if (current->columnCount() == 1) {
    return;
  }

  temp = m_objMan->getObjectWithID(current->text(1).toInt());
  propTemp = temp->get3DPipeline();
  if (current->checkState(column) == Qt::Checked) {
    // Add the item.
    if (!m_renderer3D->HasViewProp(propTemp)) {
      temp->setVisible3D(true);
      m_renderer3D->AddViewProp(propTemp);
      m_renderFlag3D = true;
    }
  } else {
    // Remove the item
    if (m_renderer3D->HasViewProp(propTemp)) {
      temp->setVisible3D(false);
      m_renderer3D->RemoveViewProp(propTemp);
      m_renderFlag3D = true;
    }
  }
   
}

//! Try to render the 3D window.
/*!
  This function will only render if the render flag has been set to true. Once the function runs it will reset the flag back to false.
 */
void rtMainWindow::tryRender3D() {
  if (m_renderFlag3D) {
    m_renWin3D->Render();
    m_renderFlag3D = false;
  }
}

void rtMainWindow::connectSignals() {
  connect(actionQuit, SIGNAL(triggered()), qApp, SLOT(quit()));
  connect(objectTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
  connect(objectTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(itemChanged(QTreeWidgetItem*,int)));
}

void rtMainWindow::setupObjectTree() {
  QHash<rtConstants::rtObjectType, QString>::iterator i;
  QTreeWidgetItem * temp;

  m_topItems.clear();

  for (i = m_rtObjectTypeNames.begin(); i !=  m_rtObjectTypeNames.end(); ++i) {
    temp = new QTreeWidgetItem();
    temp->setText(0, i.value());
    m_topItems.insert(i.key(), temp);
  }
  
}

//! Assign string names to the object types.
/*!
  With string names the objects can be displayed in the GUI.
*/
void rtMainWindow::populateObjectTypeNames() {
  m_rtObjectTypeNames.insert(rtConstants::OT_4DObject, "4D Object");
  m_rtObjectTypeNames.insert(rtConstants::OT_3DObject, "3D Object");
  m_rtObjectTypeNames.insert(rtConstants::OT_2DObject, "2D Object");
  m_rtObjectTypeNames.insert(rtConstants::OT_Cath, "Catheter");
  m_rtObjectTypeNames.insert(rtConstants::OT_vtkMatrix4x4, "4 by 4 vtkMatrix");
  m_rtObjectTypeNames.insert(rtConstants::OT_vtkPolyData, "Polygon Data");
  m_rtObjectTypeNames.insert(rtConstants::OT_vtkPiecewiseFunction, "Piecewise Function");
  m_rtObjectTypeNames.insert(rtConstants::OT_vtkColorTransferFunction, "Color Function");
  m_rtObjectTypeNames.insert(rtConstants::OT_ImageBuffer, "Image Buffer");
  m_rtObjectTypeNames.insert(rtConstants::OT_2DPointBuffer, "2D Point Buffer");
  m_rtObjectTypeNames.insert(rtConstants::OT_3DPointBuffer, "3D Point Buffer");
  m_rtObjectTypeNames.insert(rtConstants::OT_TextLabel, "Text Label");
}
