#include "ResourceHandler.h"
#include <Profiler.h>
#include "RawLoader.h"
#include <vector>
#include <Utilz\Memory.h>

using namespace std::chrono_literals;

void Push_(std::mutex& lock, const std::string&& msg)
{
	lock.lock();
}

SE::ResourceHandler::ResourceHandler::ResourceHandler() : diskLoader(nullptr)
{
	
}


SE::ResourceHandler::ResourceHandler::~ResourceHandler()
{
}

int SE::ResourceHandler::ResourceHandler::Initialize(const InitializationInfo& initInfo)
{
	StartProfile;
	this->initInfo = initInfo;

	//guidToResourceInfo.

	/*switch (initInfo.RAM_UnloadingStrategy)
	{
	case EvictPolicy::Linear:
		Unload = &ResourceHandler::LinearUnload;
		break;
	default:
		Unload = &ResourceHandler::LinearUnload;
		break;
	}
*/
	diskLoader = new RawLoader;
	auto res = diskLoader->Initialize();
	if (res < 0)
		return res;
	_ASSERT(diskLoader);

	load_threadPool = new Utilz::ThreadPool(1);
	invoke_threadPool = new Utilz::ThreadPool(2);


	ProfileReturnConst(0);
}

void SE::ResourceHandler::ResourceHandler::Shutdown()
{
	delete load_threadPool;
	delete invoke_threadPool;
	auto lam = [](auto& map) {
		for (auto& r : map)
		{
			if (r.second.state & State::RAW)
				operator delete(r.second.data.data);
			else if (r.second.state & State::LOADED)
				if(r.second.destroyCallback)
					r.second.destroyCallback(r.first, r.second.data.data, r.second.data.size);
		}
	};
	Utilz::Operate(guidToRAMEntry, lam);
	Utilz::Operate(guidToVRAMEntry, lam);

	delete diskLoader;
}

const SE::ResourceHandler::InitializationInfo & SE::ResourceHandler::ResourceHandler::GetInfo() const
{
	return initInfo;
}

void SE::ResourceHandler::ResourceHandler::UpdateInfo(const InitializationInfo & initInfo)
{
	this->initInfo = initInfo;
}

void SE::ResourceHandler::ResourceHandler::GetAllGUIDsWithExtension(const Utilz::GUID& ext, std::vector<Utilz::GUID>& guids)const
{
	diskLoader->GetAllGUIDsWithExtension(ext, guids);
}
void SE::ResourceHandler::ResourceHandler::GetAllGUIDsWithExtension(const Utilz::GUID& ext, std::vector<Utilz::GUID>& guids, std::vector<std::string>& names)const
{
	diskLoader->GetAllGUIDsWithExtension(ext, guids, names);
}
int SE::ResourceHandler::ResourceHandler::LoadResource(const Utilz::GUID & guid,
	const std::function<InvokeReturn(const Utilz::GUID&, void*, size_t)>& invokeCallback, 
	LoadFlags loadFlags)
{
	Callbacks callbacks;
	callbacks.invokeCallback = invokeCallback;
	return LoadResource(guid, callbacks, loadFlags);
}

static const auto setupInfo = [](auto& resource, auto& guid, bool& load, bool& error, auto& errors, auto& loadFlags) {

	if (resource.state & SE::ResourceHandler::State::FAIL)
	{
		errors.Push_Back("Resource has previously failed, GUID: " + std::to_string(guid.id));
		error = true;
		return;
	}

	resource.ref++;

	if (!(resource.state & SE::ResourceHandler::State::LOADED))
	{
		resource.state = (resource.state ^ SE::ResourceHandler::State::DEAD) | SE::ResourceHandler::State::LOADING;
		if (loadFlags & SE::ResourceHandler::LoadFlags::IMMUTABLE)
			resource.state |= SE::ResourceHandler::State::IMMUTABLE;
		load = true;
	}
};

static const auto getResourceData = [](auto& resource, auto& out) {
	out = resource.data;
};

static const auto setFail = [](auto& resource)
{
	resource.state = SE::ResourceHandler::State::FAIL;
};

static const auto decRef = [](auto& resource) {
	if (resource.ref > 0)
		resource.ref--;
	if (resource.ref == 0u)
		resource.state |= SE::ResourceHandler::State::DEAD;
};

static const auto invoke = [](auto& map, auto& guid, auto data, auto& callback, auto& errors)
{

	auto iresult = callback(guid, data.data, data.size);
	if (iresult & SE::ResourceHandler::InvokeReturn::FAIL)
	{
		SE::Utilz::OperateSingle(map, guid, setFail);
		errors.Push_Back("Resource failed in InvokeCallback, GUID: " + std::to_string(guid.id));
		return -1;
	}

	if (iresult & SE::ResourceHandler::InvokeReturn::DEC_RAM)
		SE::Utilz::OperateSingle(map, guid, decRef);
	if (iresult & SE::ResourceHandler::InvokeReturn::DEC_VRAM)
		SE::Utilz::OperateSingle(map, guid, decRef);
	return 0;
};


