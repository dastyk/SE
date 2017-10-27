#ifndef SE_CORE_ANIMATION_MANAGER_H_
#define SE_CORE_ANIMATION_MANAGER_H_

#include <IAnimationManager.h>
#include <unordered_map>
#include <Graphics\FileHeaders.h>
#include <list>
#include <random>
#include <Utilz\CircularFiFo.h>

#include "AnimationSystem.h"

namespace SE
{
	namespace Core
	{
		enum class AnimationTrigger : uint32_t
		{
			Move
		};


		class AnimationManager : public IAnimationManager
		{
		public:
			AnimationManager(const InitializationInfo& initInfo);
			~AnimationManager();

			void CreateAnimation(const Entity& entity, const CreateInfo& info)override;

			/**
			* @brief	Called each frame, to update the state.
			*/
			void Frame(Utilz::TimeCluster* timer)override;

			void Start(const Entity& entity, const Utilz::GUID& animation, float speed)override;
			void SetSpeed(const Entity& entity, float speed)override;
			void SetKeyFrame(const Entity& entity, float keyFrame)override;
			void Start(const Entity& entity)const override;
			void Pause(const Entity& entity)const override;
			
			void ToggleVisible(const Entity& entity, bool visible)override;

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
			* @brief	Remove an enitity
			*/
			void Destroy(const Entity& entity)override;
			/**
			* @brief	Look for dead entities.
			*/
			void GarbageCollection()override;

			int LoadSkeleton(const Utilz::GUID& guid, void*data, size_t size);
			int LoadAnimation(const Utilz::GUID& guid, void * data, size_t size);
	

			InitializationInfo initInfo;
			std::default_random_engine generator;

			struct AnimationData
			{
				static const size_t size = sizeof(Entity) + sizeof(Utilz::GUID)*2;
				size_t allocated = 0;
				size_t used = 0;
				void* data = nullptr;
				Entity* entity;
				Utilz::GUID* mesh;
				Utilz::GUID* skeleton;
			};
			
			AnimationData animationData;
			std::unordered_map <Entity, size_t, EntityHasher> entityToIndex;



			AnimationSystem animationSystem;


			struct BufferInfo
			{
				int vertexCount;
			};

			int LoadModel(const Utilz::GUID& meshGUID, void* data, size_t size, int& vertexCount);
			std::unordered_map<Utilz::GUID, BufferInfo, Utilz::GUID::Hasher> guidToBufferInfo;

		};
	}
}
#endif