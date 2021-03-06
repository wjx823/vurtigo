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
#ifndef HOVER_POINTS_H
#define HOVER_POINTS_H

#include <QtGui>

/*!
Class that draws the points on the parent and does the algorithmic work for the points

Code based on (undocumented):
http://doc.trolltech.com/4.5/demos-gradients.html
But modified to fix bugs and customize. Extended to add functionality needed by other widgets.

Concept:
There is a "visible space" and "data space" in HoverPoints. Visible space is the space shown in the parent widget.
Data space is what the visible space represents by scaling. Space points and data points are in each
space respectively. The data space can be changed by calling changeDataSpace().
*/
class HoverPoints : public QObject {
    Q_OBJECT

public:
    //Properties of the displayed HoverPoints
    enum PointShape {
        Cirle,
        Rectangle
    };

    //! Denotes whether the point is locked on the side
    enum LockType {
        NoLock = 0x00,
        LockToLeft   = 0x01,
        LockToRight  = 0x02,
        LockToTop    = 0x04,
        LockToBottom = 0x08
    };

    enum SortType {
        SortByX,
        SortByY,
        SortNone
    };

    enum ConnectionType {
        LineConnection,
        CurveConnection,
        NoConnection
    };

    //! Struct to hold the settings for the points
    struct Settings {
        QRectF dataSpace;
        SortType sortType;

        PointShape pointShape;
        QSizeF pointSize;
        QPen pointOutline;
        QBrush pointFill;

        ConnectionType connectionType;
        QPen connectionLine;

        //! True if the boundary points follow the next inner points (to immatate non-claping of a VtkPiecewiseFunction)
        bool followingEnds;
    };

    //! Struct that holds the state of the points
    struct Profile {
        QPolygonF points;
        QVector<LockType> locks;
        QRectF dataSpace;
    };

    HoverPoints(QWidget * parent, Settings & settings);

    bool eventFilter(QObject *object, QEvent *event);
    void firePointChange();
    void paintPoints();

    const QPolygonF & getPoints() { return points; }
    const QVector<LockType> & getLocks() { return locks; }

    QPointF spacePointAt(int index) const;
    QPointF getSpacePoint(QPointF const & dataPoint) const;
    QPointF getDataPoint(QPointF const & spacePoint) const;\

    void loadProfile(const Profile & profile);
    void getProfile(Profile & profile);
    
    Settings & getSettings() { return settings; }
    const QRectF & getDataSpace() { return settings.dataSpace; }

public slots:
    void setDataSpace(const QRectF & dataSpace);
    void setSpacePoints(const QPolygonF & points);
    void setDataPoints(const QPolygonF & points);
    void sortPoints();
    void changeDataPoint(int index, bool isX, QString * const value);
    void removePoint(int index);
    void addSpacePoint(int index, const QPointF & spacePoint);
    void addDataPoint(int index, const QPointF & dataPoint);
    void setPointLock(int pos, HoverPoints::LockType lock) { locks[pos] = lock; sortPoints(); }

signals:
    void pointsChanged(int index = -1);
    void dataSpaceChanged(const QRectF & dataSpace);

private:
    QWidget * parent;
    //! The points in data space
    QPolygonF points;
    //! index of the current points being modified
    int currentIndex;

    Settings settings;
    //! vector with size = points, that states whether the points are locked on the side of the widget
    QVector<LockType> locks;

    inline QRectF pointBoundingRect(int i) const;
    QPointF bound_point(const QPointF &point, const QRectF &bounds, int lock);
    void movePoint(int i, const QPointF &newPos, bool emitChange = true);
    int findUnboundedPoint(int index);
};
#endif // HOVER_POINTS_H
