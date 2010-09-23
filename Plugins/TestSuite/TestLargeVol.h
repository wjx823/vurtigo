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
#ifndef TEST_LARGE_VOL_H
#define TEST_LARGE_VOL_H

#include <QThread>

class TestLargeVol : public QThread
{
  Q_OBJECT

public:
  TestLargeVol();
  ~TestLargeVol();

  void run();

public slots:

signals:
  void sendOutput( QString );

protected:
  void createObjects();
  void testObject(int, QString);

  // Object IDs
  int m_hugeVol;
};

#endif // TEST_LARGE_VOL_H
