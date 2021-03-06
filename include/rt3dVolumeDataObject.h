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
#ifndef RT_3D_VOLUME_DATA_OBJECT_H
#define RT_3D_VOLUME_DATA_OBJECT_H

#include "rtDataObject.h"
#include "ui_volume3DOptions.h"
#include "rtWindowLevelDialog.h"
#include "DICOMCommonData.h"

#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkImageShiftScale.h"
#include "vtkImageExtractComponents.h"

#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastIsosurfaceFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkImageClip.h>

#include <QTimer>

#include "ObjectSelectionComboBox.h"

//! 3D Volume Data Object
/*!
  Data object that represents a 3D object. This object is also capable of playing back 4D (3D+time) cine volumes.
  */
class rt3DVolumeDataObject : public rtDataObject
{
Q_OBJECT

public:

  //! Different types of ray cast functions
  /*!
    Composite - Render the voxles based on the tracing rays to the first visible voxel and rendering that.
    Isosurface - Create a surface based on a threshold value.
    MIP - Maximum intensity projection also based on tracing rays through the volume and rendering the voxel of highest intensity.
    */
  enum RayCastFunction {
    RCF_COMPOSITE,
    RCF_ISOSURFACE,
    RCF_MIP
  };

  //! Constructor
  rt3DVolumeDataObject();

  //! Destructor
  ~rt3DVolumeDataObject();

  //! Get a handle to the image data object.
  /*!
  This function provides a way to access and to modify the image data object.
  Do not delete the handle returned by this object. The creation and deletion is handled by the Data Object.
  \return A handle to the image data object.
  */
  vtkImageData* getImageData();

  //! Get the image data but cast to Unsigned Short
  /*!
  The mapper requires either unsigned short or unsigned char to work properly. This function makes it easier to implement that mapper.
  \return A pointer to the unsigned short version of the data.
  */
  vtkImageData* getUShortData();


  //! Get a handle to the transform for the 3D volume
  /*!
  This function provides a way to access and to modify the transform.
  Do not delete the handle returned by this object. The creation and deletion is handled by the Data Object.
  \return A handle to the image data object.
  */
  vtkTransform* getTransform();

  //! Get the piecewise function for the 3D volume
  vtkPiecewiseFunction* getPieceFunc();

  //! Get the current color transfer function
  vtkColorTransferFunction* getColorTransFunc();

  //! Get volume property
  vtkVolumeProperty* getVolumeProperty();

  //! Get the type of ray cast function
  RayCastFunction getRayCastType() { return m_rayCastFunction; }

  //! Get the ray cast function
  vtkVolumeRayCastFunction* getRayCastFunction();

  //! Set the new image data.
  /*!
  Setting the new image data will cause a lot of re-setting of parameters and options. Volume properites and transfer functions will be modified. The image data will be copied over to this object. This finction emits a newImageData() signal if successful.
  @param temp The ImageData to be copied.
  @param type The type of image (0 = DICOM, 1 = Color Images)
  @return true if the data was copied correctly.
  */
  bool copyNewImageData(vtkImageData* temp,int type = 0);

  //! Set a new position transform for the image data.
  /*!
    @param temp The new transform to use.
    @return true if the new transform was copied.
    */
  bool copyNewTransform(vtkTransform* temp);

  //! Set the trigger delay list.
  inline void copyTriggerDelayList(QList<double> *trigDelay) { if(!trigDelay) return; m_triggerList = (*trigDelay); }

  //! Set the DICOM file information.
  inline void copyDicomCommonData(DICOMCommonData* commonData) { if(!commonData) return; m_commonData.deepCopy(commonData); }

  //! Get the trigger delay list
  QList<double> *getTriggerDelayList() { return &m_triggerList; }

  //! Get the Dicom common data
  DICOMCommonData *getDicomCommonData() { return &m_commonData; }

  //! Check if the volume should be rendered.
  bool getRenderRayTraceVolume() { return m_optionsWidget.groupRayCastVolume->isChecked(); }

  //! Check if the 3D axial slice is being shown
  bool getAxial3D() { return m_optionsWidget.checkAxial3D->isChecked(); }
  //! Check if the 3D sagittal slice is being shown
  bool getSagittal3D() { return m_optionsWidget.checkSagittal3D->isChecked(); }
  //! Check if the 3D coronal slice is being shown
  bool getCoronal3D() { return m_optionsWidget.checkCoronal3D->isChecked(); }