int SE::ResourceHandler::ResourceHandler::LoadResource(const Utilz::GUID & guid,
	const Callbacks& callbacks,
	LoadFlags loadFlags)
{
	StartProfile;
	_ASSERT_EXPR(callbacks.invokeCallback, "An invokeCallback must be provided");
	const auto load = [this, &guid, &callbacks, loadFlags](auto& map)
	{
		bool load = false;
		bool error = false;
		Utilz::OperateSingle(map, guid, setupInfo, guid, load, error, errors, loadFlags);
		if (error)
			return -1;

		if (load)
		{
			if (loadFlags & LoadFlags::ASYNC)
				load_threadPool->Enqueue(this, &ResourceHandler::Load, &map, { guid, callbacks, loadFlags });
			else
				return Load(&map, { guid, callbacks, loadFlags });
		}
		else
		{
			Data data;
			Utilz::OperateSingle(map, guid, getResourceData, data);
			return invoke(map, guid, data, callbacks.invokeCallback, errors);
		}

		return 0;
	};
	if (loadFlags & LoadFlags::LOAD_FOR_RAM) {
		ProfileReturn(load(guidToRAMEntry));
	}
	else if (loadFlags & LoadFlags::LOAD_FOR_VRAM) {
		ProfileReturn(load(guidToVRAMEntry));
	}

	ProfileReturnConst(0);
}



void SE::ResourceHandler::ResourceHandler::UnloadResource(const Utilz::GUID & guid, UnloadFlags unloadFlags)
{
	StartProfile;

	if(unloadFlags & UnloadFlags::RAM)
		Utilz::OperateSingle(guidToRAMEntry, guid, decRef);
	if (unloadFlags & UnloadFlags::VRAM)
		Utilz::OperateSingle(guidToVRAMEntry, guid, decRef);

	StopProfile;
}

static const auto checkStillLoad = [](auto& r, auto& guid, bool& error, auto& errors)
{
	if (r.state & SE::ResourceHandler::State::DEAD)
	{
		errors.Push_Back("Resource was dead, GUID: " + std::to_string(guid.id));
		error =  false;
	}

	if (!(r.state & SE::ResourceHandler::State::LOADING))
		error = false;
};

int SE::ResourceHandler::ResourceHandler::Load(Utilz::Concurrent_Unordered_Map<Utilz::GUID, Resource_Entry, Utilz::GUID::Hasher>* map, LoadJob job)
{

	// Do some checks
	{	
		bool error = false;
		Utilz::OperateSingle(*map, job.guid, checkStillLoad, job.guid, error, errors);
		if (error)
			return -1;
	}


	// Load the resource
	Data data;
	{
		Data rawData;
		if (!diskLoader->Exist(job.guid, &rawData.size))
		{
			Utilz::OperateSingle(*map, job.guid, setFail);
			errors.Push_Back("Resource does not exist, GUID: " + std::to_string(job.guid.id));
			return -1;
		}
		loadLock.lock();
		auto result = diskLoader->LoadResource(job.guid, &rawData.data);
		if (result < 0)
		{
			loadLock.unlock();
			Utilz::OperateSingle(*map, job.guid, setFail);
			errors.Push_Back("Could not load resource, GUID: " + std::to_string(job.guid.id) + ", Error: " + std::to_string(result));
			return -1;
		}


		if (job.callbacks.loadCallback)
		{
			auto lresult = job.callbacks.loadCallback(job.guid, rawData.data, rawData.size, &data.data, &data.size);
			loadLock.unlock();
			bool error = false;
			Utilz::OperateSingle(*map, job.guid, [lresult,&error,&rawData](auto& resource)
			{
				if (lresult & LoadReturn::NO_DELETE)
					resource.state |= State::RAW;
				else
					operator delete (rawData.data);
				if (lresult & LoadReturn::FAIL)
				{
					resource.state = State::FAIL;
					
					error = true;
				}
			});
			if (error)
			{
				errors.Push_Back("Resource failed in LoadCallback, GUID: " + std::to_string(job.guid.id));
				return -1;
			}
		}
		else
		{
			loadLock.unlock();
			data = rawData;
		}
	}
	{
		Utilz::OperateSingle(*map, job.guid, [&job, data](auto& resource)
		{
			resource.state = resource.state ^ State::LOADING | State::LOADED;
			if (!job.callbacks.destroyCallback)
				resource.state |= State::RAW;

			resource.destroyCallback = job.callbacks.destroyCallback;
			resource.data = data;
			resource.state |= State::LOADED;

		});
		
	
	}

	// Invoke the invoke callback
	auto res = invoke(*map, job.guid, data, job.callbacks.invokeCallback, errors);
	if (res < 0)
	{
		return -1;
	}

	return 0;
}
//
//void SE::ResourceHandler::ResourceHandler::LinearUnload(size_t addedSize)
//{
//	StartProfile;
//	if (Utilz::Memory::IsUnderLimit(initInfo.maxMemory, addedSize))
//		ProfileReturnVoid;
//
//	size_t freed = 0;
//	std::vector<size_t> toFree;
//	for (size_t i = 0; i < resourceInfo.used; i++)
//	{
//	
//		if (resourceInfo.state[i] == State::Loaded && resourceInfo.refCount[i] == 0)
//		{
//			freed += resourceInfo.resourceData[i].size;
//			toFree.push_back(i);
//			if (freed >= addedSize)
//				break;
//		}
//	
//	}
//	if (Utilz::Memory::IsUnderLimit(initInfo.maxMemory + freed, addedSize))
//	{
//		infoLock.lock();
//		for (auto& i : toFree)
//		{
//			if (resourceInfo.state[i] == State::Loaded && resourceInfo.refCount[i] == 0)
//			{
//				operator delete(resourceInfo.resourceData[i].data);
//				resourceInfo.resourceData[i].data = nullptr;
//				resourceInfo.state[i] = State::Dead;
//
//			}
//
//		}
//		infoLock.unlock();
//	}
//	
//	ProfileReturnVoid;
//}
