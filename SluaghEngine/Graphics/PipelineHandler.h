#ifndef SE_GRAPHICS_PIPELINEHANDLER_H
#define SE_GRAPHICS_PIPELINEHANDLER_H
#include "IPipelineHandler.h"
#include <d3d11.h>
#include <unordered_map>
#include <unordered_set>
#include <Utilz/ThreadSafeMap.h>

namespace SE
{
	namespace Graphics
	{
		class PipelineHandler : public IPipelineHandler
		{
		public:
			PipelineHandler(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* backbuffer, ID3D11DepthStencilView* dsv);
			~PipelineHandler();
			PipelineHandler(const PipelineHandler& other) = delete;
			PipelineHandler(PipelineHandler&& other) = delete;
			PipelineHandler& operator=(const PipelineHandler& other) = delete;

			int AddExistingRenderTargetView(const Utilz::GUID& id, void* rtv) override;
			int AddExistingDepthStencilView(const Utilz::GUID& id, void* dsv) override;
			int AddExisitingShaderResourceView(const Utilz::GUID& id, void* srv) override;

			int MergeHandlers(IPipelineHandler* other) override;

			int CreateVertexShaderFromSource(const Utilz::GUID& id, const std::string& sourceCode, const std::string& entrypoint, const std::string& shaderModel) override;
			int CreateVertexBuffer(const Utilz::GUID& id, void* data, size_t vertexCount, size_t stride, bool dynamic = false) override;
			int UpdateDynamicVertexBuffer(const Utilz::GUID& id, void* data, size_t size) override;
			int CreateIndexBuffer(const Utilz::GUID& id, void* data, size_t indexCount, size_t indexSize) override;
			int CreateBuffer(const Utilz::GUID& id, void* data, size_t elementCount, size_t elementStride, size_t maxElements, uint32_t flags) override;
			int DestroyIndexBuffer(const Utilz::GUID& id) override;
			int DestroyVertexBuffer(const Utilz::GUID& id) override;

			int CreateViewport(const Utilz::GUID& id, const Viewport& viewport) override;

			int CreateVertexShader(const Utilz::GUID& id, void* data, size_t size) override;
			int CreateGeometryShader(const Utilz::GUID& id, void* data, size_t size) override;
			int CreateGeometryShaderStreamOut(const Utilz::GUID& id, void* data, size_t size) override;
			int CreatePixelShader(const Utilz::GUID& id, void* data, size_t size) override;
			int CreateComputeShader(const Utilz::GUID& id, void* data, size_t size) override;
			int DestroyVertexShader(const Utilz::GUID& id) override;
			int DestroyGeometryShader(const Utilz::GUID& id) override;
			int DestroyPixelShader(const Utilz::GUID& id) override;
			int DestroyComputeShader(const Utilz::GUID& id) override;

			int CreateConstantBuffer(const Utilz::GUID& id, size_t size, void* initialData = nullptr) override;
			int UpdateConstantBuffer(const Utilz::GUID& id, void* data, size_t size) override;
			int MapConstantBuffer(const Utilz::GUID& id, const std::function<void(void* mappedResource)>& mapCallback) override;
			int DestroyConstantBuffer(const Utilz::GUID& id) override;

			int CreateTexture(const Utilz::GUID& id, void* data, size_t width, size_t height) override;
			int DestroyTexture(const Utilz::GUID& id) override;

			int CreateRasterizerState(const Utilz::GUID& id, const RasterizerState& state) override;
			int DestroyRasterizerState(const Utilz::GUID& id) override;

			int CreateBlendState(const Utilz::GUID& id, const BlendState& state) override;
			int DestroyBlendState(const Utilz::GUID& id) override;

			int CreateDepthStencilState(const Utilz::GUID& id, const DepthStencilState& state) override;
			int DestroyDepthStencilState(const Utilz::GUID& id) override;

			int CreateSamplerState(const Utilz::GUID& id, const SamplerState& state) override;
			int DestroySamplerState(const Utilz::GUID& id) override;

			int CreateRenderTarget(const Utilz::GUID& id, const RenderTarget& target) override;
			int DestroyRenderTarget(const Utilz::GUID& id) override;

			int CreateDepthStencilView(const Utilz::GUID& id, size_t width, size_t height, bool bindAsTexture = false) override;
			int CreateDepthStencilViewCube(const Utilz::GUID& id, size_t width, size_t height, bool bindAsTexture = true) override;
			int DestroyDepthStencilView(const Utilz::GUID& id) override;

			int CreateUnorderedAccessView(const Utilz::GUID& id, const UnorderedAccessView& view)override;
			int DestroyUnorderedAccessView(const Utilz::GUID& id)override;

