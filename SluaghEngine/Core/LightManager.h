#pragma once
#ifndef SE_CORE_LIGHT_MANAGER_H_
#define SE_CORE_LIGHT_MANAGER_H_

#include <ILightManager.h>
#include <unordered_map>
#include <stack>
#include <vector>
#include <random>
#include <Graphics\LightInfo.h>



namespace SE
{
	namespace Core
	{
		class LightManager : public ILightManager
		{
		public:
			LightManager(const InitializationInfo& initInfo);
			~LightManager();		

			void Create(const Entity& entity, const CreateInfo& data)override;

			void ToggleLight(const Entity& entity, bool show)override;

			void Frame(Utilz::TimeCluster* timer)override;
		private:
			void GarbageCollection()override;
			void Destroy(size_t index)override;
			void Destroy(const Entity& entity)override;
			void UpdateDirtyPos(const Entity& entity, size_t index);
			struct DirtyEntityInfo
			{
				size_t transformIndex;
				Entity entity;
			};
			std::vector<DirtyEntityInfo> dirtyEntites;

			struct LightData
			{
				DirectX::XMFLOAT4 colour;	//colour (rgba)
				DirectX::XMFLOAT4 pos;	//pos (pos + range)
				bool visible;
			};

			
			struct LightAttributes
			{
				DirectX::XMFLOAT4 colour;	//colour (rgba)
				DirectX::XMFLOAT4 pos;	//pos (pos + range)
			};
			struct LightDataBuffer
			{
				uint32_t size[4];
				LightAttributes data[20];
			};

			// Light variables
			std::unordered_map<Entity, LightData, EntityHasher> entityToLightData;
			std::vector<Entity> indexToEntity;

			std::default_random_engine generator;

			InitializationInfo initInfo;

			bool anyTogglesThisFrame = false;
		};
	}
}
#endif