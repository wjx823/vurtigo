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

  void removeStereoFeature();

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
