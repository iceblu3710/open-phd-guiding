/*
 *  cam_INDI.cpp
 *  PHD Guiding
 *
 *  Created by Geoffrey Hausheer.
 *  Copyright (c) 2009 Geoffrey Hausheer.
 *  All rights reserved.
 * 
 *  Redraw for libindi/baseclient by Patrick Chevalley
 *  Copyright (c) 2014 Patrick Chevalley
 *  All rights reserved.
 *
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Craig Stark, Stark Labs nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "phd.h"

#ifdef INDI_CAMERA

#include <iostream>
#include <fstream>

#include "config_INDI.h"
#include "camera.h"
#include "time.h"
#include "image_math.h"
#include "cam_INDI.h"

void  Camera_INDIClass::newBLOB(IBLOB *bp)
{
  printf("Got camera blob %s \n",bp->name);
  cam_bp = bp;
  modal = false;
}

/*
static void connect_cb(struct indi_prop_t *iprop, void *data) {
//printf("entering cam_INDI connect_cb\n");
    Camera_INDIClass *cb = (Camera_INDIClass *)(data);
    cb->Connected = (iprop->state == INDI_STATE_IDLE || iprop->state == INDI_STATE_OK) && indi_prop_get_switch(iprop, "CONNECT");
    printf("Camera connected state: %d\n", cb->Connected);
    cb->CheckState();
}
*/

/*
static void new_prop_cb(struct indi_prop_t *iprop, void *callback_data) {
//printf("entering cam_INDI new_prop_cb\n");
    Camera_INDIClass *cb = (Camera_INDIClass *)(callback_data);
    return cb->NewProp(iprop);
}
*/
Camera_INDIClass::Camera_INDIClass() 
{
    expose_prop = NULL;
    frame_prop = NULL;
    frame_type_prop = NULL;
    binning_prop = NULL;
    video_prop = NULL;
    camera_device = NULL;
    Connected = false;
    ready = false;
    INDIhost = pConfig->Profile.GetString("/indi/INDIhost", _T("localhost"));
    INDIport = pConfig->Profile.GetLong("/indi/INDIport", 7624);
    INDICameraName = pConfig->Profile.GetString("/indi/INDIcam", _T("INDI Camera"));
    INDICameraPort = pConfig->Profile.GetString("/indi/INDIcam_port",_T(""));
    PropertyDialogType = PROPDLG_ANY;
    FullSize = wxSize(640,480);
}


void Camera_INDIClass::CheckState() 
{
    if(has_blob && Connected && (expose_prop || video_prop)) {
        if (! ready) {
            printf("Camera is ready\n");
            ready = true;
            if (modal) {
                modal = false;
            }
        }
    }
}

void Camera_INDIClass::newDevice(INDI::BaseDevice *dp)
{
   camera_device = dp;
}

void Camera_INDIClass::newSwitch(ISwitchVectorProperty *svp)
{
    printf("Receving Switch: %s = %i\n", svp->name, svp->sp->s);
}

void Camera_INDIClass::newMessage(INDI::BaseDevice *dp, int messageID)
{
    printf("Receving message: %s\n", dp->messageQueue(messageID));
}

void Camera_INDIClass::newNumber(INumberVectorProperty *nvp)
{
    printf("Receving Number: %s = %g\n", nvp->name, nvp->np[0].value);
}

void Camera_INDIClass::newProperty(INDI::Property *property) 
{
   const char* PropName = property->getName();
   INDI_TYPE Proptype = property->getType();
   printf("Property: %s\n",PropName);
    
    if (Proptype == INDI_BLOB) {
        printf("Found BLOB property for camera %s\n", PropName);
        has_blob = 1;
    }
    else if (strcmp(PropName, "CCD_EXPOSURE") == 0) {
	printf("Found CCD_EXPOSURE for camera %s\n", PropName);
	expose_prop = property;
    }
    else if (strcmp(PropName, "CCD_FRAME") == 0) {
	printf("Found CCD_FRAME for camera %s\n", PropName);
	frame_prop = property;
    }
    else if (strcmp(PropName, "CCD_FRAME_TYPE") == 0) {
	printf("Found CCD_FRAME_TYPE for camera %s\n", PropName);
	frame_type_prop = property;
    }
    else if (strcmp(PropName, "CCD_BINNING") == 0) {
	printf("Found CCD_BINNING for camera %s\n", PropName);
	binning_prop = property;
    }
    else if (strcmp(PropName, "VIDEO_STREAM") == 0) {
	printf("Found Video Camera %s\n", PropName);
	video_prop = property;
    }
    else if (strcmp(PropName, "DEVICE_PORT") == 0 && Proptype == INDI_TEXT && INDICameraPort.Length()) {
	char* porttext = (const_cast<char*>((const char*)INDICameraPort.mb_str()));
	printf("Set port for camera %s\n", porttext);
	property->getText()->tp->text = porttext;
	sendNewText(property->getText());
    }
    else if (strcmp(PropName, "CONNECTION") == 0 && Proptype == INDI_SWITCH) {
	printf("Found CONNECTION for camera %s\n", PropName);
	property->getSwitch()->sp->s = ISS_ON;
	sendNewSwitch(property->getSwitch());
Connected = true;
    }
    CheckState();
}

