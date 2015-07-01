#include "Win32_OculusUtil.h"

void AddHmdCameraControl(ovrHmd hmd, Camera mainCam)
{
	// Keyboard inputs to adjust player orientation, unaffected by speed
	static auto Yaw = 3.141f;

	if (DIRECTX.Key[VK_LEFT])  mainCam.Rot = Matrix4f::RotationY(Yaw += 0.02f);
	if (DIRECTX.Key[VK_RIGHT]) mainCam.Rot = Matrix4f::RotationY(Yaw -= 0.02f);

	// Keyboard inputs to adjust player position
	if (DIRECTX.Key['Z'] || DIRECTX.Key[VK_UP])   mainCam.Pos += mainCam.Rot.Transform(Vector3f(0, 0, -0.05f));
	if (DIRECTX.Key['S'] || DIRECTX.Key[VK_DOWN]) mainCam.Pos += mainCam.Rot.Transform(Vector3f(0, 0, +0.05f));
	if (DIRECTX.Key['D'])                         mainCam.Pos += mainCam.Rot.Transform(Vector3f(+0.05f, 0, 0));
	if (DIRECTX.Key['Q'])                         mainCam.Pos += mainCam.Rot.Transform(Vector3f(-0.05f, 0, 0));
	mainCam.Pos.y = ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 0);
}
