#ifndef SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#define SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_
#include "IResourceHandler.h"
#include "IAssetLoader.h"
#include <map>

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

			int LoadResource(const Utilz::GUID& guid, const LoadResourceDelegate& callback, bool wait = false);
			void UnloadResource(const Utilz::GUID& guid);
		
		private:
			struct ResourceInfo
			{
				ResourceInfo() : data(nullptr), size(0), refCount(0), extension() {}
				void* data;
				size_t size;
				unsigned refCount;
				Utilz::GUID extension;
			};


			IAssetLoader* diskLoader;

			std::map<Utilz::GUID, ResourceInfo, Utilz::GUID::Compare> resourceMap;
		};
	}
}

#endif //SE_RESOURCE_HANDLER_RESOURCE_HANDLER_H_