bool Camera_INDIClass::Connect() 
{
    setServer(INDIhost.mb_str(wxConvUTF8), INDIport);
    watchDevice(INDICameraName.mb_str(wxConvUTF8));
    if (connectServer()) {
	setBLOBMode(B_ALSO, INDICameraName.mb_str(wxConvUTF8), NULL);
	modal = true;
	wxLongLong msec = wxGetUTCTimeMillis();
	while (modal && wxGetUTCTimeMillis() - msec < 10 * 1000) {
	    ::wxSafeYield();
	}
	modal = false;
	if(! ready) {
	    Disconnect();
	    Connected = false;
	    return true;
	}
	Connected = true;
	return false;
    }
    else {
	return true;
    }
  /*

    bool ShowConfig = false;

    if (! INDIClient) {
	ShowConfig = true;
    }
    
    if (INDICameraName.IsEmpty() || INDICameraName.IsSameAs(_T("INDI Camera"))) {
	ShowConfig = true;
    }

    if (ShowConfig) {
	INDI_Setup();
	if (! INDIClient || INDICameraName.IsEmpty() || INDICameraName.IsSameAs(_T("INDI Camera"))) {
		printf("No INDI camera is set.  Please set INDIcam in the preferences file\n");
		return true;
	}
    }

    modal = true;
    indi_device_add_cb(INDIClient, INDICameraName.ToAscii(), (IndiDevCB)new_prop_cb, this);

    wxLongLong msec = wxGetUTCTimeMillis();
    while (modal && wxGetUTCTimeMillis() - msec < 10 * 1000) {
        ::wxSafeYield();
    }
    modal = false;

    if(! ready)
        return true;
    Connected = true;
    INDIClient->ClientCount++;
    return false;*/
}

bool Camera_INDIClass::Disconnect() 
{
    if (disconnectServer()){
	ready = false;
	Connected = false;
	return false;
    }
    else return true;
}

void Camera_INDIClass::ShowPropertyDialog() 
{
    if (Connected) {
      CameraDialog();
    }
    else {
      CameraSetup();
    }
}

void Camera_INDIClass::CameraDialog() 
{
}

void Camera_INDIClass::CameraSetup() 
{
    INDIConfig *indiDlg = new INDIConfig(wxGetActiveWindow());
    indiDlg->DevName = _T("Camera");
    indiDlg->INDIhost = INDIhost;
    indiDlg->INDIport = INDIport;
    indiDlg->INDIDevName = INDICameraName;
    indiDlg->INDIDevPort = INDICameraPort;
    indiDlg->SetSettings();
    indiDlg->Connect();
    if (indiDlg->ShowModal() == wxID_OK) {
	indiDlg->SaveSettings();
	INDIhost = indiDlg->INDIhost;
	INDIport = indiDlg->INDIport;
	INDICameraName = indiDlg->INDIDevName;
	INDICameraPort = indiDlg->INDIDevPort;
	pConfig->Profile.SetString("/indi/INDIhost", INDIhost);
	pConfig->Profile.SetLong("/indi/INDIport", INDIport);
	pConfig->Profile.SetString("/indi/INDIcam", INDICameraName);
	pConfig->Profile.SetString("/indi/INDIcam_port",INDICameraPort);
    }
    indiDlg->Disconnect();
    indiDlg->Destroy();
    delete indiDlg;
}

bool Camera_INDIClass::ReadFITS(usImage& img) {
    int xsize, ysize;
    fitsfile *fptr;  // FITS file pointer
    int status = 0;  // CFITSIO status value MUST be initialized to zero!
    int hdutype, naxis;
    int nhdus=0;
    long fits_size[2];
    long fpixel[3] = {1,1,1};
    size_t bsize = static_cast<size_t>(cam_bp->bloblen);
    
    if (fits_open_memfile(&fptr,
            "",
            READONLY,
            &(cam_bp->blob),
            &bsize,
            0,
            NULL,
            &status) )
    {
        // Got status 252 with libindiclient from Debian, the first four character of blob 
        // are overwriten. Work with INDI trunk.
        pFrame->Alert(_("Unsupported type or read error loading FITS file"));
        return true;
    }
    if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU) {
        pFrame->Alert(_("FITS file is not of an image"));
        return true;
    }

    // Get HDUs and size
    fits_get_img_dim(fptr, &naxis, &status);
    fits_get_img_size(fptr, 2, fits_size, &status);
    xsize = (int) fits_size[0];
    ysize = (int) fits_size[1];
    fits_get_num_hdus(fptr,&nhdus,&status);
    if ((nhdus != 1) || (naxis != 2)) {
        pFrame->Alert(_("Unsupported type or read error loading FITS file"));
        return true;
    }
    if (img.Init(xsize,ysize)) {
        pFrame->Alert(_("Memory allocation error"));
        return true;
    }
    if (fits_read_pix(fptr, TUSHORT, fpixel, xsize*ysize, NULL, img.ImageData, NULL, &status) ) { // Read image
        pFrame->Alert(_("Error reading data"));
        return true;
    }
    fits_close_file(fptr,&status);
    return false;
}

