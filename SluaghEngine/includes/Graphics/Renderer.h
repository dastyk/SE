#ifndef SE_GRAPHICS_RENDERER_H_
#define SE_GRAPHICS_RENDERER_H_
#include "IRenderer.h"
#include "DeviceManager.h"
#include "GraphicResourceHandler.h"

namespace SE
{
	namespace Graphics
	{

		class Renderer : public IRenderer
		{
		public:
			Renderer();
			~Renderer();

			/**
			* @brief Initialize the renderer
			* @param[in] window A pointer to the window.
			* @retval 0 On success.
			* @endcode
			*/
			int Initialize(void* window) override;

			/**
			* @brief Shutdown the renderer
			* @endcode
			*/
			void Shutdown() override;

			/**
			* @brief    Sets a render job
			* @param[in] handles The handles struct
			* @retval Returns a handle to the job on success.
			* @retval -1 on failure.
			* @sa RenderObjectInfo
			*/
			int EnableRendering(const RenderObjectInfo& handles) override;

			/**
			* @brief    Removes a render job.
			* @param[in] handles The handles struct
			* @retval 0 On success.
			* @endcode
			*/
			int DisableRendering(const RenderObjectInfo& handles) override;

			/**
			* @brief Updates the view matrix used for rendering
			* @param[in] viewMatrix The view matrix to use, an array of 16 floats in row major format.
			* @retval return_value_0 Returns 0 on success.
			* @endcode
			*/
			int UpdateView(float* viewMatrix) override;

			/**
			* @brief Renders the scene
			* @retval return_value_0 Returns 0 On success.
			* @endcode
			*/
			int Render() override;


			/**
			* @brief Creates a vertex buffer.
			* @param[in] data The vertex data.
			* @param[in] vertexCount Number of vertices
			* @param[in] stride The size of one vertex
			* @retval BufferHandle Returns a handle to the created buffer.
			* @retval -1 If something went wrong
			* @endcode
			*/
			int CreateVertexBuffer(void*data, size_t vertexCount, size_t stride) override;

			/**
			* @brief Destroys a buffer.
			* @param[in] bufferHandle The handle to the buffer to destroy
			* @endcode
			*/
			void DestroyVertexBuffer(int bufferHandle) override;
			
			/**
			* @brief Creates a texture (SRV)
			* @param[in] data Raw image data
			* @param[in] description Texture description
			* @sa TextureDesc
			*
			*/
			int CreateTexture(void* data, const TextureDesc& description) override;

			/**
			* @brief Create a transform.
			* @param[in] transform Initial transform.
			* @retval transformHandle Returns a handle to the created transform.
			* @retval -1 If something went wrong
			* @endcode
			*/
			int CreateTransform() override;
			/**
			* @brief Destroy a transform
			* @param[in] transformHandle Handle to the transform to destroy.
			* @endcode
			*/
			void DestroyTransform(int transformHandle) override;
			/**
			* @brief Updates the transformation for an entity that is bound to rendering.
			* @param[in] transformHandle The transform handle that is bound to the renderable object.
			* @param[in] transform The transfrom to apply to the renderable object, an array of 16 floats in row major format.
			* @retval 0 On success.
			* @endcode
			*/
			int UpdateTransform(int transformHandle, float* transform) override;



			/**
			* @brief Create a pixel shader from raw data
			* @param[in] data A pointer to shader blob.
			* @param[in] size The size of the shader blob.
			* @param[out] reflection Stores information from a shader reflection at the address pointed to by reflection if not nullptr.
			* @retval handle On success.
			* @retval -1 Something went wrong.
			* @endcode
			*/
			int CreatePixelShader(void* data, size_t size, ShaderSettings* reflection = nullptr) override;

			/**
			* @brief Create a vertex buffer with CPU write access
			* @param[in] bytewidth The size of the buffer
			* @param[in] initialData The data to create the buffer with
			* @param[in] vertexByteSize The size of the vertex type in bytes
			* @param[in] initialDataSize The size in bytes of the initial data
			* @retval handle On success.
			* @retval -1 Something went wrong.
			*/
			int CreateDynamicVertexBuffer(size_t bytewidth, size_t vertexByteSize, void* initialData = nullptr, size_t initialDataSize = 0) override;

			/**
			* @brief Updates a dynamic vertex buffer. Replaces the existing data.
			* @param[in] handle The handle of the vertex buffer to update
			* @param[in] data The data to put in the vertex buffer
			* @param[in] totalSize The total size in bytes of the data to put in the buffer
			* @param[in] sizePerElement The size per vertex in bytes
			* @retval 0 On success.
			* @retval -1 Something went wrong.
			*/
			int UpdateDynamicVertexBuffer(int handle, void* data, size_t totalSize, size_t sizePerElement) override;

			/**
			* @brief Create a vertex shader from raw data
			* @param[in] data A pointer to shader blob.
			* @param[in] size The size of the shader blob.
			* @retval handle On success.
			* @retval -1 Something went wrong.
			* @endcode
			*/
			int CreateVertexShader(void* data, size_t size) override;
			/**
			* @brief Resizes the swapchain
			* @param[in] windowHandle A window handle.
			* @endcode
			*/
			void ResizeSwapChain(void* windowHandle) override;
		private:
			Renderer(const Renderer& other) = delete;
			Renderer(const Renderer&& other) = delete;
			Renderer& operator=(const Renderer& other) = delete;


			struct OncePerFrameConstantBuffer
			{
				DirectX::XMFLOAT4X4 viewproj;
			};

			int oncePerFrameBufferID;

			DeviceManager* device;

			GraphicResourceHandler* graphicResourceHandler;

			std::vector<RenderObjectInfo> renderJobs;

			struct RenderBucket
			{
				RenderObjectInfo stateInfo;
				size_t constantBufferHandle;
				std::vector<DirectX::XMFLOAT4X4> transforms;
			};
			std::vector<RenderBucket> renderBuckets;
			struct BucketAndTransformIndex
			{
				uint32_t bucketIndex;
				uint32_t transformIndex;
			};
			std::map<uint32_t, BucketAndTransformIndex> jobIDToBucketAndTransformIndex;
		};

	}
}
#endif //SE_GRAPHICS_RENDERER_H_