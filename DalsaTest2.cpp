// DalsaTest2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>

#include "stdio.h"
#include "conio.h"
#include "sapclassbasic.h"
#include "ExampleUtils.h"

#include <vector>
static void AcqCallback(SapXferCallbackInfo *pInfo);
static void StartFrameCallback(SapAcqCallbackInfo *pInfo);


int main()
{
	SapAcquisition *Acq = NULL;
	SapAcqDevice *AcqDevice = NULL;
	SapBuffer *Buffers = NULL;
	SapTransfer *Xfer = NULL;
	SapView *View = NULL;


	SapManager::Open();
	//////// Ask questions to user to select acquisition board/device and config file ////////

	// Get total number of boards in the system
	int serverCount = SapManager::GetServerCount();
	int GenieIndex = 0;
	BOOL serverFound = FALSE;
	char serverName[CORSERVER_MAX_STRLEN];
	SapManager::SetDisplayStatusMode(SapManager::StatusLog);
	if (serverCount == 0)
	{
		printf("No device found!\n");
		return FALSE;
	}

	for (int serverIndex = 0; serverIndex < serverCount; serverIndex++)
	{
		if (SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice) != 0)
			serverFound = TRUE;
	}

	GenieIndex = 0;


	// At least one server must be available
	if (!serverFound)
	{
		printf("No camera found!\n");
		return FALSE;
	}
	else
	{
		printf("\nCameras listed by User Defined Name:\n\n");

		char userDefinedName[STRING_LENGTH];
		for (int serverIndex = 0; serverIndex < serverCount; serverIndex++)
		{
			if (SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice) != 0)
			{
				SapManager::GetServerName(serverIndex, serverName, sizeof(serverName));

				SapAcqDevice camera(serverName);
				BOOL status = camera.Create();
				if (status)
				{
					// Get user defined name
					status = camera.GetFeatureValue("DeviceUserID", userDefinedName, sizeof(userDefinedName));
					if (status)
						printf("%d/ %s\n", GenieIndex + 1, userDefinedName);
					else
						printf("%d/ %s\n", GenieIndex + 1, "N/A");
					GenieIndex++;
				}

				// Destroy acquisition device object
				if (!camera.Destroy()) return FALSE;
			}
		}

		printf("\nCameras listed by Server Name:\n\n");
		for (int serverIndex = 0; serverIndex < serverCount; serverIndex++)
		{
			if (SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice) != 0)
			{
				// Get Server Name Value
				SapManager::GetServerName(serverIndex, serverName, sizeof(serverName));
				SapAcqDevice camera(serverName);
				BOOL status = camera.Create();
				if (status)
				{
					printf("%d/ %s\n", GenieIndex + 1, serverName);
					GenieIndex++;
				}
				// Destroy acquisition device object
				if (!camera.Destroy()) return FALSE;
			}
		}

		SapLocation loc(serverName,0);
		Acq = new SapAcquisition(loc, FALSE); //ETHERNET came doesnt work with this?
		AcqDevice = new SapAcqDevice(loc, FALSE);
		Buffers = new SapBufferWithTrash(2, AcqDevice);
		View = new SapView(Buffers, SapHwndAutomatic);
		Xfer = new SapAcqDeviceToBuf(AcqDevice, Buffers, AcqCallback, View);

		// Create acquisition object
		if (AcqDevice && !*AcqDevice && !AcqDevice->Create())
			goto FreeHandles;

	}

	// Create buffer object
	if (Buffers && !*Buffers && !Buffers->Create())
		goto FreeHandles;

	// Create transfer object
	if (Xfer && !*Xfer && !Xfer->Create())
		goto FreeHandles;

	// Create view object
	if (View && !*View && !View->Create())
		goto FreeHandles;


	// Start continous grab
	Xfer->Grab();


	printf("Press any key to stop grab\n");
	CorGetch();

	// Stop grab
	Xfer->Freeze();
	if (!Xfer->Wait(5000))
		printf("Grab could not stop properly.\n");

FreeHandles:
	printf("Press any key to terminate\n");
	CorGetch();

	//unregister the acquisition callback
	if (Acq)
		Acq->UnregisterCallback();

	// Destroy view object
	if (View && *View && !View->Destroy()) return FALSE;

	// Destroy transfer object
	if (Xfer && *Xfer && !Xfer->Destroy()) return FALSE;

	// Destroy buffer object
	if (Buffers && *Buffers && !Buffers->Destroy()) return FALSE;

	// Destroy acquisition object
	if (Acq && *Acq && !Acq->Destroy()) return FALSE;

	// Destroy acquisition object
	if (AcqDevice && *AcqDevice && !AcqDevice->Destroy()) return FALSE;

	// Delete all objects
	if (View)		delete View;
	if (Xfer)		delete Xfer;
	if (Buffers)	delete Buffers;
	if (Acq)		delete Acq;
	if (AcqDevice)	delete AcqDevice;

		/*View->Create();
		Xfer->Grab();
		Xfer->Wait(5000);
		Xfer->Freeze();
		if (View) delete View;
		if (Xfer) delete Xfer;
		if (Buffers) delete Buffers;
		if (Acq) delete Acq;
		if (AcqDevice) delete AcqDevice;*/

	SapManager::Close();
    return 0;
}

static void AcqCallback(SapXferCallbackInfo *pInfo)
{
	SapView *pView = (SapView *)pInfo->GetContext();

	// Resfresh view
	pView->Show();
}


int start, end, duration, fps = 0;
BOOL firstFrame = TRUE;

/*
*  Callback function - StartOfFrame event
*  The function will record the timestamp(in ms) of the system when
*  each callback is received and then calculate an approximate FPS rate
*  from two consecutive StartOfframe events.
*  NOTE : The FPS is only an approximation due to delays in callbacks
*  and it should not be used as the actual FPS achieved by the system.
*/
static void StartFrameCallback(SapAcqCallbackInfo *pInfo)
{
	//for the first start of frame record the start time.
	if (firstFrame)
	{
		firstFrame = FALSE;
		start = GetTickCount();
		return;
	}
	end = GetTickCount();
	duration = end - start;
	start = end;

	//update FPS only if the value changed. 1000 is used because the duration is in ms.
	if (fps != 1000 / duration)
	{

		printf("Approximate FPS = %d \r", fps);
	}

}
