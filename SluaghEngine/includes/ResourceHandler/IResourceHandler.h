#ifndef SE_RESOURCE_HANDLER_IRESOURCE_HANDLER_H_
#define SE_RESOURCE_HANDLER_IRESOURCE_HANDLER_H_
#include <Utilz\GUID.h>
#include <functional>
#include <Utilz\Delegate.h>
#include <Utilz\Memory.h>
#include <vector>
#if defined DLL_EXPORT_RH
#define DECLDIR __declspec(dllexport)
#else
#define DECLDIR __declspec(dllimport)
#endif


#define ENUM_FLAG_OPERATOR(T,X) inline T operator X (T lhs, T rhs) { return (T) (static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
#define ENUM_FLAG_OPERATOR2(T,X) inline void operator X= (T& lhs, T rhs) { lhs = (T)(static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
#define ENUM_FLAGS(T) \
enum class T; \
inline T operator ~ (T t) { return (T) (~static_cast<std::underlying_type_t <T>>(t)); } \
inline bool operator & (T lhs, T rhs) { return (static_cast<std::underlying_type_t <T>>(lhs) & static_cast<std::underlying_type_t <T>>(rhs));  } \
ENUM_FLAG_OPERATOR2(T,|) \
ENUM_FLAG_OPERATOR2(T,&) \
ENUM_FLAG_OPERATOR(T,|) \
ENUM_FLAG_OPERATOR(T,^)
//enum class T
//ENUM_FLAG_OPERATOR(T,&)

#include <functional>

namespace SE
{
	namespace ResourceHandler
	{

		enum class LoadReturn
		{
			SUCCESS = 1 << 0,
			FAIL = 1 << 1,
			SEMI_FAIL = 1 << 2,
			NO_DELETE = 1 << 3
		};

		enum class InvokeReturn {
			FAIL = 1 << 1,
			SUCCESS = 1 << 2,
			DEC_RAM  = 1 << 3,
			DEC_VRAM = 1 << 4,
			SEMI_FAIL = 1 << 5
		};

		enum class LoadFlags {
			LOAD_FOR_VRAM = 1 << 0,
			LOAD_FOR_RAM = 1 << 1,
			ASYNC = 1 << 2,
			IMMUTABLE = 1 << 3
		};

		enum class ResourceType {
			VRAM = 1 << 0,
			RAM = 1 << 1
		};

		struct Callbacks
		{
			std::function<LoadReturn(const Utilz::GUID&, void*, size_t, void**, size_t*)> loadCallback;
			std::function<InvokeReturn(const Utilz::GUID&, void*, size_t)> invokeCallback;
			std::function<void(const Utilz::GUID&, void*, size_t)> destroyCallback;
		};

		enum class EvictStrickness
		{
			IMMIDIATE,
			HARD,
			SEMI_HARD,
			SEMI_LOW,
			LOW,
			ONLY_WHEN_FULL
		};
	
		enum class EvictPolicy
		{
			LINEAR,
			FIFO,
			LIFO,
			RANDOM,
			LEST_USED,
			MOST_USED
		};

		struct EvictInfo
		{
			size_t max = 256_mb;
			float tryUnloadWhenOver = 0.5f; /**< The resource handler will start trying to unload resource when the max times this factor is over current usage.*/
			EvictPolicy nloadingStrategy = EvictPolicy::LINEAR; /**< How the resource handler will look for resources to evict. */
			EvictStrickness evictStrickness = EvictStrickness::SEMI_HARD; /**< How strict the resource handler should be when checking if resources need to be unloaded. */
			std::function<size_t()> getCurrentMemoryUsage = []()->size_t { return ~0; }; /**< The callback the resource handler will use to get the memory used right now*/
		};

		struct InitializationInfo
		{
			EvictInfo RAM = { 256_mb, 0.5f, EvictPolicy::LINEAR, EvictStrickness::SEMI_HARD };
			EvictInfo VRAM = { 512_mb, 0.5f, EvictPolicy::LINEAR, EvictStrickness::SEMI_HARD };
		};

		/**
		*
		* @brief Resource Handler Interface
		*
		* @details The virtual resource handler class. The resource handler is used to loaded all assets.
		* @sa LoadResource
		*
		**/
		class IResourceHandler
		{
		public:
			virtual ~IResourceHandler() {};

			/**
			* @brief	Initialize the resource handler
			*
			* @retval 0 On success.
			* @retval -1 Asset loader failed to load
			*/
			virtual int Initialize(const InitializationInfo& initInfo) = 0;

			virtual void Shutdown() = 0;

