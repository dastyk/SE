#include "TransformManager.h"

using namespace DirectX;

SE::Core::TransformManager::TransformManager(EntityManager* em)
{
	entityManager = em;

	transformCapacity = 512;
	transformCount = 0;
	const size_t bytes = transformCapacity * sizePerEntity;
	positions = new XMFLOAT3[transformCapacity];
	rotations = new XMFLOAT3[transformCapacity];
	scalings = new float[transformCapacity];


}

SE::Core::TransformManager::~TransformManager()
{
	delete[] positions;
	delete[] rotations;
	delete[] scalings;
}

void SE::Core::TransformManager::Create(const Entity& e, const DirectX::XMFLOAT3& pos,
	const DirectX::XMFLOAT3& rotation, float scale)
{
	_ASSERT_EXPR(entityToIndex.find(e) == entityToIndex.end(), "Entity created twice in transformmanager");
	if (transformCount == transformCapacity)
		ExpandTransforms();

	const uint32_t index = transformCount++;
	entityToIndex[e] = index;
	positions[index] = pos;
	rotations[index] = rotation;
	scalings[index] = scale;
}

void SE::Core::TransformManager::Move(const Entity& e, const DirectX::XMFLOAT3& dir)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	positions[index].x += dir.x;
	positions[index].y += dir.y;
	positions[index].z += dir.z;
}

void SE::Core::TransformManager::Rotate(const Entity& e, float roll, float pitch, float yaw)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	rotations[index].x += roll;
	rotations[index].y += pitch;
	rotations[index].z += yaw;
}

void SE::Core::TransformManager::Scale(const Entity& e, float scale)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	scalings[index] *= scale;
}

void SE::Core::TransformManager::SetPosition(const Entity& e, const DirectX::XMFLOAT3& pos)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	positions[index] = pos;
}

void SE::Core::TransformManager::SetRotation(const Entity& e, float roll, float pitch, float yaw)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	rotations[index] = { roll, pitch, yaw };
}

void SE::Core::TransformManager::SetScale(const Entity& e, float scale)
{
	_ASSERT_EXPR(entityToIndex.find(e) != entityToIndex.end(), "Undefined entity referenced in transform manager");
	const uint32_t index = entityToIndex[e];
	scalings[index] = scale;
}

DirectX::XMFLOAT3 SE::Core::TransformManager::GetPosition(const Entity& e) const
{
	auto entry = entityToIndex.find(e);
	_ASSERT_EXPR(entry != entityToIndex.end(), "Undefined entity referenced in transform manager");
	return positions[entry->second];
}

DirectX::XMFLOAT3 SE::Core::TransformManager::GetRotation(const Entity& e) const
{
	auto entry = entityToIndex.find(e);
	_ASSERT_EXPR(entry != entityToIndex.end(), "Undefined entity referenced in transform manager");
	return rotations[entry->second];
}

float SE::Core::TransformManager::GetScale(const Entity& e) const
{
	auto entry = entityToIndex.find(e);
	_ASSERT_EXPR(entry != entityToIndex.end(), "Undefined entity referenced in transform manager");
	return scalings[entry->second];
}

void SE::Core::TransformManager::ExpandTransforms()
{
	const uint32_t bytes = (transformCapacity + transformCapacityIncrement) * sizePerEntity;
	const uint32_t newCapacity = transformCapacity + transformCapacityIncrement;

	XMFLOAT3* newPos = new XMFLOAT3[newCapacity];
	XMFLOAT3* newRot = new XMFLOAT3[newCapacity];
	float* newScale = new float[newCapacity];

	memcpy(newPos, positions, sizeof(XMFLOAT3) * transformCount);
	memcpy(newRot, rotations, sizeof(XMFLOAT3) * transformCount);
	memcpy(newScale, scalings, sizeof(float) * transformCount);

	delete[] positions;
	delete[] rotations;
	delete[] scalings;

	positions = newPos;
	rotations = newRot;
	scalings = newScale;

	transformCapacity = newCapacity;
}
