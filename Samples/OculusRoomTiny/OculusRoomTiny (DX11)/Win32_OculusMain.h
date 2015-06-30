#include "Win32_OculusUtil.h"
// Include the Oculus SDK and define its version here
#define   OVR_D3D_VERSION 11
#include "OVR_CAPI_D3D.h"

/*
struct OculusTexture
{
	ovrSwapTextureSet      * TextureSet;
	ID3D11RenderTargetView * TexRtv[3];
	int Width;
	int Height;
	OculusTexture(ovrHmd hmd, Sizei size)
	{
		D3D11_TEXTURE2D_DESC dsDesc;
		dsDesc.Width            = size.w;
		dsDesc.Height           = size.h;
		Width					= size.w;
		Height					= size.h;
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
		for (auto i = 0; i < TextureSet->TextureCount; ++i)
		{
			auto tex = reinterpret_cast<ovrD3D11Texture*>(&TextureSet->Textures[i]);
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
};*/

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int);
bool Init();
void ProcessAndRender(char* data);
//void ProcessAndRender(char* leftEyeImage, char* rightEyeImage);
void Release();
