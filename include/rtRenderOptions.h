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

#ifndef RT_RENDER_OPTIONS_H
#define RT_RENDER_OPTIONS_H

#include "ui_rtRenderOptions.h"

#include <QHash>
#include <QString>

class rtRenderOptions : public QDialog, private Ui::rtRenderOptions {
 Q_OBJECT

public:
  rtRenderOptions();
  ~rtRenderOptions();

  int getRenQuality() { return m_renQuality; }
  int getRenUpdateQuality() { return m_renUpdateQuality; }
  int getStereoType() { return m_currStereo; }

  void setStereoType(int t);
  void setDirectRender(bool);
  void setGLRender(bool);

public slots:
  void renQuality(int);
  void renUpdateQuality(int);
  void stereoComboChanged(QString);

protected:
  int m_renQuality;
  int m_renUpdateQuality;

  int m_currStereo;
  QHash<QString, int> m_stereoTypes;

  bool m_directRender;
  bool m_glRender;
};

#endif