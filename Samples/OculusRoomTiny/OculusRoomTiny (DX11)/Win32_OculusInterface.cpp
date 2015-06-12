// This file is intended to be a warpper which provide require interface called by SiVIC
// As it is designed to be a utility file for the main.cpp, we will deport some logic of the main here too
// Include main
#include "Win32_OculusMain.h"
#include "Win32_OculusInterface.h"

// Set of file-wide variables
extern HINSTANCE hInstance;
extern ovrHmd	hmd;
extern OculusTexture  * pEyeRenderTexture[2];

bool	initDone = false;
float positionOculus[6];

extern "C"
{
	__declspec(dllexport) bool birth()
	{
		if (initDone) {
			return true;
		}
		auto initSuccess = Init();
		if (initSuccess) {
			initDone = true;

			ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation |
				ovrTrackingCap_MagYawCorrection |
				ovrTrackingCap_Position, 0);
		}

		return initSuccess;
	}

	__declspec(dllexport) void process(char* data){
		ProcessAndRender(data);
	}

	__declspec(dllexport) void death(){
		Release();
	}

	// The API provides one eye texture size, to get the image full size, we need to calculate it
	__declspec(dllexport) int getImageHeight(){
		auto imgageHeight = pEyeRenderTexture[0]->Height + eyeGap + pEyeRenderTexture[1]->Height;
		return imgageHeight ;
	}

	__declspec(dllexport) int getImageWidth(){
		auto imgageWidth = pEyeRenderTexture[0]->Width + eyeGap + pEyeRenderTexture[1]->Width;
		return imgageWidth ;
	}

	__declspec(dllexport) float* getTracker(){
		auto ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());

		Posef pose = ts.HeadPose.ThePose;
		Quatf PoseOrientation = ts.HeadPose.ThePose.Orientation;
		PoseOrientation.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&positionOculus[2], &positionOculus[1], &positionOculus[0]);
		positionOculus[3] = -ts.HeadPose.ThePose.Position.z;
		positionOculus[4] = -ts.HeadPose.ThePose.Position.x;
		positionOculus[5] = ts.HeadPose.ThePose.Position.y;

		return positionOculus;
	}
}