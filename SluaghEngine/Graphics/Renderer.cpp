#include <Renderer.h>
#include <Profiler.h>

using namespace SE;

SE::Graphics::Renderer::Renderer()
{
}

SE::Graphics::Renderer::~Renderer()
{
}

int SE::Graphics::Renderer::Initialize(void * window)
{
	StartProfile;
	_ASSERT(window);
	device = new DeviceManager();
	HRESULT hr = device->Init((HWND)window);
	if (FAILED(hr))
		return -1;

	graphicResourceHandler = new GraphicResourceHandler(device->GetDevice(), device->GetDeviceContext());

	/*int shaderID[2];
	hr = graphicResourceHandler->CreateVertexShader(device->GetDevice(), &shaderID[0]);
	if (FAILED(hr))
		return -1;*/

	//hr = graphicResourceHandler->CreatePixelShader(device->GetDevice(), &shaderID[1]);
	//if (FAILED(hr))
	//	return -1;
	
	cam.SetPosition(0.0f, 1.0f, -2.0f);
	cam.Update(0.01f);
	TargetOffset off;
	off.shaderTarget[0] = true;
	off.shaderTarget[1] = true;
	off.shaderTarget[2] = true;
	off.offset[0] = 0;
	off.offset[1] = 0;
	off.offset[2] = 0;

	hr = graphicResourceHandler->CreateConstantBuffer(sizeof(OncePerFrameConstantBuffer), off, &oncePerFrameBufferID);
	if (FAILED(hr))
	{
		throw "omg";
	}

	graphicResourceHandler->BindConstantBuffer(oncePerFrameBufferID);

	graphicResourceHandler->CreateSamplerState();

	ProfileReturnConst( 0);
}

void SE::Graphics::Renderer::Shutdown()
{
	graphicResourceHandler->Shutdown();
	device->Shutdown();
	
	delete graphicResourceHandler;
	delete device;
}

int SE::Graphics::Renderer::EnableRendering(const RenderObjectInfo & handles)
{
	renderJobs.push_back(handles);

	int insertion = renderJobs.size() - 1;
	const int prior = insertion - 1;

	int minChange = 0;
	if (renderJobs.size() >= 2)
		minChange = handles - renderJobs[prior];

	for (int i = 0; i <= prior; i++)
	{
		int change = renderJobs[i] - handles;
		if (change <= minChange)
		{
			minChange = change;
			insertion = i;
		}
	}

	if (insertion != prior)
	{
		for (int i = prior + 1; i > insertion; i--)
		{
			renderJobs[i] = renderJobs[i - 1];
		}
		renderJobs[insertion] = handles;
	}

	return 0;
}

int SE::Graphics::Renderer::DisableRendering(const RenderObjectInfo & handles)
{
	return 0;
}

int SE::Graphics::Renderer::UpdateView(float * viewMatrix)
{
	return 0;
}

int SE::Graphics::Renderer::Render() {
	StartProfile;
	// clear the back buffer
	float clearColor[] = { 0, 0, 1, 1 };


	cam.Update(0.01f);

	ID3D11RenderTargetView* views[] = { device->GetRTV() };
	device->GetDeviceContext()->OMSetRenderTargets(1, views, device->GetDepthStencil());

	// Clear the primary render target view using the specified color
	device->GetDeviceContext()->ClearRenderTargetView(
	device->GetRTV(), 
	clearColor);

	// Clear the standard depth stencil view
	device->GetDeviceContext()->ClearDepthStencilView(
	device->GetDepthStencil(), 
	D3D11_CLEAR_DEPTH,  
	1.0f, 
	0);


	DirectX::XMFLOAT4X4 wo;

	DirectX::XMStoreFloat4x4(&wo, DirectX::XMMatrixTranspose(cam.ViewProj()));
	graphicResourceHandler->SetConstantBuffer(&wo, oncePerFrameBufferID);

	
	device->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	for (auto& job : renderJobs)
	{
		graphicResourceHandler->SetMaterial(job.vertexShader, job.pixelShader);
		graphicResourceHandler->SetVertexBuffer(job.bufferHandle);
		graphicResourceHandler->BindConstantBuffer(job.transformHandle);
		graphicResourceHandler->BindShaderResourceView(job.diffuseTexture, 0);
		device->GetDeviceContext()->Draw(graphicResourceHandler->GetVertexCount(job.bufferHandle), 0);
	}

	device->Present();

	ProfileReturnConst(0);
}

int SE::Graphics::Renderer::CreateVertexBuffer(void * data, size_t vertexCount, size_t stride)
{
	StartProfile;
	int handle = 0;
	auto result = graphicResourceHandler->CreateVertexBuffer(data, vertexCount, stride, &handle);
	if(result)
		ProfileReturnConst(result);
	ProfileReturnConst(handle);
}

void SE::Graphics::Renderer::DestroyVertexBuffer(int bufferHandle)
{
	StartProfile;
	graphicResourceHandler->RemoveVertexBuffer(bufferHandle);
	StopProfile;
}

int Graphics::Renderer::CreateTexture(void* data, const TextureDesc& description)
{
	StartProfile;
	ProfileReturn(graphicResourceHandler->CreateShaderResourceView(data, description))
}


int SE::Graphics::Renderer::CreateTransform()
{
	StartProfile;
	int handle;
	TargetOffset off;
	off.shaderTarget[0] = true;
	off.shaderTarget[1] = true;
	off.shaderTarget[2] = true;
	off.offset[0] = 1;
	off.offset[1] = 1;
	off.offset[2] = 1;
	auto hr = graphicResourceHandler->CreateConstantBuffer(sizeof(OncePerObjectConstantBuffer), off, &handle);
	if (FAILED(hr))
		ProfileReturnConst(hr);
	ProfileReturnConst(handle);
}

void SE::Graphics::Renderer::DestroyTransform(int transformHandle)
{
}

int SE::Graphics::Renderer::UpdateTransform(int transformHandle, float* transform)
{
	StartProfile;
	DirectX::XMMATRIX trans = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)transform);
	DirectX::XMFLOAT4X4 transposed;
	DirectX::XMStoreFloat4x4(&transposed, DirectX::XMMatrixTranspose(trans));
	graphicResourceHandler->SetConstantBuffer(&transposed, transformHandle);
	ProfileReturnConst(0);
}



int SE::Graphics::Renderer::CreatePixelShader(void * data, size_t size)
{
	int handle;
	auto hr = graphicResourceHandler->CreatePixelShader(device->GetDevice(),data, size, &handle);
	if (FAILED(hr))
		return hr;

	return handle;
}

int SE::Graphics::Renderer::CreateVertexShader(void * data, size_t size)
{
	int handle;
	auto hr = graphicResourceHandler->CreateVertexShader(device->GetDevice(),data, size, &handle);
	if (hr)
		return hr;
	return handle;
}