  //! Set if the 3D axial slice is being shown
  void setAxial3D(bool set) {m_optionsWidget.checkAxial3D->setChecked(set); }
  //! Set if the 3D sagittal slice is being shown
  void setSagittal3D(bool set) {m_optionsWidget.checkSagittal3D->setChecked(set);}
  //! Set if the 3D coronal slice is being shown
  void setCoronal3D(bool set) {m_optionsWidget.checkCoronal3D->setChecked(set);}


  //! Get the opacity of the axial slice.
  inline double getAxialOpacity() { return ((double)m_optionsWidget.axialOpacitySlider->value())/100.0; }
  inline double getSagittalOpacity() { return ((double)m_optionsWidget.sagittalOpacitySlider->value())/100.0; }
  inline double getCoronalOpacity() { return ((double)m_optionsWidget.coronalOpacitySlider->value())/100.0; }

  inline void setAxialOpacity(double v) {
    if ( v<0.0 ) v=0.0;
    else if ( v>1.0 ) v = 1.0;
    m_optionsWidget.axialOpacitySlider->setValue(v*100);
    Modified();
  }

  inline void setSagittalOpacity(double v) {
    if ( v<0.0 ) v=0.0;
    else if ( v>1.0 ) v = 1.0;
    m_optionsWidget.sagittalOpacitySlider->setValue(v*100);
    Modified();
  }

  inline void setCoronalOpacity(double v) {
    if ( v<0.0 ) v=0.0;
    else if ( v>1.0 ) v = 1.0;
    m_optionsWidget.coronalOpacitySlider->setValue(v*100);
    Modified();
  }


  //! Check if valid data exists in the data object.
  /*!
    When the data obejct is created the data is not valid. When new image data is copied into the data object then it becomes valid.
    */
  bool isDataValid() { return m_imgDataValid; }

  //! Translate the data object
  void translateData(double x, double y, double z);
  //! Scale the data object
  void scaleData(double x, double y, double z);
  //! Set the direction cosines for X and Y. The Z will be calculated from there. This function must be called FIRST.
  void setDirectionCosinesXY(float* dirCos);

  //! Get the type of interpolation used.
  /*!
    The three types possible are: Nearest Neighbour = 0, Linear = 1, Cubic = 2
    \return The interpolation value as an integer.
    */
  int getInterpolation() { return m_interpolationType; }

  //! Send the info to the GUI
  void update();

  int getVisibleComponent() { return m_visibleComponent; }

  //! Get the window part of the window level.
  int getWindow() { return m_window; }
  //! Get the level part of the window level.
  int getLevel() { return m_level; }
  //! Get the window level dialog
  rtWindowLevelDialog *getWLDialog() {return m_wlDialog;}
  //! Get the Image type
  int getImageType() {return m_imgType;}
  //! Get the axial color
  QColor getAxialColor() {return m_axialColor;}
  //! Get the sagittal color
  QColor getSagittalColor() {return m_sagittalColor;}
  //! Get the coronal color
  QColor getCoronalColor() {return m_coronalColor;}

  //! set the axial color
  void setAxialColor(QColor clr);
  //! set the sagittal color
  void setSagittalColor(QColor clr);
  //! set the coronal color
  void setCoronalColor(QColor clr);

  //! Has the user changed the axial color
  bool didAxialColorChange() {return m_AColor;}
  //! Has the user changed the sagittal color
  bool didSagittalColorChange() {return m_SColor;}
  //! Has the user changed the coronal color
  bool didCoronalColorChange() {return m_CColor;}

  //! Get the image spacing
  void getSpacing(double s[3]) {m_imgUShortCast->GetOutput()->GetSpacing(s);}

  //! Get the annotation status.
  /*!
    \return true if the annotation check box is checked. false otherwise.
    */
  inline bool getDoAnnotate() { return m_optionsWidget.annotateCheckBox->isChecked(); }

  //! Get the annotation string.
  /*!
    \return The annotation string to be displayed.
    */
  inline QString getAnnotation() { return m_annotation; }

  //! Get the transform matrix for a particular plane.
  vtkTransform* getTransformForPlane(int id) {
    if (id < 0 || id > 2) return NULL;
    return m_planeTransform[id];
  }

  //! Set the transform for one of the three planes.
  void setTransformForPlane(int id, vtkTransform* t) {
    if (id < 0 || id > 2) return;
    return m_planeTransform[id]->DeepCopy(t);
  }