/*
bool Camera_INDIClass::ReadStream(usImage& img) {
    int xsize, ysize;
    unsigned char *inptr;
    unsigned short *outptr;
    struct indi_elem_t *elem;

    if (! frame_prop) {
        pFrame->Alert(_("Failed to determine image dimensions"));
        return true;
    }
    if (! (elem = indi_find_elem(frame_prop, "WIDTH"))) {
        pFrame->Alert(_("Failed to determine image dimensions"));
        return true;
    }
    xsize = elem->value.num.value;
    if (! (elem = indi_find_elem(frame_prop, "HEIGHT"))) {
        pFrame->Alert(_("Failed to determine image dimensions"));
        return true;
    }
    ysize = elem->value.num.value;
    if (img.Init(xsize,ysize)) {
        pFrame->Alert(_("Memory allocation error"));
        return true;
    }
    outptr = img.ImageData;
    inptr = (unsigned char *)blob_elem->value.blob.data;
    for (int i = 0; i < xsize * ysize; i++)
        *outptr ++ = *inptr++;
    return false;
}
*/

bool Camera_INDIClass::Capture(int duration, usImage& img, wxRect subframe, bool recon)
{
  if (Connected) {
      if (expose_prop) {
	  printf("Exposing for %d(ms)\n", duration);
	  expose_prop->getNumber()->np->value = duration/1000;
	  sendNewNumber(expose_prop->getNumber());
      }
      else if (video_prop){
	  printf("Enabling video capture\n");
	  video_prop->getSwitch()->sp->s = ISS_ON;
	  sendNewSwitch(video_prop->getSwitch());
      }
      else {
	  return true;
      }

      modal = true;
      
      unsigned long loopwait = duration > 100 ? 10 : 1;
      
      CameraWatchdog watchdog(duration);
      
      while (modal) {
	  wxMilliSleep(loopwait);
	  if (WorkerThread::TerminateRequested())
	      return true;
	  if (watchdog.Expired())
	  {
	      pFrame->Alert(_("Camera timeout during capure"));
	      Disconnect();
	      return true;
	  }
      }
      
      if (video_prop) {
	  printf("Stop video capture\n");
	  video_prop->getSwitch()->sp->s = ISS_OFF;
	  sendNewSwitch(video_prop->getSwitch());
      }

      printf("Exposure end\n");
      
      if (strcmp(cam_bp->format, ".fits") == 0) {
	 printf("Processing fits file\n");
	 if ( ! ReadFITS(img) ) {
	    if ( recon ) {
	       printf("Subtracting dark\n");
	       SubtractDark(img);
	    }
	    return false;
	 } else {
	    return true;
	 }
      } /*else if (strncmp(".stream", blob_elem->value.blob.fmt, 7) == 0) {
	 printf("Processing stream file\n");
	 return ReadStream(img);
      } */else {
	 pFrame->Alert(_("Unknown image format: ") + wxString::FromAscii(cam_bp->format));
	 return true;
      }

  }
  else {
      return true;
  }
  /*
    modal = true;

    if (expose_prop) {
        printf("Exposing for %d(ms)\n", duration);
        indi_dev_enable_blob(expose_prop->idev, TRUE);
        indi_prop_set_number(expose_prop, "CCD_EXPOSURE_VALUE", duration / 1000.0);
        indi_send(expose_prop, NULL);
    } else {
        printf("Enabling video capture\n");
        indi_dev_enable_blob(expose_prop->idev, TRUE);
        indi_send(video_prop, indi_prop_set_switch(video_prop, "ON", TRUE));
    }

    unsigned long loopwait = duration > 100 ? 10 : 1;

    CameraWatchdog watchdog(duration);

    while (modal) {
        wxMilliSleep(loopwait);
        if (WorkerThread::TerminateRequested())
            return true;
        if (watchdog.Expired())
        {
            pFrame->Alert(_("Camera timeout during capure"));
            Disconnect();
            return true;
        }
    }

    if (! expose_prop) {
        indi_send(video_prop, indi_prop_set_switch(video_prop, "OFF", TRUE));
    }

    if (strncmp(".fits",blob_elem->value.blob.fmt, 5) == 0) {
        printf("Processing fits file\n");
        if ( ! ReadFITS(img) ) {
            if ( recon ) {
                printf("Subtracting dark\n");
                SubtractDark(img);
            }
            return false;
        } else {
            return true;
        }
    } else if (strncmp(".stream", blob_elem->value.blob.fmt, 7) == 0) {
        printf("Processing stream file\n");
        return ReadStream(img);
    } else {
        pFrame->Alert(_("Unknown image format: ") + wxString::FromAscii(blob_elem->value.blob.fmt));
        return true;
    }*/
  return true;
}

bool Camera_INDIClass::HasNonGuiCapture(void) 
{
    return true;
}

#endif
