#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <d3d11.h>
#include <d3dcompiler.h>	// We also need the D3DCompiler header to compile shaders
#include <DirectXMath.h>
#include <Windows.h>
#include <iostream>
#include <wrl.h>

#include "LiveObjectReporter.h"

namespace SE {

	namespace Graphics {

		/**
		*
		* @brief 
		* DeviceManager is responsible for managing and initializing the DirectX core components
		*
		* @details DeviceManager requires a window handle for its initialization, mainly for the swap chain
		*
		* @warning Error handling is taken care of internally from DeviceManager. If any of the core components fail to initialize, the application will be closed
		*
		*
		**/
		class DeviceManager {

		public:

			DeviceManager();
			~DeviceManager();

			HRESULT Init(HWND windowHandle);
			void Shutdown();

			/**
			* @brief	
			* CreateDeviceResources creates the graphic device and Graphic device context
			*
			* @details
			* CreateDeviceResources first specifies the feature level and setups the graphic device debug flag.
			* Finally, it calls upon D3D11CreateDevice to create both the graphic device and the graphic device context
			*
			* @retval return_value_n Returns a HRESULT indicating if the Graphic Device and Graphic Context was successfully created or not
			*
			* @warning Make sure to interpret the HRESULT returned from CreateDeviceResources and stop the application from proceeding if HRESULT is S_FALSE == FAILED(hr)
			*
			* Example code:
			* @code
			*	HRESULT hr = S_OK;
			*
			*	hr = CreateDeviceResources();
			*
			*	if (FAILED(hr)) {
			*
			*		Error handling
			*	}
			* @endcode
			*/
			HRESULT CreateDeviceResources();

			/**
			* @brief	
			* CreateSwapChain creates the swap chain
			*
			* @details
			* CreateSwapChain first defines a swap chain description
			* It proceeds to create an adapter that calls upon the parent factory
			* From the factory, a swap chain is created
			*
			* @param[in] windowHandle A window handle 
			*
			* @retval return_value_n Returns a HRESULT indicating if the swap chain was successfully created or not
			*
			* @warning Make sure to interpret the HRESULT returned from CreateSwapChain and stop the application from proceeding if HRESULT is S_FALSE == FAILED(hr)
			*
			* Example code:
			* @code
			*	HRESULT hr = S_OK;
			*
			*	hr = CreateSwapChain(HWND windowHandle);
			*
			*	if (FAILED(hr)) {
			*
			*		Error handling
			*	}
			* @endcode
			*/
			HRESULT CreateSwapChain(HWND windowHandle);

			/**
			* @brief	
			* CreateBackBufferRTV creates the back buffer render target view
			*
			* @details
			* CreateBackBufferRTV gets the Texture2D from the swap chain and puts in in gBackBuffer
			* The texture in gBackBuffer is then used to create the gBackBufferRTV
			*
			* @retval return_value_n Returns a HRESULT indicating if the render target view was successfully created or not
			*
			* @warning Make sure to interpret the HRESULT returned from CreateBackBufferRTV and stop the application from proceeding if HRESULT is S_FALSE == FAILED(hr)
			*
			* Example code:
			* @code
			*	HRESULT hr = S_OK;
			*
			*	hr = CreateBackBufferRTV();
			*
			*	if (FAILED(hr)) {
			*
			*		Error handling
			*	}
			* @endcode
			*/
			HRESULT CreateBackBufferRTV();

			/**
			* @brief	
			* CreateDepthStencil creates the default depth stencil view
			*
			* @details
			* CreateDepthStencil defines a texture 2D description and creates a texture from it
			* The texture is then used to create the depth stencil view
			*
			* @retval return_value_n Returns a HRESULT indicating if the depth stencil view was successfully created or not
			*
			* @warning Make sure to interpret the HRESULT returned from CreateDepthStencil and stop the application from proceeding if HRESULT is S_FALSE == FAILED(hr)
			*
			* Example code:
			* @code
			*	HRESULT hr = S_OK;
			*
			*	hr = CreateDepthStencil();
			*
			*	if (FAILED(hr)) {
			*
			*		Error handling
			*	}
			* @endcode
			*/
			HRESULT CreateDepthStencil();

			/**
			* @brief
			* SetViewport initializes the default viewport
			*
			* @details
			* From the back buffer texture, width and height is used to set the size of the viewport
			*
			*/
			void SetViewport();

			float GetAspectRatio();

			void Present();

			inline Microsoft::WRL::ComPtr<ID3D11Device>           GetDevice() { return gDevice; };
			inline Microsoft::WRL::ComPtr<ID3D11DeviceContext>    GetDeviceContext() { return gDeviceContext; };

		private:

			Microsoft::WRL::ComPtr<ID3D11Device>			gDevice;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext>		gDeviceContext;
			Microsoft::WRL::ComPtr<IDXGISwapChain>			gSwapChain;
			
			Microsoft::WRL::ComPtr<ID3D11Texture2D>			gBackBuffer;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	gBackbufferRTV;
			
			Microsoft::WRL::ComPtr<ID3D11Texture2D>			gDepthStencil;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	gDepthStencilView;

			D3D11_TEXTURE2D_DESC	gBB_Desc;
			D3D_FEATURE_LEVEL		gFeatureLevel;
			D3D11_VIEWPORT			gViewportDefault;

		};
	}
}

#endif 
