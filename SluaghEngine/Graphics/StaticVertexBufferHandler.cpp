#include "StaticVertexBufferHandler.h"

namespace SE
{
	namespace Graphics
	{
		StaticVertexBufferHandler::StaticVertexBufferHandler()
		{

		}

		StaticVertexBufferHandler::~StaticVertexBufferHandler()
		{

		}

		StaticVertexBufferHandler::StaticVertexBufferHandler(ID3D11Device* inDevice, ID3D11DeviceContext* inDeviceContext)
		{
			device = inDevice;
			deviceContext = inDeviceContext;
		}

		HRESULT StaticVertexBufferHandler::CreateVertexBuffer(void* inputData, int inputSize, int *vertexBufferID)
		{
			D3D11_BUFFER_DESC _vertexBufferDesc;
			// description for vertexbuffer
			_vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			_vertexBufferDesc.ByteWidth = inputSize;
			_vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			_vertexBufferDesc.CPUAccessFlags = 0;
			_vertexBufferDesc.MiscFlags = 0;
			_vertexBufferDesc.StructureByteStride = 0;

			D3D11_SUBRESOURCE_DATA _vertexData;
			_vertexData.pSysMem = inputData;
			_vertexData.SysMemPitch = 0;
			_vertexData.SysMemSlicePitch = 0;

			
			// Creates vertex buffern
			ID3D11Buffer* tempBuffer;
			HRESULT _hr = device->CreateBuffer(&_vertexBufferDesc, &_vertexData, &tempBuffer);

			if (_hr != S_OK)
			{
				return _hr;
			}
			if (stackID.size() == 0)
			{
				vertexBuffer.push_back(tempBuffer);
				*vertexBufferID = vertexBuffer.size() - 1;
			}
			else
			{
				vertexBuffer.at(stackID.top()) = tempBuffer;
				*vertexBufferID = stackID.top();
				stackID.pop();
			}
			return _hr;
		}

		void StaticVertexBufferHandler::SetVertexBuffer(int vertexBufferID)
		{
			UINT32 vertexSize = sizeof(float) * 8;
			UINT32 offset = 0;
			deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.at(vertexBufferID), &vertexSize, &offset);
		}

		void StaticVertexBufferHandler::RemoveVertexBuffer(int vertexBufferID)
		{
			vertexBuffer.at(vertexBufferID)->Release();
			stackID.push(vertexBufferID);
		}
	}	//namespace Graphics
}	//namespace SE