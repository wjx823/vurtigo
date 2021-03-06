#ifndef GEOM_SERVER_PLUGIN_H
#define GEOM_SERVER_PLUGIN_H

#include <QMutex>

#include "rtPluginInterface.h"
#include "SenderThread.h"
#include "SenderSimp.h"
#include "Converter.h"

//Time in milliseconds the plugin calls update()
#define UPDATE_TIME 10

//! Plugin object for GeomServer
/*!
  Workflow from the plugin side and management of high level objects is done here.
 */
class GeomServerPlugin : public QObject, public DataInterface {

  Q_OBJECT
  Q_INTERFACES(DataInterface)

 public:
  void objectModified(int objID);
  bool init();
  void cleanup();
  void update();
  void point3DSelected(double px, double py, double pz, int intensity);
 private:
  //! Thread for GeomServerPlugin::readAndSetData()
  SenderThread senderThread;

  //! A lock to ensure that the update call is not flooded.
  QMutex m_singleUpdate;
};

#endif
