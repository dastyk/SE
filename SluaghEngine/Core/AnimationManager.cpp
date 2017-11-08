#include "AnimationManager.h"
#include <Profiler.h>
#include <Graphics\VertexStructs.h>

static const SE::Utilz::GUID SkinnedVertexShader("SkinnedVS.hlsl");
static const SE::Utilz::GUID SkinnedOncePerObject("SkinnedOncePerObject");


SE::Core::AnimationManager::AnimationManager(const IAnimationManager::InitializationInfo & initInfo) : initInfo(initInfo)
{
	_ASSERT(initInfo.renderer);
	_ASSERT(initInfo.resourceHandler);
	_ASSERT(initInfo.console);
	_ASSERT(initInfo.entityManager);
	_ASSERT(initInfo.eventManager);
	_ASSERT(initInfo.transformManager);

	initInfo.eventManager->RegisterToSetRenderObjectInfo({ this, &AnimationManager::CreateRenderObjectInfo });

	animationSystem = new AnimationSystem(initInfo.renderer);

	renderableManager = new RenderableManager({ initInfo.resourceHandler, initInfo.renderer,
		initInfo.console, initInfo.entityManager,
		initInfo.eventManager, initInfo.transformManager },
		10, animationSystem);

	auto result = initInfo.resourceHandler->LoadResource(SkinnedVertexShader, [this](auto guid, auto data, auto size) {
		auto result = this->initInfo.renderer->GetPipelineHandler()->CreateVertexShader(guid, data, size);
		if (result < 0)
			return ResourceHandler::InvokeReturn::FAIL;
		return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM;
	});
	if (result < 0)
		throw std::exception("Could not load SkinnedVertexShader.");


	Allocate(10);
}

SE::Core::AnimationManager::~AnimationManager()
{
	delete renderableManager;
	operator delete(animationData.data);
}

void SE::Core::AnimationManager::CreateAnimatedObject(const Entity & entity, const IAnimationManager::CreateInfo & info)
{
	StartProfile;
	auto& find = entityToIndex.find(entity);
	if (find != entityToIndex.end())
		ProfileReturnVoid;

	// Check if the entity is alive
	if (!initInfo.entityManager->Alive(entity))
		ProfileReturnVoid;


	// Make sure we have enough memory.
	if (animationData.used + 1 > animationData.allocated)
		Allocate(animationData.allocated * 2);

	// Register the entity
	auto index = animationData.used++;
	entityToIndex[entity] = index;
	animationData.entity[index] = entity;
	animationData.animInfo[index].nrOfLayers = 0;

	for(size_t j = 0; j < AnimationPlayInfo::maxLayers; j++){

	animationData.animInfo[index].timePos[j] = 0.0f;

	}

	renderableManager->CreateRenderableObject(entity, { info.mesh });
	

	// Load skeleton
	if (!animationSystem->IsSkeletonLoaded(info.skeleton))
	{

		auto result = initInfo.resourceHandler->LoadResource(info.skeleton, [this](auto guid, auto data, auto size) {
			auto result = LoadSkeleton(guid, data, size);
			if (result < 0)
				return ResourceHandler::InvokeReturn::FAIL;
			return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM;
		});
		if (result < 0)
		{
			initInfo.console->PrintChannel("Resources", "Could not load skeleton %u. Error: %d\n",  info.skeleton, result);
			animationData.used--;
			ProfileReturnVoid;
		}
	}

	animationData.animInfo[index].skeleton = info.skeleton;

	// Load animations
	for (size_t i = 0; i < info.animationCount; i++)
	{
		if (!animationSystem->IsAnimationLoaded(info.animations[i]))
		{
			auto result = initInfo.resourceHandler->LoadResource(info.animations[i], [this](auto guid, auto data, auto size) {
				auto result = LoadAnimation(guid, data, size);
				if (result < 0)
					return ResourceHandler::InvokeReturn::FAIL;
				return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM;
			});
			if (result < 0)
			{
				initInfo.console->PrintChannel("Resources", "Could not load animation %u. Error: %d",  info.animations[i], result);
				ProfileReturnVoid;
			}
		}

	}

	// Set Playing to false
	animationData.playing[index] = 0u;

	StopProfile;
}

void SE::Core::AnimationManager::Frame(Utilz::TimeCluster * timer)
{
	timer->Start(("AnimationManager"));
	
	renderableManager->Frame(nullptr);

	auto dt = initInfo.window->GetDelta();

	for (size_t i = 0; i < animationData.used; i++)
	{
		if (animationData.playing[i] == 1u)
		{
			auto& ai = animationData.animInfo[i];

			for(size_t j = 0; j < ai.nrOfLayers; j++){

				ai.timePos[j] += ai.animationSpeed[j] * dt;

			}

			animationSystem->CalculateMatrices(animationData.entity[i], ai);
		}
			
	}
	renderableManager->Frame(nullptr);
	GarbageCollection();
	timer->Stop(("AnimationManager"));
}

