/*****************************************************************************
*									     *
*  OpenNI 2.x Alpha							     *
*  Copyright (C) 2012 PrimeSense Ltd.					     *
*									     *
*  This file is part of OpenNI. 					     *
*									     *
*  Licensed under the Apache License, Version 2.0 (the "License");	     *
*  you may not use this file except in compliance with the License.	     *
*  You may obtain a copy of the License at				     *
*									     *
*      http://www.apache.org/licenses/LICENSE-2.0			     *
*									     *
*  Unless required by applicable law or agreed to in writing, software	     *
*  distributed under the License is distributed on an "AS IS" BASIS,	     *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and	     *
*  limitations under the License.					     *
*									     *
*****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "OpenCvColorStream.h"

#if ONI_PLATFORM == ONI_PLATFORM_WIN32
#include <atlstr.h>
#include <atlbase.h>
#include <dshow.h>
#include <iostream>
#endif

OpenCvColorStream::OpenCvColorStream(XnSensor* pSensor, XnOniDevice* pDevice) 
  :	oni::driver::StreamBase()
  , m_mirroring(true)
{
  m_videoMode.fps = 30;
  m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
  m_videoMode.resolutionX = 640;
  m_videoMode.resolutionY = 480;

  m_cropping.enabled = false;
}

OniStatus OpenCvColorStream::setProperty(int propertyId, const void* data, int dataSize)
{
	XnStatus nRetVal = XN_STATUS_ERROR;

	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		{
			if (dataSize != sizeof(OniVideoMode))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}

			nRetVal = SetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);

			break;
		}
		case ONI_STREAM_PROPERTY_MIRRORING:
		{
			if (dataSize != sizeof(OniBool))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniBool));
				return ONI_STATUS_ERROR;
			}

			nRetVal = SetMirror((OniBool*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);

			break;
		}
		case ONI_STREAM_PROPERTY_CROPPING:
		{
			if (dataSize != sizeof(OniCropping))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", dataSize, sizeof(OniCropping));
				return ONI_STATUS_ERROR;
			}

			nRetVal = SetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);

			break;
		}
		default:
		{
			return oni::driver::StreamBase::setProperty(propertyId, data, dataSize);
		}
	}

	return ONI_STATUS_OK;
}

OniStatus OpenCvColorStream::getProperty(int propertyId, void* data, int* pDataSize)
{
  XnStatus nRetVal = XN_STATUS_ERROR;
	switch(propertyId)
	{
		case ONI_STREAM_PROPERTY_VIDEO_MODE:
		{
			if (*pDataSize != sizeof(OniVideoMode))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetVideoMode((OniVideoMode*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		case ONI_STREAM_PROPERTY_MIRRORING:
		{
			if (*pDataSize != sizeof(OniBool))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniBool));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetMirror((OniBool*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		case ONI_STREAM_PROPERTY_CROPPING:
		{
			if (*pDataSize != sizeof(OniCropping))
			{
				xnLogError(XN_MASK_DEVICE_SENSOR, "Unexpected size: %d != %d", *pDataSize, sizeof(OniCropping));
				return ONI_STATUS_ERROR;
			}

			nRetVal = GetCropping(*(OniCropping*)data);
			XN_IS_STATUS_OK_RET(nRetVal, ONI_STATUS_ERROR);
			break;
		}
		default:
		{
			return oni::driver::StreamBase::getProperty(propertyId, data, pDataSize);
		}
	}

	return ONI_STATUS_OK;
}

OniBool OpenCvColorStream::isPropertySupported(int propertyId)
{
	return (
		propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE ||
		propertyId == ONI_STREAM_PROPERTY_MIRRORING ||
		propertyId == ONI_STREAM_PROPERTY_CROPPING ||
		oni::driver::StreamBase::isPropertySupported(propertyId));
}


XnStatus OpenCvColorStream::GetVideoMode(OniVideoMode* pVideoMode)
{
  *pVideoMode = m_videoMode;
  return XN_STATUS_OK;
}

XnStatus OpenCvColorStream::SetVideoMode(OniVideoMode* pVideoMode)
{
  m_videoMode = *pVideoMode;
  return XN_STATUS_OK;
}

XnStatus OpenCvColorStream::GetMirror(OniBool* pEnabled)
{
  *pEnabled = static_cast<OniBool>(m_mirroring);
  return XN_STATUS_OK;
}

XnStatus OpenCvColorStream::SetMirror(OniBool* pEnabled)
{
  m_mirroring = *pEnabled == TRUE;
  return XN_STATUS_OK;
}

XnStatus OpenCvColorStream::GetCropping(OniCropping &cropping)
{
  cropping = m_cropping;
  return XN_STATUS_OK;
}

XnStatus OpenCvColorStream::SetCropping(const OniCropping &cropping)
{
  m_cropping = cropping;
  return XN_STATUS_OK;
}

int OpenCvColorStream::getSensorIndex() const
{
  //TODO: find cross-platform device discovery/caputer API
  int deviceIndex = 0;
#if ONI_PLATFORM == ONI_PLATFORM_WIN32
  // find correct UVC cam
  //TODO: this only works for a single Astra Pro, so find a simple way to match the UVC RGB cam to the active depth cam
  bool deviceFound = false;
  HRESULT hr;
  hr = CoInitialize(NULL);
  if (SUCCEEDED(hr)) 
  {
    CComPtr<ICreateDevEnum> deviceEnumerator;
    CComPtr<IEnumMoniker>   enumMoniker;

    // create enumerator
    deviceEnumerator.CoCreateInstance(CLSID_SystemDeviceEnum);  
    if (deviceEnumerator) 
    {
      // enumerate video capture devices
      deviceEnumerator->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
      if(enumMoniker) 
      {
        enumMoniker->Reset();
          
        deviceIndex = 0;

        // go through video capture devices
        while(1) 
        {
          ULONG ulFetched = 0;
          CComPtr<IMoniker> pM;
          hr = enumMoniker->Next( 1, &pM, &ulFetched );
          if(hr != S_OK)  break;

          // get the property bag interface from the moniker
          CComPtr<IPropertyBag> pBag;
          hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
          if (hr != S_OK) continue;

          // ask for the english-readable name
          CComVariant var;
          var.vt = VT_BSTR;
          hr = pBag->Read(L"FriendlyName", &var, NULL);
          if (hr != S_OK) continue;

          const CString deviceName(var.bstrVal);
          if (deviceName == "Astra Pro HD Camera")
          {
            // just use the first Astra Pro RGB cam you find
            deviceFound = true;
            break;
          }

          ++deviceIndex;
        }
      }
    }
  }
  CoUninitialize();

  if (!deviceFound)
    return -1;
#endif

  return deviceIndex;
}

OniStatus OpenCvColorStream::start()
{
  if (m_running != TRUE) 
  {
    const int deviceIndex = getSensorIndex();
    if (deviceIndex < 0)
      return ONI_STATUS_ERROR;

    m_cap.open(deviceIndex);
	  if(!m_cap.isOpened())
    {
		  return ONI_STATUS_ERROR;
    }

    // By default, the RGB camera streams with 15 fps. So you have set the FPS to 30 to make sure you actually get 30 fps.
    // But the camera (or OpenCV(?)) also streams images by default as YUY2 format. That's 16bit per pixel, meaning at 
    // 1280x720@30fps it's too much data for the maximum USB2 bandwidth. So instead we have to use MJPEG if possible.
    // These options can by changed by setting cv::VideoCapture properties, but...
    // WARNING: The order these properties are set in matters!
    // Setting MJPG codec only works, if you DON'T set FPS afterwards, and only if you DO set FRAME_WIDTH afterwards.
    // Therefore, you first have to set FPS, then MJPG, then the resolution.
    // And even then, it won't work for 640x480. For some reason that still streams as YUY2. But at that size, the 
    // USB2 bandwidth isn't exceeded, so that's not so bad. But it's still weird.
    m_cap.set(CV_CAP_PROP_FPS, m_videoMode.fps);
    m_cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G') );
    m_cap.set(CV_CAP_PROP_FRAME_WIDTH, m_videoMode.resolutionX);
    m_cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_videoMode.resolutionY);

    // just some debug output...
    //std::cout<<"CV_CAP_PROP_FRAME_WIDTH: "<<m_cap.get(CV_CAP_PROP_FRAME_WIDTH)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FRAME_HEIGHT: "<<m_cap.get(CV_CAP_PROP_FRAME_HEIGHT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FPS: "<<m_cap.get(CV_CAP_PROP_FPS)<<std::endl;
    //int ex = static_cast<int>(m_cap.get(CV_CAP_PROP_FOURCC));
    //char EXT[] = {(char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0};
    //std::cout<<"CV_CAP_PROP_FOURCC: "<<EXT<<std::endl;
    //std::cout<<"CV_CAP_PROP_FORMAT: "<<m_cap.get(CV_CAP_PROP_FORMAT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_MODE: "<<m_cap.get(CV_CAP_PROP_MODE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_BRIGHTNESS: "<<m_cap.get(CV_CAP_PROP_BRIGHTNESS)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONTRAST: "<<m_cap.get(CV_CAP_PROP_CONTRAST)<<std::endl;
    //std::cout<<"CV_CAP_PROP_SATURATION: "<<m_cap.get(CV_CAP_PROP_SATURATION)<<std::endl;
    //std::cout<<"CV_CAP_PROP_HUE: "<<m_cap.get(CV_CAP_PROP_HUE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_GAIN: "<<m_cap.get(CV_CAP_PROP_GAIN)<<std::endl;
    //std::cout<<"CV_CAP_PROP_EXPOSURE: "<<m_cap.get(CV_CAP_PROP_EXPOSURE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONVERT_RGB: "<<m_cap.get(CV_CAP_PROP_CONVERT_RGB)<<std::endl;
    //std::cout<<": "<<m_cap.get()<<std::endl;

    m_frameIdx = 0;
    
    XnStatus nRetVal = xnOSCreateThread(threadFunc, this, &m_threadHandle);
    if (nRetVal != XN_STATUS_OK) 
    {
      return ONI_STATUS_ERROR;
    }
  }

  return ONI_STATUS_OK;
}

void OpenCvColorStream::stop()
{
  if (m_running == true)
  {
    m_running = false;
    xnOSWaitForThreadExit(m_threadHandle, INFINITE);
    xnOSCloseThread(&m_threadHandle);

    m_cap.release();
  }
}

XN_THREAD_PROC OpenCvColorStream::threadFunc(XN_THREAD_PARAM pThreadParam)
{
  OpenCvColorStream* pStream = (OpenCvColorStream*)pThreadParam;
  pStream->mainLoop();
  XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}

void OpenCvColorStream::mainLoop()
{
  m_running = TRUE;
  while (m_running) 
  {
    cv::Mat frame,rgb;

    //std::cout<<std::endl<<"####### Before reading frame ##############"<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_MSEC: "<<m_cap.get(CV_CAP_PROP_POS_MSEC)<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_FRAMES: "<<m_cap.get(CV_CAP_PROP_POS_FRAMES)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FPS: "<<m_cap.get(CV_CAP_PROP_FPS)<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_AVI_RATIO: "<<m_cap.get(CV_CAP_PROP_POS_AVI_RATIO)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FRAME_COUNT: "<<m_cap.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FORMAT: "<<m_cap.get(CV_CAP_PROP_FORMAT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_MODE: "<<m_cap.get(CV_CAP_PROP_MODE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_BRIGHTNESS: "<<m_cap.get(CV_CAP_PROP_BRIGHTNESS)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONTRAST: "<<m_cap.get(CV_CAP_PROP_CONTRAST)<<std::endl;
    //std::cout<<"CV_CAP_PROP_SATURATION: "<<m_cap.get(CV_CAP_PROP_SATURATION)<<std::endl;
    //std::cout<<"CV_CAP_PROP_HUE: "<<m_cap.get(CV_CAP_PROP_HUE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_GAIN: "<<m_cap.get(CV_CAP_PROP_GAIN)<<std::endl;
    //std::cout<<"CV_CAP_PROP_EXPOSURE: "<<m_cap.get(CV_CAP_PROP_EXPOSURE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONVERT_RGB: "<<m_cap.get(CV_CAP_PROP_CONVERT_RGB)<<std::endl;
    //std::cout<<"CV_CAP_PROP_WHITE_BALANCE_U: "<<m_cap.get(CV_CAP_PROP_WHITE_BALANCE_U)<<std::endl;
    //std::cout<<"CV_CAP_PROP_WHITE_BALANCE_V: "<<m_cap.get(CV_CAP_PROP_WHITE_BALANCE_V)<<std::endl;
    //std::cout<<"CV_CAP_PROP_RECTIFICATION: "<<m_cap.get(CV_CAP_PROP_RECTIFICATION)<<std::endl;
    //std::cout<<"CV_CAP_PROP_ISO_SPEED: "<<m_cap.get(CV_CAP_PROP_ISO_SPEED)<<std::endl;
    //std::cout<<"CV_CAP_PROP_BUFFERSIZE: "<<m_cap.get(CV_CAP_PROP_BUFFERSIZE)<<std::endl;


		m_cap >> frame;

    //std::cout<<"####### After reading frame ##############"<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_MSEC: "<<m_cap.get(CV_CAP_PROP_POS_MSEC)<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_FRAMES: "<<m_cap.get(CV_CAP_PROP_POS_FRAMES)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FPS: "<<m_cap.get(CV_CAP_PROP_FPS)<<std::endl;
    //std::cout<<"CV_CAP_PROP_POS_AVI_RATIO: "<<m_cap.get(CV_CAP_PROP_POS_AVI_RATIO)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FRAME_COUNT: "<<m_cap.get(CV_CAP_PROP_FRAME_COUNT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_FORMAT: "<<m_cap.get(CV_CAP_PROP_FORMAT)<<std::endl;
    //std::cout<<"CV_CAP_PROP_MODE: "<<m_cap.get(CV_CAP_PROP_MODE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_BRIGHTNESS: "<<m_cap.get(CV_CAP_PROP_BRIGHTNESS)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONTRAST: "<<m_cap.get(CV_CAP_PROP_CONTRAST)<<std::endl;
    //std::cout<<"CV_CAP_PROP_SATURATION: "<<m_cap.get(CV_CAP_PROP_SATURATION)<<std::endl;
    //std::cout<<"CV_CAP_PROP_HUE: "<<m_cap.get(CV_CAP_PROP_HUE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_GAIN: "<<m_cap.get(CV_CAP_PROP_GAIN)<<std::endl;
    //std::cout<<"CV_CAP_PROP_EXPOSURE: "<<m_cap.get(CV_CAP_PROP_EXPOSURE)<<std::endl;
    //std::cout<<"CV_CAP_PROP_CONVERT_RGB: "<<m_cap.get(CV_CAP_PROP_CONVERT_RGB)<<std::endl;
    //std::cout<<"CV_CAP_PROP_WHITE_BALANCE_U: "<<m_cap.get(CV_CAP_PROP_WHITE_BALANCE_U)<<std::endl;
    //std::cout<<"CV_CAP_PROP_WHITE_BALANCE_V: "<<m_cap.get(CV_CAP_PROP_WHITE_BALANCE_V)<<std::endl;
    //std::cout<<"CV_CAP_PROP_RECTIFICATION: "<<m_cap.get(CV_CAP_PROP_RECTIFICATION)<<std::endl;
    //std::cout<<"CV_CAP_PROP_ISO_SPEED: "<<m_cap.get(CV_CAP_PROP_ISO_SPEED)<<std::endl;
    //std::cout<<"CV_CAP_PROP_BUFFERSIZE: "<<m_cap.get(CV_CAP_PROP_BUFFERSIZE)<<std::endl;
    //std::cout<<std::endl;

		if(frame.empty())
			break;

    OniFrame* pFrame = getServices().acquireFrame();
    pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
    pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
    pFrame->croppingEnabled = m_cropping.enabled;
    if (m_cropping.enabled)
    {
      pFrame->cropOriginX = m_cropping.originX;
      pFrame->cropOriginY = m_cropping.originY;
      pFrame->width = m_cropping.width;
      pFrame->height = m_cropping.height;
    }
    else
    {
      pFrame->cropOriginX = 0;
      pFrame->cropOriginY = 0;
      pFrame->width = m_videoMode.resolutionX;
      pFrame->height = m_videoMode.resolutionY;
    }
    pFrame->dataSize = pFrame->height * pFrame->width * sizeof(OniRGB888Pixel);
    pFrame->stride = pFrame->width * sizeof(OniRGB888Pixel);
    pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
    pFrame->videoMode.fps = m_videoMode.fps;
    pFrame->sensorType = ONI_SENSOR_COLOR;
    pFrame->frameIndex = m_frameIdx++;
    //pFrame->timestamp = timestamp / 10;
    
    cv::cvtColor(frame, rgb, cv::COLOR_RGB2BGR);
    if (m_mirroring)
      cv::flip(rgb, rgb, 1);
    
    uchar* data_in = rgb.data;
    OniRGB888Pixel* data_out = reinterpret_cast<OniRGB888Pixel*>(pFrame->data);

    //TODO: cropping
    memcpy(data_out, data_in, m_videoMode.resolutionX * m_videoMode.resolutionY * sizeof(OniRGB888Pixel));


    //for (int y = 0; y < 480; ++y) {
    //  for (int x = 639; x >= 0; --x) {
    //    uchar* iter = data_in + (y * 640 + x) * sizeof(OniRGB888Pixel);
    //    data_out->b = *iter++;
    //    data_out->g = *iter++;
    //    data_out->r = *iter++;
    //    data_out++;
    //  }
    //}

    raiseNewFrame(pFrame);
    getServices().releaseFrame(pFrame);
  }
}

int OpenCvColorStream::getRequiredFrameSize()
{
	return m_videoMode.resolutionX*m_videoMode.resolutionY*sizeof(OniRGB888Pixel);
}
