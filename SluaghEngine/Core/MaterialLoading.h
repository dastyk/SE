#ifndef SE_CORE_MATERIAL_LOADING_H_
#define SE_CORE_MATERIAL_LOADING_H_
#include <Utilz\GUID.h>
#include <Graphics\Pipeline.h>
#include <Graphics\FileHeaders.h>
#include <list>
#include <Entity.h>
#include <map>
#include <Graphics\IRenderer.h>
#include <ResourceHandler\IResourceHandler.h>

namespace SE
{
	namespace Core
	{
		struct TextureInfo
		{
			uint8_t numTextures;
			Utilz::GUID bindings[Graphics::ShaderStage::maxTextures];
			Utilz::GUID textures[Graphics::ShaderStage::maxTextures];
		};

		struct MaterialFileData
		{
			Graphics::MaterialAttributes attrib;
			TextureInfo textureInfo;
			std::list<Entity> entities;
		};
		struct TextureData
		{
			std::list<Entity> entities;
		};
		struct ShaderData
		{
			size_t refCount;
		};

		class MaterialLoading
		{
		public:
			MaterialLoading(Graphics::IRenderer* renderer,
			ResourceHandler::IResourceHandler* resourceHandler);
			~MaterialLoading();





			int LoadShader(const Utilz::GUID& guid);
			bool IsShaderLoaded(const Utilz::GUID& guid)const;

			int LoadMaterialFile(const Utilz::GUID& guid, MaterialFileData& matInfo);
			bool IsMaterialFileLoaded(const Utilz::GUID& guid)const;

			int LoadShaderAndMaterialFileAndTextures(const Utilz::GUID& shader, const Utilz::GUID& materialFile, bool async, ResourceHandler::Behavior behavior);
			int LoadShaderAndTextures(const Utilz::GUID& shader, const Utilz::GUID& materialFile, bool async, ResourceHandler::Behavior behavior);
			int LoadMaterialFileAndTextures(const Utilz::GUID& materialFile, bool async, ResourceHandler::Behavior behavior);


			int LoadTextures(MaterialFileData& material, bool async, ResourceHandler::Behavior behavior);
			int LoadTexture(const Utilz::GUID& guid);


			

		private:
			int LoadTexture(const Utilz::GUID& guid, void*data, size_t size)const;


			Graphics::IRenderer* renderer;
			ResourceHandler::IResourceHandler* resourceHandler;

			std::map<Utilz::GUID, ShaderData, Utilz::GUID::Compare> guidToShader;
			std::map<Utilz::GUID, TextureData, Utilz::GUID::Compare> guidToTexture;
			std::map<Utilz::GUID, MaterialFileData, Utilz::GUID::Compare> guidToMaterial;
		};
	}
}


#endif //SE_CORE_MATERIAL_LOADING_H_