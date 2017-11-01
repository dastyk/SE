#ifndef SE_CORE_MATERIAL_MANAGER_H_
#define SE_CORE_MATERIAL_MANAGER_H_


#include <IMaterialManager.h>

#include <unordered_map>
#include <map>
#include <random>
#include <vector>
#include <Utilz\CircularFiFo.h>
#include "MaterialLoading.h"

namespace SE
{
	namespace Core
	{
		class MaterialManager : public IMaterialManager
		{
		public:
			MaterialManager(const IMaterialManager::InitializationInfo& initInfo);
			~MaterialManager();
			MaterialManager(const MaterialManager& other) = delete;
			MaterialManager(const MaterialManager&& other) = delete;
			MaterialManager& operator=(const MaterialManager& other) = delete;

			/**
			* @brief	Create a texture for the entity. This is for mesh with no submeshes.
			* @param [in] entity The entity to bind the texture to.
			* @param [in] info The info used when creating the material.
			* @param [in] async If the resource should be streamed.
			* @param [in] behavior The streaming behavior.
			* @sa CreateInfo
			*/
			void Create(const Entity& entity, const CreateInfo& info, bool async = false, ResourceHandler::Behavior behavior = ResourceHandler::Behavior::QUICK)override;


			/**
			* @brief	Called each frame, to update the state.
			*/
			void Frame(Utilz::TimeCluster* timer)override;
		private:
			/**
			* @brief	Allocate more memory
			*/
			void Allocate(size_t size);
			/**
			* @brief	Remove an enitity entry
			*/
			void Destroy(size_t index)override;
			/**
			* @brief	Remove an enitity entry
			*/
			void Destroy(const Entity& entity)override;
			/**
			* @brief	Look for dead entities.
			*/
			void GarbageCollection()override;

			void SetRenderObjectInfo(const Entity& entity, Graphics::RenderJob* info);

		

			std::map<Utilz::GUID, ShaderData, Utilz::GUID::Compare> guidToShader;
			std::map<Utilz::GUID, TextureData, Utilz::GUID::Compare> guidToTexture;
			std::map<Utilz::GUID, MaterialFileData, Utilz::GUID::Compare> guidToMaterial;


			MaterialLoading mLoading;
		

			struct MaterialData
			{
				static const size_t size = sizeof(Entity) + sizeof(Utilz::GUID) + sizeof(Utilz::GUID) + sizeof(uint8_t);
				size_t allocated = 0;
				size_t used = 0;
				void* data = nullptr;
				Entity* entity;
				Utilz::GUID* shader;
				Utilz::GUID* material;
				uint8_t* bloom;
			};
			
			InitializationInfo initInfo;
			std::default_random_engine generator;

			MaterialData materialInfo;
			std::unordered_map<Entity, size_t, EntityHasher> entityToMaterialInfo;

			Utilz::GUID defaultPixelShader;
			Utilz::GUID defaultTextureBinding;
			Utilz::GUID defaultSampler;
			Utilz::GUID defaultMaterial;

			struct toUpdateStruct
			{
				Utilz::GUID material;
			};
			Utilz::CircularFiFo<toUpdateStruct, 10> toUpdate;
		};
	}
}


#endif //SE_CORE_MATERIAL_MANAGER_H_