#ifndef SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#include <IResourceHandler.h>
#include "IAssetLoader.h"
#include <map>
#include <thread>
#include <mutex>
#include <stack>
#include <Utilz\Event.h>

namespace SE
{
	namespace ResourceHandler
	{
		/**
		*
		* @brief The resource handler
		*
		* @details The resource handler can process requests for loading resources.
		* It also keeps track of which resources are already loaded.
		*
		**/
		class ResourceHandler : public IResourceHandler
		{
		public:
			ResourceHandler();
			~ResourceHandler();

			int Initialize();
			void Shutdown();

			int LoadResource(const Utilz::GUID& guid, const LoadResourceDelegate& callback, bool async = false, Behavior behavior = Behavior::QUICK);
			void UnloadResource(const Utilz::GUID& guid);
		
		private:
			/**
			* @brief	Allocate more memory
			*/
			void Allocate(size_t size);
			/**
			* @brief	Remove an entry
			*/
			void Destroy(size_t index);

			enum class State : uint8_t
			{
				Loaded,
				Loading,
				Dead
			};
			void Run();

			struct Data
			{
				void* data;
				size_t size;
			};
			struct ResourceInfo
			{
				static const size_t size = sizeof(Data) + sizeof(uint16_t) + sizeof(State) + sizeof(Utilz::GUID);
				size_t allocated = 0;
				size_t used = 0;
				void* data = nullptr;
				Data* resourceData;
				uint16_t* refCount;
				State* state;
				Utilz::GUID* extension;
			};
			struct ToLoadInfo
			{
				Utilz::GUID guid;
				size_t resourceInfoIndex;
				Utilz::Event<int(const Utilz::GUID&, void*data, size_t)> callbacks;
				Behavior behavior;
			};

			void LoadAsync();
			void CreateLoadJob(const Utilz::GUID& guid, size_t index, const LoadResourceDelegate& callback, Behavior behavior);
			void UpdateLoadJob(ToLoadInfo& loadInfo, const LoadResourceDelegate& callback, Behavior behavior);
			void RemoveLoadJob(const Utilz::GUID& guid);
			int LoadSync(const Utilz::GUID& guid, size_t index, const LoadResourceDelegate& callback);
			int InvokeCallback(const Utilz::GUID& guid, size_t index, const LoadResourceDelegate& callback);

			IAssetLoader* diskLoader;

			ResourceInfo resourceInfo;
			std::map<Utilz::GUID, size_t, Utilz::GUID::Compare> guidToResourceInfoIndex;

			std::map<Utilz::GUID, ToLoadInfo, Utilz::GUID::Compare> toLoad;
			std::mutex toLoadLock;
			std::mutex infoLock;
			std::thread myThread;
			bool running;
		};
	}
}

#endif //SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_