void SE::Core::AnimationManager::AttachToEntity(const Entity& source, const Entity& entityToAttach, const Utilz::GUID& jointGUID, int slotIndex) {

	// Assert the given slot index is larger than max slots
	_ASSERT(slotIndex < Attacher::maxSlots);

	// Find the source entity
	auto &sourceEntityIndex = entityToIndex.find(source);
	if (sourceEntityIndex != entityToIndex.end())
	{
		// Get animation info and attacher slots for the source entity
		auto& ai = animationData.animInfo[sourceEntityIndex->second];
		auto& at = animationData.attacher[sourceEntityIndex->second];

		// Check if the entity to attach is alive
		if(initInfo.entityManager->Alive(entityToAttach)){

			// If the entity to attach exists, check if the joint can be found in the source entity skeleton
			int found = animationSystem->FindJointIndex(ai.skeleton, jointGUID);
			if(found != -1){

				at.slots[slotIndex].entity = entityToAttach;
				at.slots[slotIndex].jointIndex = found;

			}

		}
	}
}

void SE::Core::AnimationManager::Start(const Entity & entity, const AnimationPlayInfo& playInfo)
{	
	StartProfile;

	_ASSERT(playInfo.nrOfLayers < AnimationPlayInfo::maxLayers);

	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		auto& ai = animationData.animInfo[entityIndex->second];
		ai.nrOfLayers = playInfo.nrOfLayers;

		for(size_t i = 0; i < playInfo.nrOfLayers; i++){

			if (animationSystem->IsAnimationLoaded(playInfo.animations[i]))
			{
				ai.animation[i] = playInfo.animations[i];
				ai.animationSpeed[i] = playInfo.animationSpeed[i];
				ai.looping[i] = playInfo.looping[i];
				ai.blendFactor[i] = playInfo.blendFactor[i];
				ai.blendSpeed[i] = playInfo.blendSpeed[i];
				ai.timePos[i] = playInfo.timePos[i];

				animationData.playing[entityIndex->second] = 1u;
			}

			else
				initInfo.console->PrintChannel("Resources", "Tried to start an unloaded animation. GUID: %u.", playInfo.animations[i]);
		}	

	}
	StopProfile;
}

void SE::Core::AnimationManager::SetSpeed(const Entity & entity, float speed)
{
	StartProfile;
	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		auto& ai = animationData.animInfo[entityIndex->second];

		for (size_t i = 0; i < ai.nrOfLayers; i++) {

			animationData.animInfo[entityIndex->second].animationSpeed[i] = speed;
		}
	}
	StopProfile;
}

void SE::Core::AnimationManager::SetBlendSpeed(const Entity& entity, int index, float speed) {

	StartProfile;

	auto dt = initInfo.window->GetDelta();

	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		auto& ai = animationData.animInfo[entityIndex->second];

		if (index == -1) {

			for (size_t i = 0; i < ai.nrOfLayers; i++) {

				ai.blendSpeed[i] = speed;
			}
		}

		else {

			if(index < AnimationInfo::maxLayers){

				ai.blendSpeed[index] = speed;

			}
		}

	}
	StopProfile;
}

void SE::Core::AnimationManager::SetKeyFrame(const Entity & entity, float keyFrame)
{
	StartProfile;
	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		auto& ai = animationData.animInfo[entityIndex->second];

		for (size_t i = 0; i < ai.nrOfLayers; i++) {

			ai.timePos[i] = keyFrame;
			animationData.playing[entityIndex->second] = 0u;
			animationSystem->CalculateMatrices(animationData.entity[entityIndex->second], ai);

		}
	}
	StopProfile;
}

void SE::Core::AnimationManager::Start(const Entity & entity, bool looping)const
{
	StartProfile;
	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		if (renderableManager->IsVisible(entity))
		{
			auto& ai = animationData.animInfo[entityIndex->second];

			for(size_t i = 0; i < ai.nrOfLayers; i++){

				if (animationSystem->IsAnimationLoaded(ai.animation[i]))
				{

					ai.looping[i] = looping;
					animationData.playing[entityIndex->second] = 1u;
					
				}

			}
		}
	}
	StopProfile;
}

void SE::Core::AnimationManager::Pause(const Entity & entity)const
{
	StartProfile;
	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		animationData.playing[entityIndex->second] = 0u;
	}
	StopProfile;
}

void SE::Core::AnimationManager::UpdateBlending(const Entity& entity, int index) {

	StartProfile;

	auto dt = initInfo.window->GetDelta();

	// Get the entity register from the animationManager
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		auto& ai = animationData.animInfo[entityIndex->second];

		if (index == -1) {

			for (size_t i = 0; i < ai.nrOfLayers; i++) {

				ai.blendFactor[i] += ai.blendSpeed[i] * dt;
				ai.blendFactor[index] = max(0.0f, min(ai.blendFactor[index], 1.0f));

			}
		}

		else {

			if (index < AnimationInfo::maxLayers) {
					
				ai.blendFactor[index] += ai.blendSpeed[index] * dt;
				ai.blendFactor[index] = max(0.0f, min(ai.blendFactor[index], 1.0f));

			}
		}

	}
	StopProfile;
}

