#include "ResourceHandler.h"
#include <Profiler.h>
#include "RawLoader.h"
#include <vector>
#include <Utilz\Memory.h>

using namespace std::chrono_literals;

SE::ResourceHandler::ResourceHandler::ResourceHandler() : diskLoader(nullptr), stop(false)
{
	
}


SE::ResourceHandler::ResourceHandler::~ResourceHandler()
{
}

int SE::ResourceHandler::ResourceHandler::Initialize(const InitializationInfo& initInfo) 
{
	StartProfile;
	this->initInfo = initInfo;

	diskLoader = new RawLoader;
	auto res = diskLoader->Initialize();
	if (res < 0)
		return res;
	_ASSERT(diskLoader);

	//load_threadPool = new Utilz::ThreadPool(2);

	for (size_t i = 0; i < 4; ++i)
		workers.emplace_back(
			[this]
	{
		for (;;)
		{
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(this->queue_mutex);
				this->condition.wait(lock,
					[this] { return this->stop || !this->tasks.empty(); });
				if (this->stop && this->tasks.empty())
					return;
				task = std::move(this->tasks.front());
				this->tasks.pop();
			}

			task();
		}
	}
	);
	ProfileReturnConst(0);
}

void SE::ResourceHandler::ResourceHandler::Shutdown()
{
	//delete load_threadPool;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();

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
	if (iresult & SE::ResourceHandler::InvokeReturn::SEMI_FAIL)
	{
		errors.Push_Back("Resource semi-failed in InvokeCallback, GUID: " + std::to_string(guid.id));
		return -2;
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
	const auto load = [this, &guid, &callbacks, loadFlags](auto& map, auto& evictInfo)
	{
		bool load = false;
		bool error = false;
		Utilz::OperateSingle(map, guid, setupInfo, guid, load, error, errors, loadFlags);
		if (error)
			return -1;


		if (load)
		{
			if (loadFlags & LoadFlags::ASYNC)
			{
				{
					std::unique_lock<std::mutex> lock(queue_mutex);

					// don't allow enqueueing after stopping the pool
					if (stop)
						throw std::runtime_error("enqueue on stopped ThreadPool");

					LoadJob c = { guid, callbacks, loadFlags };
					auto asd = [this, &map, &evictInfo, c]()->void { Load(map, evictInfo, c); } ;
					tasks.emplace(asd);
				}
				condition.notify_one();		
			}
				//load_threadPool->Enqueue(this, &ResourceHandler::Load, &map, { guid, callbacks, loadFlags });
			else
				return Load(map,evictInfo, { guid, callbacks, loadFlags });
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
		ProfileReturn(load(guidToRAMEntry, initInfo.RAM));
	}
	else if (loadFlags & LoadFlags::LOAD_FOR_VRAM) {
		ProfileReturn(load(guidToVRAMEntry, initInfo.VRAM));
	}

	
	ProfileReturnConst(0);
}



void SE::ResourceHandler::ResourceHandler::UnloadResource(const Utilz::GUID & guid, ResourceType type)
{
	StartProfile;

	if(type & ResourceType::RAM)
		Utilz::OperateSingle(guidToRAMEntry, guid, decRef);
	else if (type & ResourceType::VRAM)
		Utilz::OperateSingle(guidToVRAMEntry, guid, decRef);

	StopProfile;
}

bool SE::ResourceHandler::ResourceHandler::IsResourceLoaded(const Utilz::GUID & guid, ResourceType type)
{
	bool found = false;
	if (type & ResourceType::RAM)
		Utilz::Find(guidToRAMEntry, guid, [&found](auto& r) {
		if (r->second.state & State::LOADED)
			found = true;
	});
	else if (type & ResourceType::VRAM)
		Utilz::Find(guidToVRAMEntry, guid, [&found](auto& r) {
		if (r->second.state & State::LOADED)
			found = true;
	});

	return found;
}

bool SE::ResourceHandler::ResourceHandler::Exist(const Utilz::GUID guid)
{
	return diskLoader->Exist(guid, nullptr);
}

size_t SE::ResourceHandler::ResourceHandler::GetMemoryUsed(ResourceType type)
{
	if (type & ResourceType::RAM)
	{
		size_t ramTot = 0;
		Utilz::Operate(guidToRAMEntry, [&ramTot](auto& map) {
			for (auto& r : map)
			{
				if (r.second.state & State::LOADED)
					ramTot += r.second.data.size;
			}
		});
		return ramTot;
	}
	else if (type & ResourceType::VRAM)
	{
		size_t vramTot = 0;
		Utilz::Operate(guidToVRAMEntry, [&vramTot](auto& map) {
			for (auto& r : map)
			{
				if (r.second.state & State::LOADED)
					vramTot += r.second.data.size;
			}
		});
		return vramTot;
	}
	return 0;
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
static const auto linearEvict = [](auto& map, auto& out)
{
	for (auto& resource : map) {
		if (!(resource.second.state & SE::ResourceHandler::State::IMMUTABLE) && resource.second.state & SE::ResourceHandler::State::DEAD && resource.second.state & SE::ResourceHandler::State::LOADED && resource.second.ref == 0)
		{
			out.push_back(resource.first);
		}
	}
};

template<class COMP, class KEY, class VAL>
void insert(std::vector<KEY> &sortArray, KEY key, std::vector<VAL> &valueArray, VAL value) {
	std::vector<KEY>::iterator it = std::lower_bound(sortArray.begin(), sortArray.end(), key, COMP()); // find proper position in descending order
	it = sortArray.insert(it, key); // insert before iterator it
	auto pos = it - sortArray.begin();
	valueArray.insert(valueArray.begin() + pos, value); // insert before iterator it
}

static const auto fifoEvict = [](auto& map, auto& out)
{
	std::vector<std::chrono::high_resolution_clock::time_point> sortArr;
	for (auto& resource : map) {
		if (!(resource.second.state & SE::ResourceHandler::State::IMMUTABLE) && resource.second.state & SE::ResourceHandler::State::DEAD && resource.second.state & SE::ResourceHandler::State::LOADED && resource.second.ref == 0)
		{
			insert<std::less<std::chrono::high_resolution_clock::time_point>>(sortArr, resource.second.loadedAt, out, resource.first);
		}
	}
};

static const auto lifoEvict = [](auto& map, auto& out)
{
	std::vector<std::chrono::high_resolution_clock::time_point> sortArr;
	for (auto& resource : map) {
		if (!(resource.second.state & SE::ResourceHandler::State::IMMUTABLE) && resource.second.state & SE::ResourceHandler::State::DEAD && resource.second.state & SE::ResourceHandler::State::LOADED && resource.second.ref == 0)
		{
			insert<std::greater<std::chrono::high_resolution_clock::time_point>>(sortArr, resource.second.loadedAt, out, resource.first);
		}
	}
};

static const auto randomEvict = [](auto& map, auto& out)
{
	linearEvict(map, out);
	std::random_shuffle(out.begin(), out.end());
};

static const auto lufoEvict = [](auto& map, auto& out)
{
	std::vector<size_t> sortArr;
	for (auto& resource : map) {
		if (!(resource.second.state & SE::ResourceHandler::State::IMMUTABLE) && resource.second.state & SE::ResourceHandler::State::DEAD && resource.second.state & SE::ResourceHandler::State::LOADED && resource.second.ref == 0)
		{
			insert<std::less<size_t>>(sortArr, resource.second.timesLoaded, out, resource.first);
		}
	}
};
static const auto mufoEvict = [](auto& map, auto& out)
{
	std::vector<size_t> sortArr;
	for (auto& resource : map) {
		if (!(resource.second.state & SE::ResourceHandler::State::IMMUTABLE) && resource.second.state & SE::ResourceHandler::State::DEAD && resource.second.state & SE::ResourceHandler::State::LOADED && resource.second.ref == 0)
		{
			insert<std::greater<size_t>>(sortArr, resource.second.timesLoaded, out, resource.first);
		}
	}
};
int SE::ResourceHandler::ResourceHandler::Load(entryMap& map, const EvictInfo& evictInfo, LoadJob job)
{

	// Do some checks
	{	
		bool error = false;
		Utilz::OperateSingle(map, job.guid, checkStillLoad, job.guid, error, errors);
		if (error)
			return -1;
	}


	// Load the resource
	Data data;
	{
	Data rawData;
	if (!diskLoader->Exist(job.guid, &rawData.size))
	{
		Utilz::OperateSingle(map, job.guid, setFail);
		errors.Push_Back("Resource does not exist, GUID: " + std::to_string(job.guid.id));
		return -1;
	}

	auto mem = evictInfo.getCurrentMemoryUsage();
	auto limit = evictInfo.max * evictInfo.tryUnloadWhenOver;
	auto total = mem + rawData.size;
	if (total >= limit)
	{
		errors.Push_Back("Unloading stuff. Limit: " + std::to_string(toMB(limit)) + "mb, Current: " + std::to_string(toMB(total))+ "mb");
		auto needed = total - limit;

		std::vector<Utilz::GUID> order;
		if(evictInfo.nloadingStrategy == EvictPolicy::LINEAR)
			Utilz::Operate(map, linearEvict, order);
		else if (evictInfo.nloadingStrategy == EvictPolicy::FIFO)
			Utilz::Operate(map, fifoEvict, order);
		else if (evictInfo.nloadingStrategy == EvictPolicy::LIFO)
			Utilz::Operate(map, lifoEvict, order);
		else if (evictInfo.nloadingStrategy == EvictPolicy::RANDOM)
			Utilz::Operate(map, randomEvict, order);
		else if (evictInfo.nloadingStrategy == EvictPolicy::LEST_USED)
			Utilz::Operate(map, lufoEvict, order);
		else if (evictInfo.nloadingStrategy == EvictPolicy::MOST_USED)
			Utilz::Operate(map, mufoEvict, order);

		EvictResources(map, order, needed);
		errors.Push_Back("Current after: " + std::to_string(toMB(evictInfo.getCurrentMemoryUsage())));
	}

	auto result = diskLoader->LoadResource(job.guid, &rawData.data);
	if (result < 0)
	{
		Utilz::OperateSingle(map, job.guid, setFail);
		errors.Push_Back("Could not load resource, GUID: " + std::to_string(job.guid.id) + ", Error: " + std::to_string(result));
		return -1;
	}

	if (job.callbacks.loadCallback)
	{
		//loadCallbackLock.lock();
		auto lresult = job.callbacks.loadCallback(job.guid, rawData.data, rawData.size, &data.data, &data.size);
		//loadCallbackLock.unlock();

		bool error = false;
		Utilz::OperateSingle(map, job.guid, [lresult, &error, &rawData](auto& resource)
		{
			if (lresult & LoadReturn::NO_DELETE)
				resource.state |= State::RAW;
			else
			{
				operator delete(rawData.data);
				rawData.size = 0;
			}
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
		data = rawData;
	}
	
	}
	{
		Utilz::OperateSingle(map, job.guid, [&job, data](auto& resource)
		{
			resource.loadedAt = std::chrono::high_resolution_clock::now();
			resource.timesLoaded++;
			resource.state = resource.state ^ State::LOADING | State::LOADED;
			if (!job.callbacks.destroyCallback)
				resource.state |= State::RAW;

			resource.destroyCallback = job.callbacks.destroyCallback;
			resource.data = data;
			resource.state |= State::LOADED;

		});


	}

	// Invoke the invoke callback
	auto res = invoke(map, job.guid, data, job.callbacks.invokeCallback, errors);
	if (res < 0)
	{
		return -1;
	}

#include <Utilz\Memory.h>
	errors.Push_Back("RH IN MEM: RAM: " + std::to_string(toMB(GetMemoryUsed(ResourceType::RAM))) + "mb VRAM: " + std::to_string(toMB(GetMemoryUsed(ResourceType::VRAM))) + "mb");

	return 0;
}



static const auto evictResources = [](auto& map, auto& order, size_t needed, size_t& total, size_t& count)
{
	for (auto& r : order)
	{
		auto& re = map[r];
		if (!(re.state & SE::ResourceHandler::State::IMMUTABLE) && re.state & SE::ResourceHandler::State::DEAD && re.state & SE::ResourceHandler::State::LOADED && re.ref == 0)
		{
			count++;
			total += re.data.size;
			re.state = re.state ^ SE::ResourceHandler::State::LOADED;
			if (re.state & SE::ResourceHandler::State::RAW)
			{
				operator delete(re.data.data);
				re.data.data = nullptr;
				re.data.size = 0;
			}
			else	
				re.destroyCallback(r, re.data.data, re.data.size);
			
			if (total >= needed)
				break;
		}
	}

};

void SE::ResourceHandler::ResourceHandler::EvictResources(entryMap& map,
	const std::vector<Utilz::GUID>& order, size_t needed)
{
	if (order.size())
	{
		size_t count = 0;
		size_t total = 0;
		Utilz::Operate(map, evictResources, order, needed, total, count);
		errors.Push_Back("Unloaded " + std::to_string(count) + " resources, " + std::to_string(toMB( total)) + "mb");
	}
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
