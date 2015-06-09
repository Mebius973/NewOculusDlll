/************************************************************************************
Filename    :   Win32_RoomTiny_Main.cpp
Content     :   First-person view test application for Oculus Rift
Created     :   18th Dec 2014
Authors     :   Tom Heath
Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*************************************************************************************/
/// This is an entry-level sample, showing a minimal VR sample, 
/// in a simple environment.  Use WASD keys to move around, and cursor keys.
/// Dismiss the health and safety warning by tapping the headset, 
/// or pressing any key. 
/// It runs with DirectX11.

// Include DirectX
#include "Win32_DirectXAppUtil.h" 

// Include the Oculus SDK
#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"
using namespace OVR;

// Needed global variables
ovrHmd hmd;

//------------------------------------------------------------
// ovrSwapTextureSet wrapper class that also maintains the render target views
// needed for D3D11 rendering.
struct OculusTexture
{
	ovrSwapTextureSet      * TextureSet;
	ID3D11RenderTargetView * TexRtv[3];

	OculusTexture(ovrHmd hmd, Sizei size)
	{
		D3D11_TEXTURE2D_DESC dsDesc;
		dsDesc.Width            = size.w;
		dsDesc.Height           = size.h;
		dsDesc.MipLevels        = 1;
		dsDesc.ArraySize        = 1;
		dsDesc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
		dsDesc.SampleDesc.Count = 1;   // No multi-sampling allowed
		dsDesc.SampleDesc.Quality = 0;
		dsDesc.Usage            = D3D11_USAGE_DEFAULT;
		dsDesc.CPUAccessFlags   = 0;
		dsDesc.MiscFlags        = 0;
		dsDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		ovrHmd_CreateSwapTextureSetD3D11(hmd, DIRECTX.Device, &dsDesc, &TextureSet);
		for (int i = 0; i < TextureSet->TextureCount; ++i)
		{
			ovrD3D11Texture* tex = (ovrD3D11Texture*)&TextureSet->Textures[i];
			DIRECTX.Device->CreateRenderTargetView(tex->D3D11.pTexture, NULL, &TexRtv[i]);
		}
	}

	void AdvanceToNextTexture()
	{
		TextureSet->CurrentIndex = (TextureSet->CurrentIndex + 1) % TextureSet->TextureCount;
	}
	void Release(ovrHmd hmd)
	{
		ovrHmd_DestroySwapTextureSet(hmd, TextureSet);
	}
};

// Even if it should be wiser to use the WinMain function, we use other functions in order to support legacy func interface.
// This is something that should be dropped.


bool Init()
{
	auto result = ovr_Initialize(nullptr);
	VALIDATE(result == ovrSuccess, "Failed to initialize libOVR.");
	result = ovrHmd_Create(0, &hmd);
	if (result != ovrSuccess) result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
	VALIDATE(result == ovrSuccess, "Oculus Rift not detected.");
	VALIDATE(hmd->ProductName[0] != '\0', "Rift detected, display not enabled.");

	return false;
}
void ProcessAndRender(char* data)
{
}
void Release(void)
{
}