bool SE::Core::AnimationManager::IsAnimationPlaying(const Entity& entity) const
{
	StartProfile;
	
	auto &entityIndex = entityToIndex.find(entity);
	if (entityIndex != entityToIndex.end())
	{
		ProfileReturnConst(animationData.playing[entityIndex->second]);
	}


	ProfileReturnConst(false);
}

void SE::Core::AnimationManager::ToggleVisible(const Entity & entity, bool visible)
{
	StartProfile;

	renderableManager->ToggleRenderableObject(entity, visible);
	
	StopProfile;
}

void SE::Core::AnimationManager::Allocate(size_t size)
{
	StartProfile;
	_ASSERT(size > animationData.allocated);

	// Allocate new memory
	AnimationData newData;
	newData.allocated = size;
	newData.data = operator new(size * AnimationData::size);
	newData.used = animationData.used;

	// Setup the new pointers
	newData.entity = (Entity*)newData.data;
	newData.animInfo = reinterpret_cast<AnimationInfo*>(newData.entity + size);
	newData.playing = (uint8_t*)(newData.animInfo + size);
	newData.attacher = (Attacher*)(newData.playing + size);
	
	// Copy data
	memcpy(newData.entity, animationData.entity, animationData.used * sizeof(Entity));
	memcpy(newData.animInfo, animationData.animInfo, animationData.used * sizeof(AnimationInfo));
	memcpy(newData.playing, animationData.playing, animationData.used * sizeof(uint8_t));
	memcpy(newData.attacher, animationData.attacher, animationData.used * sizeof(Attacher));


	// Delete old data;
	operator delete(animationData.data);
	animationData = newData;

	StopProfile;
}

void SE::Core::AnimationManager::Destroy(size_t index)
{
	StartProfile;

	renderableManager->Destroy(index);

	// Temp variables
	size_t last = animationData.used - 1;
	const Entity entity = animationData.entity[index];
	const Entity last_entity = animationData.entity[last];

	// Copy the data
	animationData.entity[index] = last_entity;
	animationData.animInfo[index] = animationData.animInfo[last];
	animationData.playing[index] = animationData.playing[last];

	// Replace the index for the last_entity 
	entityToIndex[last_entity] = index;
	entityToIndex.erase(entity);

	animationData.used--;
	StopProfile;
}

void SE::Core::AnimationManager::Destroy(const Entity & entity)
{
	const auto exists = entityToIndex.find(entity);
	if (exists != entityToIndex.end())
		Destroy(exists->second);
}

void SE::Core::AnimationManager::GarbageCollection()
{
	StartProfile;
	uint32_t alive_in_row = 0;
	while (animationData.used > 0 && alive_in_row < 4U)
	{
		std::uniform_int_distribution<uint32_t> distribution(0U, animationData.used - 1U);
		uint32_t i = distribution(generator);
		if (initInfo.entityManager->Alive(animationData.entity[i]))
		{
			alive_in_row++;
			continue;
		}
		alive_in_row = 0;
		Destroy(i);
	}
	StopProfile;
}

int SE::Core::AnimationManager::LoadSkeleton(const Utilz::GUID& guid, void * data, size_t size)
{
	auto skelH = (Skeleton_Header*)data;

	// After the skeleton header, there will only be joints
	auto jointAttr = (JointAttributes*)(skelH + 1);

	return animationSystem->AddSkeleton(guid, jointAttr, skelH->nrOfJoints);
}

int SE::Core::AnimationManager::LoadAnimation(const Utilz::GUID& guid, void * data, size_t size)
{
	auto animH = (Animation_Header*)data;

	// After the animation header comes the animated joint indices
	auto joints = (uint32_t*)(animH + 1);

	// After the joint indices comes the keyframe matrices
	auto matrices = (DirectX::XMFLOAT4X4*)(joints + animH->nrOfJoints);

	return animationSystem->AddAnimation(guid, matrices, joints, animH->animationLength, animH->nrOfJoints);

	//return initInfo.renderer->CreateAnimation(matrices, animH->animationLength, animH->nrOfJoints);
}

void SE::Core::AnimationManager::CreateRenderObjectInfo(const Entity& entity, Graphics::RenderJob * info)
{

	StartProfile;
	const auto fe = entityToIndex.find(entity);
	if (fe != entityToIndex.end())
	{
		info->pipeline.IAStage.inputLayout = SkinnedVertexShader;
		info->pipeline.VSStage.shader = SkinnedVertexShader;
		info->maxInstances = 8;
		info->specialHaxxor = SkinnedOncePerObject;
	}
	StopProfile;
}