  //! Move the three planes so that all three planes intersect with the point
  void movePlanestoPoint(double point[3],double offset,bool axial,bool sagittal,bool coronal) {
      if ( (m_movePoint[0] != point[0]) || (m_movePoint[1] != point[1]) || (m_movePoint[2] != point[2]))
      {
          m_movePoint[0] = point[0];
          m_movePoint[1] = point[1];
          m_movePoint[2] = point[2];
          m_moveOffset = offset;
          Modified();
          emit slicePlaneMoveSignal(axial,sagittal,coronal);
      }
  }

  //! move the axial slice along its own normal
  void moveAxialAlongNormal(double dist) {emit axialNormal(dist);}

  //! move the axial slice along the axial direction
  void moveAxial(double dist) {emit axialAxial(dist);}

  //! move the sagittal slice along its own normal
  void moveSagittalAlongNormal(double dist) {emit sagittalNormal(dist);}

  //! move the sagittal slice along the sagittal direction
  void moveSagittal(double dist) {emit sagittalSagittal(dist);}

  //! move the coronal slice along its own normal
  void moveCoronalAlongNormal(double dist) {emit coronalNormal(dist);}

  //! move the coronal slice along the coronal direction
  void moveCoronal(double dist) {emit coronalCoronal(dist);}


  //! Get the move Point
  void getMovePoint(double point[3]) {
      point[0] = m_movePoint[0];
      point[1] = m_movePoint[1];
      point[2] = m_movePoint[2];
  }

  //! Get the move offset
  double getMoveOffset() {return m_moveOffset;}

  void setCanGPU(bool canGPU) {m_canGPU = canGPU;}

  void setupGPUGUI();

  //! Save this data object to file
  virtual bool saveFile(QFile *file);
  //! Load this data object from file
  virtual bool loadFile(QFile *file);

 public slots:
  //! Slot called when the user changes the surface function.
  void surfaceFunctionChanged();

  //! The color transfer function has changed
  void colorTransferChanged(int);

  //! Choose a new axial color
  void axialColorChanged();
  //! Choose a new sagittal color
  void sagittalColorChanged();
  //! Choose a new coronal color
  void coronalColorChanged();

  //! The piecewise function has changed from the GUI
  void piecewiseChanged(int);

  //! The user has changed the intrpolation type from the GUI
  /*!
    The three types possible are: Nearest Neighbour = 0, Linear = 1, Cubic = 2
    \param interp The interpolation value.
    */
  void interpolationChanged(int interp);

  void setVisibleComponent(int c);
  void nextVisibleComponent();

  void isoValueChanged(int v);

  void cineLoop(bool);

  //! Show the window level dialog
  void showWindowLevel();

  //! The window level has been changed by WL dialog.
  void wlChanged(int w, int l);

  //! The button was pressed to reset the plane to the axial position
  void axialResetSlot();
  //! The button was pressed to reset the plane to the sagittal position
  void sagittalResetSlot();
  //! The button was pressed to reset the plane to the coronal position
  void coronalResetSlot();

  //! Update the info in the GUI text box for this volume object.
  void updateInfoText();

  //! Turn the cropping feature on or off
  void cropStatusChanged(bool);

  //! The slider for the min x value has changed
  void xminSliderChanged(int);
  //! The slider for the max x value has changed
  void xmaxSliderChanged(int);
  //! The slider for the min y value has changed
  void yminSliderChanged(int);
  //! The slider for the max y value has changed
  void ymaxSliderChanged(int);
  //! The slider for the min z value has changed
  void zminSliderChanged(int);
  //! The slider for the max z value has changed
  void zmaxSliderChanged(int);

  //! The checkbox for viewing the bounding box was pressed
  void boundBoxChanged(bool);

  //! Create a new piecewisefunction and select it for the rendering
  void createNewPWF();
  //! Create a new color transfer function and select that for rendering.
  void createNewCTF();

  //! Determine if the ray cast volume is pickable
  inline bool isVolumePickable() { return m_optionsWidget.volumePickableCheckBox->isChecked(); }

  //! Set the volume as pickable or not.
  inline void setVolumePickable(bool pick) {
    if (pick != m_optionsWidget.volumePickableCheckBox->isChecked()) {
      m_optionsWidget.volumePickableCheckBox->setChecked(pick);
      Modified();
    }
  }

  //! Turn GPU rendering on or off
  void gpuBoxChecked(bool flag) {m_gpuBoxOn = flag; setupGPUGUI(); emit gpuSignal(flag);}

