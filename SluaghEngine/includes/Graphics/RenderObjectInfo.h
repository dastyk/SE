#ifndef SE_GRAPHICS_RENDEROBJECTINFO_H_
#define SE_GRAPHICS_RENDEROBJECTINFO_H_
#include <cstdint>
#include <bitset>

namespace SE
{
	namespace Graphics
	{
		struct RenderObjectInfo
		{
			int bufferHandle;
			int transformHandle;
			int pixelShader;
			int vertexShader;
			int diffuseTexture;
			/**
			* @brief Computes the difference between two RenderObjectInfo structs in terms of how many attributes are different. As such, a - b == b - a.
			* @details This operator is used to determine which render job to complete next, i.e. the render job that requires the least amount of state changes from the current one.
			* @param[in] rhs The other RenderObjectInfo to compare against
			* @retval Returns the number of differing attributes between the objects
			*/
			inline int operator-(const RenderObjectInfo& rhs) const
			{
				uint32_t stateChanges = 0;
				stateChanges = (stateChanges << 1) | (bufferHandle != rhs.bufferHandle);
				stateChanges = (stateChanges << 1) | (transformHandle != rhs.transformHandle);
				stateChanges = (stateChanges << 1) | (pixelShader != rhs.pixelShader);
				stateChanges = (stateChanges << 1) | (vertexShader != rhs.vertexShader);
				stateChanges = (stateChanges << 1) | (diffuseTexture != rhs.diffuseTexture);
				std::bitset<32> bits(stateChanges);
				return bits.count();
			}
		};
	}
}

#endif