			void SetPipeline(const Pipeline& pipeline) override;
			void SetPipelineForced(const Pipeline& pipeline) override;

			void ClearAllRenderTargets() override;

		private:
			void SetInputAssemblerStage(const InputAssemblerStage& pIA);
			void SetVertexShaderStage(const ShaderStage& vss);
			void SetGeometryShaderStage(const ShaderStage& gss);
			void SetRasterizerStage(const RasterizerStage& rs);
			void SetPixelShaderStage(const ShaderStage& pss);
			void SetOutputMergerStage(const OutputMergerStage& oms);
			void SetComputeShaderStage(const ShaderStage& css);

			void ForcedSetInputAssemblerStage(const InputAssemblerStage& pIA);
			void ForcedSetVertexShaderStage(const ShaderStage& vss);
			void ForcedSetGeometryShaderStage(const ShaderStage& gss);
			void ForcedSetRasterizerStage(const RasterizerStage& rs);
			void ForcedSetPixelShaderStage(const ShaderStage& pss);
			void ForcedSetOutputMergerStage(const OutputMergerStage& oms);
			void ForcedSetComputeShaderStage(const ShaderStage& css);

			ID3D11Device* device;
			ID3D11DeviceContext* deviceContext;

			struct VertexBuffer
			{
				ID3D11Buffer* buffer;
				uint32_t stride;
			};
			struct IndexBuffer
			{
				ID3D11Buffer* buffer;
				uint32_t stride;
			};
			Pipeline currentPipeline;


			struct VertexShaderInfo
			{
				ID3D11VertexShader* shader;
				std::vector<Utilz::GUID> constantBuffers;		
			};
			struct GeomtryShaderInfo
			{
				ID3D11GeometryShader* shader;
				std::vector<Utilz::GUID> constantBuffers;
			};
			struct PixelShaderInfo
			{
				ID3D11PixelShader* shader;
				std::vector<Utilz::GUID> constantBuffers;
			};
			struct RenderTargetInfo
			{
				ID3D11RenderTargetView* rtv;
				float clearColor[4];
			};
			struct UnorderedAccessViewInfo
			{
				ID3D11UnorderedAccessView* uav;
				float clearColor[4];
			};
			struct ComputeShaderInfo
			{
				ID3D11ComputeShader* shader;
				std::vector<Utilz::GUID> constantBuffers;
			};
			
			std::unordered_set<Utilz::GUID, Utilz::GUID::Hasher> manuallyAddedResources;
			Utilz::ThreadSafeMap<Utilz::GUID, VertexBuffer, Utilz::GUID::Hasher> vertexBuffers;
			Utilz::ThreadSafeMap<Utilz::GUID, IndexBuffer, Utilz::GUID::Hasher> indexBuffers;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11InputLayout*, Utilz::GUID::Hasher> inputLayouts;
			Utilz::ThreadSafeMap<Utilz::GUID, VertexShaderInfo, Utilz::GUID::Hasher> vertexShaders;
			Utilz::ThreadSafeMap<Utilz::GUID, GeomtryShaderInfo, Utilz::GUID::Hasher> geometryShaders;
			Utilz::ThreadSafeMap<Utilz::GUID, PixelShaderInfo, Utilz::GUID::Hasher> pixelShaders;
			Utilz::ThreadSafeMap<Utilz::GUID, ComputeShaderInfo, Utilz::GUID::Hasher> computeShaders;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11Buffer*, Utilz::GUID::Hasher> constantBuffers;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11ShaderResourceView*, Utilz::GUID::Hasher> shaderResourceViews;
			Utilz::ThreadSafeMap<Utilz::GUID, RenderTargetInfo, Utilz::GUID::Hasher> renderTargetViews;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11DepthStencilView*, Utilz::GUID::Hasher> depthStencilViews;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11SamplerState*, Utilz::GUID::Hasher> samplerStates;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11BlendState*, Utilz::GUID::Hasher> blendStates;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11RasterizerState*, Utilz::GUID::Hasher> rasterizerStates;
			Utilz::ThreadSafeMap<Utilz::GUID, D3D11_VIEWPORT, Utilz::GUID::Hasher> viewports;
			Utilz::ThreadSafeMap<Utilz::GUID, ID3D11DepthStencilState*, Utilz::GUID::Hasher> depthStencilStates;
			Utilz::ThreadSafeMap<Utilz::GUID, UnorderedAccessViewInfo, Utilz::GUID::Hasher> unorderedAccessViews;

			/**<Key is evaluated by (GUID(shader) + GUID(resourceBindingName))*/
			Utilz::ThreadSafeMap<Utilz::GUID, int, Utilz::GUID::Hasher> shaderAndResourceNameToBindSlot;

			//std::mutex mapLock;
		};
	}
}


#endif