  //! Setup volume combo box
  void setupVolumes();
  //! Copy the selected volume's transform
  void copyVolume();

 signals:
  //! The volume has received new image data
  void newImageData();

  //! Reset the axial plane
  void axialResetSignal();
  //! Reset the sagittal plane
  void sagittalResetSignal();
  //! Reset the coronal plane
  void coronalResetSignal();
  //! Tell the render object to turn on/off bounding box
  void boundBoxSignal(bool);
  //! Tell the render object to turn gpu render on/off
  void gpuSignal(bool);

  //! Move the given slice planes to the move point
  void slicePlaneMoveSignal(bool,bool,bool);
  //! Move the axial plane along its normal
  void axialNormal(double);
  //! move the axial plane along the axial direction
  void axialAxial(double);
  //! Move the sagittal plane along its normal
  void sagittalNormal(double);
  //! move the sagittal plane along the sagittal direction
  void sagittalSagittal(double);
  //! Move the coronal plane along its normal
  void coronalNormal(double);
  //! move the coronal plane along the coronal direction
  void coronalCoronal(double);

 protected:
  /////////////
  // Functions
  /////////////
  //! Set the GUI widgets.
  void setupGUI();
  //! Cleanup the GUI widgets.
  void cleanupGUI();

  ////////////////
  // Variables
  ///////////////
  vtkImageData* m_imgData;
  vtkImageClip* m_imgClip;
  vtkTransform* m_dataTransform;
  vtkPiecewiseFunction* m_pieceFunc;
  vtkColorTransferFunction* m_colorTransFunc;
  vtkVolumeProperty* m_volumeProperty;
  vtkImageShiftScale * m_imgUShortCast;

  vtkPiecewiseFunction* m_pieceFuncDefault;
  vtkColorTransferFunction* m_colorTransFuncDefault;

  //! The ID of the external piecewise function used.
  int m_piecewiseFuncID;
  //! The ID of the external color function used.
  int m_colorFuncID;

  //! Meta-data is filled in if it is read from DICOM files.
  DICOMCommonData m_commonData;

  //! The window value
  double m_window;
  //! The level value
  double m_level;

  //! The annotation for this volume.
  QString m_annotation;

  //! The type of image (0 = DICOM, 1 = Color Image)
  int m_imgType;

  QList<double> m_triggerList;
  vtkSmartPointer<vtkImageExtractComponents> m_subImg;
  int m_visibleComponent;
  QTimer *m_cineFrame;

  //! Flag to determine if the image data has been initialized.
  bool m_imgDataValid;

  //! The type of interpolation that will be used by the cut planes and the volume reslice.
  /*! The three types possible are: Nearest Neighbour = 0, Linear = 1, Cubic = 2
    */
  int m_interpolationType;

  // Ray Cast Functions
  RayCastFunction m_rayCastFunction;
  vtkVolumeRayCastCompositeFunction* m_compositeFunc;
  vtkVolumeRayCastIsosurfaceFunction* m_isosurfaceFunc;
  vtkVolumeRayCastMIPFunction* m_MIPFunc;

  // UI objects
  //! The widget for the options for this object
  Ui::volume3DOptions m_optionsWidget;

  //! The dialog object where the user changes window and level paramters.
  rtWindowLevelDialog *m_wlDialog;

  //! Positions for the three planes
  vtkTransform* m_planeTransform[3];

  //! Axial plane color
  QColor m_axialColor;
  //! Sagittal plane color
  QColor m_sagittalColor;
  //! Coronal plane color
  QColor m_coronalColor;

  //! The user changed the axial color
  bool m_AColor;
  //! The user changed the sagittal color
  bool m_SColor;
  //! The user changed the coronal color
  bool m_CColor;

  //! The point to move the three planes to
  double m_movePoint[3];

  //! The offset of the move point
  double m_moveOffset;

  //! The GPU checkbox is on
  bool m_gpuBoxOn;
  //! GPU rendering is supported
  bool m_canGPU;

  //! Color transfer function combo box
  ObjectSelectionComboBox m_CTFunc;
  //! Piecewise function combo box
  ObjectSelectionComboBox m_PWFunc;

private:
  void saveTransformToXML(QString name, vtkTransform *trans, QXmlStreamWriter *writer);
  void loadTransformFromXML(vtkTransform *trans,QXmlStreamReader *reader);

};

#endif 