			virtual const InitializationInfo& GetInfo()const = 0;
			virtual void UpdateInfo(const InitializationInfo& initInfo) = 0;

			virtual void GetAllGUIDsWithExtension(const Utilz::GUID& ext, std::vector<Utilz::GUID>& guids)const = 0;
			virtual void GetAllGUIDsWithExtension(const Utilz::GUID& ext, std::vector<Utilz::GUID>& guids, std::vector<std::string>& names)const = 0;

			/**
			* @brief	Load a given resource
			*
			* @details This function will load a resource for either RAM or VRAM, and in either SYNC or ASYNC. This is specified in the loadFlags argument.
			* 
			* Not providing the load and destroy callbacks the resource is considered RAW and will be handled by the resource handler.
			* This only works with RAM, if specifying VRAM the resource handler can not handle unloading of the resource from VRAM.
			*
			* @param[in] guid The GUID of the resource to be unloaded.
			* @param[in] callbacks A struct containing the callbacks used when loading a resource. Each callback is a std function with a specific template.
			* @param[in] loadFlags See UnloadFlags.
			* @warning When using asynchronious loading you need to make sure that your invoke and destroy callback is threadsafe!!!! Load does not need to be threadsafe (for now).
			* @warning When VRAM has been specified AND a destroy callback is not provided the resource handler can not unload resources from VRAM.
			* @sa LoadFlags
			* @sa Callbacks
			**/
			virtual int LoadResource(const Utilz::GUID& guid,
				const Callbacks& callbacks,
				LoadFlags loadFlags) = 0;
			
			/**
			* @brief	Load a given resource, with only an invoke.
			*
			* @details This function will load a resource for either RAM or VRAM, and in either SYNC or ASYNC. This is specified in the loadFlags argument.
			*
			* When using this overload the load and destroy callbacks are not used. This means that the resource is considered RAW and will be handled by the resource handler.
			* This only works with RAM, if specifying VRAM the resource handler can not handle unloading of the resource from VRAM.
			*
			* @param[in] guid The GUID of the resource to be unloaded.
			* @param[in] invokeCallback The callback to use when the resource has been loaded.
			* @param[in] loadFlags See UnloadFlags.
			* @warning When using asynchronious loading you need to make sure that your invoke and destroy callback is threadsafe!!!! Load does not need to be threadsafe (for now).
			* @warning When VRAM has been specified AND a destroy callback is not provided the resource handler can not unload resources from VRAM.
			* @sa LoadFlags
			* @sa Callbacks
			**/
			virtual int LoadResource(const Utilz::GUID& guid, 
				const std::function<InvokeReturn(const Utilz::GUID&, void*, size_t)>& invokeCallback, 
				LoadFlags loadFlags = LoadFlags::LOAD_FOR_RAM) = 0;

			/**
			* @brief	Unload the given resource
			*
			* @details Decreases the refcount for the given resource.
			* When the refcount reaches 0 the resource handler may unload the resource.
			* Either by dumping the memory to disk, or just discarding it.
			*
			* @param[in] guid The GUID of the resource to be unloaded.
			* @param[in] type See ResourceType.
			* @warning This does not force the resource to unload!
			**/
			virtual void UnloadResource(const Utilz::GUID& guid, ResourceType type) = 0;

			/**
			* @brief Checks if a resource is loaded or not.
			*
			**/
			virtual bool IsResourceLoaded(const Utilz::GUID& guid, ResourceType type) = 0;

			/**
			* @brief	Checks if a resource exist.
			*
			**/
			virtual bool Exist(const Utilz::GUID guid) = 0;
				
			/**
			* @brief	Get the amount resource memory that is currently in memory.
			*
			**/
			virtual size_t GetMemoryUsed(ResourceType type) = 0;


			/**
			* @brief	Get the error messages that have accumulated. This will also clear the errors messages.
			*
			**/
			virtual void GetErrorMessages(std::vector<std::string>& errors) = 0;

		protected:
			IResourceHandler() {};
			IResourceHandler(const IResourceHandler& other) = delete;
			IResourceHandler(const IResourceHandler&& other) = delete;
			IResourceHandler& operator=(const IResourceHandler& other) = delete;
		};

		DECLDIR IResourceHandler* CreateResourceHandler();
	}
}

ENUM_FLAGS(SE::ResourceHandler::LoadReturn);
ENUM_FLAGS(SE::ResourceHandler::InvokeReturn);
ENUM_FLAGS(SE::ResourceHandler::LoadFlags);
ENUM_FLAGS(SE::ResourceHandler::ResourceType);


#endif //SE_RESOURCE_HANDLER_IRESOURCE_HANDLER_H_