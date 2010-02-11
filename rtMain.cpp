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

/*
 File Created By: Stefan Pintilie
 File Edited By: Stefan Pintilie
 */

#include <QApplication>
#include <QStringList>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

#include <iostream> 
#include <sstream>
#include <cstdio>

#include "rtApplication.h"

#include "rtMainWindow.h"
#include "rtObjectManager.h"
#include "rtTimeManager.h"
#include "rtPluginLoader.h"
#include "rtBaseHandle.h"
#include "rtMessage.h"
#include "rtConfigOptions.h"

#include "buildParam.h"

#include "rt3dPointBufferRenderObject.h"
#include "rt3dPointBufferDataObject.h"

void setupOrigin();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    bool runVurtigo = true;
    int exitCode = 0;

    rtApplication::instance().displayCopyright();
    rtApplication::instance().initApp();

    QStringList args = app.arguments();

    // Start at 1 since the first element is the name of the program
    for (int ix1=1; ix1<args.count(); ix1++) {
      if (args.at(ix1) == "--version" || args.at(ix1) == "-v") {
        std::stringstream msg;
        msg << "Vurtigo Version: " << VURTIGO_MAJOR_VERSION << "." << VURTIGO_MINOR_VERSION;
        rtMessage::instance().log(msg.str());
        runVurtigo = false; // Just checking the version no need to run the software.
      }
    }

    // Check if the user wants Vurtigo to run and if the config file was loaded properly.
    if (runVurtigo) {
      rtBaseHandle::instance(); // Important to create the object in THIS THREAD.
      rtApplication::instance().getMainWinHandle()->setupHelpFiles();

      // Load default plugins
      QList<QString> pluginList;
      pluginList = rtApplication::instance().getConfigOptions()->getPluginList();

#ifdef DEBUG_VERBOSE_MODE_ON
      rtMessage::instance().debug( QString("main() Processing Plugin List") );
#endif

      for (int ix1=0; ix1<pluginList.size(); ix1++) {
        QFile file(pluginList[ix1]);
        if (!file.exists()) {
          rtMessage::instance().error(__LINE__, __FILE__, QString("File not found: ").append(pluginList[ix1]));
          continue;
        }
        if (!rtApplication::instance().getPluginLoader()->loadPluginsFromConfig(&file)) {
          rtMessage::instance().error(__LINE__, __FILE__, QString("Failed to load plugins from: ").append(pluginList[ix1]));
        }
      }

#ifdef DEBUG_VERBOSE_MODE_ON
      rtMessage::instance().debug( QString("main() Setup Origin") );
#endif
      setupOrigin();
#ifdef DEBUG_VERBOSE_MODE_ON
      rtMessage::instance().debug( QString("main() Show Window") );
#endif
      rtApplication::instance().getMainWinHandle()->show();
#ifdef DEBUG_VERBOSE_MODE_ON
      rtMessage::instance().debug( QString("main() Before call to app.exec()") );
#endif
      exitCode = app.exec();
    }
    return exitCode;
}

void setupOrigin() {
  // Setup the origin.
  rt3DPointBufferRenderObject *renObj = static_cast<rt3DPointBufferRenderObject*>(rtObjectManager::instance().addObjectOfType(rtConstants::OT_3DPointBuffer, "True Origin"));
  rt3DPointBufferDataObject *datObj = static_cast<rt3DPointBufferDataObject*>(renObj->getDataObject());
  rt3DPointBufferDataObject::SimplePoint origin;
  rt3DPointBufferDataObject::SimplePoint xDir, yDir, zDir;
  origin.px = 0.0f;
  origin.py = 0.0f;
  origin.pz = 0.0f;
  origin.pSize = 5.0f;
  datObj->addPoint(origin);

  for (int ix1=10; ix1<=200; ix1+=10) {
    xDir.px = ix1;
    xDir.py = 0.0f;
    xDir.pz = 0.0f;
    xDir.pSize = 2.0f;
    datObj->addPoint(xDir);
    yDir.px = 0.0f;
    yDir.py = ix1;
    yDir.pz = 0.0f;
    yDir.pSize = 2.0f;
    datObj->addPoint(yDir);
    zDir.px = 0.0f;
    zDir.py = 0.0f;
    zDir.pz = ix1;
    zDir.pSize = 2.0f;
    datObj->addPoint(zDir);
  }

  datObj->Modified();


}
