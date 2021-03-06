#include "HistoryData.h"
#include <iostream>
#include <cmath>

#include "rtBasic3DPointData.h"
#include "rt3DTimePointData.h"

#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

HistoryData::HistoryData(rtCathDataObject* cath, rt3DPointBufferDataObject* points)
 : m_pointSize(1)
{
  m_cath   = cath;
  m_points = points;
  
  m_fAutoTrack = false;

  m_prevAutoTrackPoint.setPoint(-1000, -1000, -1000);
  
  m_autoTrackDistanceThreshold = 2;
}

HistoryData::~HistoryData()
{
}

bool HistoryData::equivalentTo(rtCathDataObject* cath, rt3DPointBufferDataObject* points) {
  return (m_cath == cath && m_points == points);
}

rt3DTimePointData HistoryData::createPoint(double pos[3])
{
    rt3DTimePointData p;

    p.setPoint(pos);

    p.setPointSize(m_pointSize);

    p.setColor(1, 0, 0); // red
    p.getProperty()->SetOpacity(0.5);
    return p;
}

bool HistoryData::getCathPosition(double cathPos[3])
{
    //get (first) catheter position
       QList<int> locs;
       locs    = m_cath->getLocationList();
       if (locs.isEmpty())
           return false;

       // get position (Location starts at 1...)
        return m_cath->getPositionAtLocation(locs[0], cathPos);   /// xxxxxx should we use locs[1]?
}

bool HistoryData::savePoint()
  {
   // fail if either object doesn't exist
    if (!m_cath || !m_points)
      return false;

   double pos[3];
   if (!getCathPosition(pos))
       return false;
      
   // create a point at this position
    m_points->lock();
    m_points->addTimePoint(createPoint(pos));
    m_points->Modified();
    m_points->unlock();

    return true;
  }

bool HistoryData::saveSetPoint(int set)
  {
   // fail if either object doesn't exist
    if (!m_cath || !m_points)
      return false;

   // get (first) catheter position
    double pos[3];
    if (!getCathPosition(pos))
        return false;

   // create a point at this position
    QList<QPair<QString,double> > tags;
    tags.append(qMakePair(QString("Set"),(double)set));
    m_points->lock();
    m_points->addCustomTimePoint(createPoint(pos),tags);
    m_points->Modified();
    m_points->unlock();

    return true;
  }


void HistoryData::setAutoTrack(bool fEnabled)
  {
    m_fAutoTrack = fEnabled;
    
    if (m_fAutoTrack)
      connect(m_cath, SIGNAL(objectChanged(int)), this, SLOT(doAutoTrack()));
    else
      disconnect(m_cath, SIGNAL(objectChanged(int)), this, SLOT(doAutoTrack()));
  }
  
void HistoryData::doAutoTrack()
  {
   // fail if either object doesn't exist
    if (!m_cath || !m_points)
      return;
      
   // get (first) catheter position
    double pos[3];
    if (!getCathPosition(pos))
        return;
      
   // create a point at this position
    rt3DTimePointData p;
    p = createPoint(pos);

    if (rtBasic3DPointData::findDistance(p, m_prevAutoTrackPoint) < m_autoTrackDistanceThreshold)
      return;

    m_prevAutoTrackPoint = p;
    
    m_points->lock();
    m_points->addTimePoint(p);
    m_points->Modified();
    m_points->unlock();
  }

