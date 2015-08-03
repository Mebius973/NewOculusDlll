#include "Win32_OculusMain.h"

/*
//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.


// get the app hinstance */
HWND      hWnd = NULL;
HINSTANCE hInstance;
/*
// Create the hmd */
ovrHmd	hmd;
/*
// Create the eye render texture set */
OculusTexture  * pEyeRenderTexture[2];

// Create the eye renderer
ovrRecti         eyeRenderViewport[2];
ovrEyeRenderDesc eyeRenderDesc[2];

// Set the default visibility
auto isVisible = false;
Sizei idealSize;

// This function is the main function automatically called by windows.
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
  // We manually set the size of the image, otherwise in some cases we end up with some weird sizes causing the Oculus to crash
  idealSize.w = 1180;
  idealSize.h = 1460;
  
  for (auto eye = 0; eye <2; eye++)
  {
    pEyeRenderTexture[eye]   = new OculusTexture(hmd, idealSize);
    eyeRenderViewport[eye].Pos  = Vector2i(0, 0);
    eyeRenderViewport[eye].Size = idealSize;
  }

  // Setup VR components, filling out description
  eyeRenderDesc[0] = ovrHmd_GetRenderDesc(hmd, ovrEye_Left, hmd->DefaultEyeFov[0]);
  eyeRenderDesc[1] = ovrHmd_GetRenderDesc(hmd, ovrEye_Right, hmd->DefaultEyeFov[1]);
  isVisible = true;
  return true;
}

void ProcessAndRender(char* leftEyeImage, char* rightEyeImage)
{
  // Get both eye poses simultaneously, with IPD offset already included. 
  // We actually need to keep this, otherwise the HMD orientates the image in a non desirable way.
  ovrPosef         EyeRenderPose[2];
  ovrVector3f      HmdToEyeViewOffset[2] = {eyeRenderDesc[0].HmdToEyeViewOffset, eyeRenderDesc[1].HmdToEyeViewOffset};
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

      // Update the textureset's D3D11 texture
      auto texIndex = pEyeRenderTexture[eye]->TextureSet->CurrentIndex;
      auto tex = reinterpret_cast<ovrD3D11Texture*>(&pEyeRenderTexture[eye]->TextureSet->Textures[texIndex]);
      DIRECTX.Context->UpdateSubresource(tex->D3D11.pTexture, 0, NULL, (eye == 0 ? leftEyeImage : rightEyeImage), idealSize.w * 4, idealSize.w * idealSize.h * 4);
    }
  }

  // Initialize our single full screen Fov layer.
  ovrLayerEyeFovDepth ld;
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
  isVisible = OVR_SUCCESS(result);
}

void Release()
{
  pEyeRenderTexture[0]->Release(hmd);
  pEyeRenderTexture[1]->Release(hmd);
  ovrHmd_Destroy(hmd);
  ovr_Shutdown();
  DIRECTX.ReleaseWindow(hInstance);
}

int ImageHeight()
{
  return pEyeRenderTexture[0]->Height ;
}

int ImageWidth()
{
  return pEyeRenderTexture[0]->Width * 2 ;
}
