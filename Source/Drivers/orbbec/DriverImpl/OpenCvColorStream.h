///*****************************************************************************
//*									     *
//*  OpenNI 2.x Alpha							     *
//*  Copyright (C) 2012 PrimeSense Ltd.					     *
//*									     *
//*  This file is part of OpenNI. 					     *
//*									     *
//*  Licensed under the Apache License, Version 2.0 (the "License");	     *
//*  you may not use this file except in compliance with the License.	     *
//*  You may obtain a copy of the License at				     *
//*									     *
//*      http://www.apache.org/licenses/LICENSE-2.0			     *
//*									     *
//*  Unless required by applicable law or agreed to in writing, software	     *
//*  distributed under the License is distributed on an "AS IS" BASIS,	     *
//*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
//*  See the License for the specific language governing permissions and	     *
//*  limitations under the License.					     *
//*									     *
//*****************************************************************************/
#ifndef OPENCVCOLORSTREAM_H
#define OPENCVCOLORSTREAM_H

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
//#include "XnOniMapStream.h"
#include <Driver/OniDriverAPI.h>
#include "../Sensor/XnSensor.h"
#include "XnOniDevice.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------
class OpenCvColorStream :
	public oni::driver::StreamBase
{
public:
	OpenCvColorStream(XnSensor* pSensor, XnOniDevice* pDevice);

  virtual OniStatus start();
  virtual void stop();

	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniBool isPropertySupported(int propertyId);


  virtual int getRequiredFrameSize();

	XnStatus GetVideoMode(OniVideoMode* pVideoMode);
	XnStatus SetVideoMode(OniVideoMode* pVideoMode);

	XnStatus GetMirror(OniBool* pEnabled);
	XnStatus SetMirror(OniBool* pEnabled);

	XnStatus GetCropping(OniCropping &cropping);
	XnStatus SetCropping(const OniCropping &cropping);

private:
  cv::VideoCapture m_cap;
  int m_frameIdx;

  OniVideoMode m_videoMode;
  bool m_mirroring;
  OniCropping m_cropping;

  int getSensorIndex() const;

  XN_THREAD_HANDLE m_threadHandle;
  bool m_running;
  static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);
  void mainLoop();

};

#endif // OPENCVCOLORSTREAM_H
