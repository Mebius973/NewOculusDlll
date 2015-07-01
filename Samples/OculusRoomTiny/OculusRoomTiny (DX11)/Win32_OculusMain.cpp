#include "Win32_OculusMain.h"

/*
//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.


// get the app hinstance */
HINSTANCE hInstance;
/*
// Create the hmd */
ovrHmd	hmd;
/*
// Create the eye render texture set */
OculusTexture  * pEyeRenderTexture[2];

// required global variables
// Create the camera
Camera mainCam;

// Create the room model
// The following statement causes the dll to not be loaded
// Scene roomScene;
// Create the eye renderer
DepthBuffer    * pEyeDepthBuffer[2];
ovrRecti         eyeRenderViewport[2];
ovrEyeRenderDesc eyeRenderDesc[2];
// Mirror image to display on the computer
ovrTexture*          mirrorTexture;
// Set the default visibility
auto isVisible = true;

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
	hInstance = hinst;
	return 0;
}

// Even if it should be wiser to use the WinMain function, we use other functions in order to support legacy func interface.
// This is the reason why WinMain is almost empty
// This is something that should be dropped.

bool Init()
{
	// Initializes LibOVR, and the Rift
	auto result = ovr_Initialize(nullptr);
	VALIDATE(result == ovrSuccess, "Failed to initialize libOVR.");
	result = ovrHmd_Create(0, &hmd);
	if (result != ovrSuccess) result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
	VALIDATE(result == ovrSuccess, "Oculus Rift not detected.");
	VALIDATE(hmd->ProductName[0] != '\0', "Rift detected, display not enabled.");

	// Setup Window and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	ovrSizei winSize = { hmd->Resolution.w / 2, hmd->Resolution.h / 2 };
	auto initialized = DIRECTX.InitWindowAndDevice(hInstance, Recti(Vector2i(0), winSize), L"Oculus Room Tiny (DX11)");
	VALIDATE(initialized, "Unable to initialize window and D3D11 device.");

	// Set HMD capabilities
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

	// Start the sensor which informs of the Rift's pose and motion
	VALIDATE( ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0) == ovrSuccess, "Failed to configure tracking.");

	// Make the eye render buffers (caution if actual size < requested due to HW limits).
	DepthBuffer    * pEyeDepthBuffer[2];
	ovrRecti         eyeRenderViewport[2];

	for (auto eye = 0; eye < 2; eye++)
	{
		Sizei idealSize = ovrHmd_GetFovTextureSize(hmd, static_cast<ovrEyeType>(eye), hmd->DefaultEyeFov[eye], 1.0f);
		pEyeRenderTexture[eye]      = new OculusTexture(hmd, idealSize);
		pEyeDepthBuffer[eye]        = new DepthBuffer(DIRECTX.Device, idealSize);
		eyeRenderViewport[eye].Pos  = Vector2i(0, 0);
		eyeRenderViewport[eye].Size = idealSize;
	}

	// Create a mirror to see on the monitor.
	mirrorTexture = nullptr;
	D3D11_TEXTURE2D_DESC td = { };
	td.ArraySize        = 1;
	td.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.Width            = DIRECTX.WinSize.w;
	td.Height           = DIRECTX.WinSize.h;
	td.Usage            = D3D11_USAGE_DEFAULT;
	td.SampleDesc.Count = 1;
	td.MipLevels        = 1;
	ovrHmd_CreateMirrorTextureD3D11(hmd, DIRECTX.Device, &td, &mirrorTexture);

	// Update camera
	mainCam = Camera(Vector3f(0.0f, 1.6f, -5.0f), Matrix4f::RotationY(3.141f));

	// Setup VR components, filling out description
	eyeRenderDesc[0] = ovrHmd_GetRenderDesc(hmd, ovrEye_Left, hmd->DefaultEyeFov[0]);
	eyeRenderDesc[1] = ovrHmd_GetRenderDesc(hmd, ovrEye_Right, hmd->DefaultEyeFov[1]);

	return true;
}

//void ProcessAndRender(char* leftEyeImage, char* rightEyeImage)
void ProcessAndRender(char* data)
{
	while (DIRECTX.HandleMessages())
	{
		AddHmdCameraControl(hmd, mainCam);

		// Animate the cube
		static float cubeClock = 0;
		//roomScene.Models[0]->Pos = Vector3f(9 * sin(cubeClock), 3, 9 * cos(cubeClock+=0.015f));

		// Get both eye poses simultaneously, with IPD offset already included.
		ovrPosef         EyeRenderPose[2];
		ovrVector3f      HmdToEyeViewOffset[2] = { eyeRenderDesc[0].HmdToEyeViewOffset,
			eyeRenderDesc[1].HmdToEyeViewOffset };
		auto ftiming  = ovrHmd_GetFrameTiming(hmd, 0);
		auto hmdState = ovrHmd_GetTrackingState(hmd, ftiming.DisplayMidpointSeconds);
		ovr_CalcEyePoses(hmdState.HeadPose.ThePose, HmdToEyeViewOffset, EyeRenderPose);

		if (isVisible)
		{
			// Render Scene to Eye Buffers
			for (auto eye = 0; eye < 2; eye++)
			{
				// Increment to use next texture, just before writing
				pEyeRenderTexture[eye]->AdvanceToNextTexture();

				// Clear and set up rendertarget
				auto texIndex = pEyeRenderTexture[eye]->TextureSet->CurrentIndex;
				DIRECTX.SetAndClearRenderTarget(pEyeRenderTexture[eye]->TexRtv[texIndex], pEyeDepthBuffer[eye]);
				DIRECTX.SetViewport(Recti(eyeRenderViewport[eye]));

				// Get view and projection matrices for the Rift camera
				Camera finalCam(mainCam.Pos + mainCam.Rot.Transform(EyeRenderPose[eye].Position),
					mainCam.Rot * Matrix4f(EyeRenderPose[eye].Orientation));
				auto view = finalCam.GetViewMatrix();
				Matrix4f proj = ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.2f, 1000.0f, ovrProjection_RightHanded);

				// Render the scene
				//roomScene.Render(proj*view, 1, 1, 1, 1, true);
			}
		}

		// Initialize our single full screen Fov layer.
		ovrLayerEyeFov ld;
		ld.Header.Type  = ovrLayerType_EyeFov;
		ld.Header.Flags = 0;

		for (auto eye = 0; eye < 2; eye++)
		{
			ld.ColorTexture[eye] = pEyeRenderTexture[eye]->TextureSet;
			ld.Viewport[eye]     = eyeRenderViewport[eye];
			ld.Fov[eye]          = hmd->DefaultEyeFov[eye];
			ld.RenderPose[eye]   = EyeRenderPose[eye];
		}

		auto layers = &ld.Header;
		auto result = ovrHmd_SubmitFrame(hmd, 0, nullptr, &layers, 1);
		isVisible = result == ovrSuccess;

		// Render mirror
		auto tex = reinterpret_cast<ovrD3D11Texture*>(mirrorTexture);
		DIRECTX.Context->CopyResource(DIRECTX.BackBuffer, tex->D3D11.pTexture);
		DIRECTX.SwapChain->Present(0, 0);
	}
}

void Release()
{
	ovrHmd_DestroyMirrorTexture(hmd, mirrorTexture);
	pEyeRenderTexture[0]->Release(hmd);
	pEyeRenderTexture[1]->Release(hmd);
	ovrHmd_Destroy(hmd);
	ovr_Shutdown();
	DIRECTX.ReleaseWindow(hInstance);
}
