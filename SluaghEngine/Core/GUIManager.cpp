#include <Core\GUIManager.h>
#include <Profiler.h>
#include <Utilz\Console.h>
#include <OBJParser\Parsers.h>
#include <list>


#ifdef _DEBUG
#pragma comment(lib, "OBJParserD.lib")
#else
#pragma comment(lib, "OBJParser.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "UtilzD.lib")
#else
#pragma comment(lib, "Utilz.lib")
#endif

namespace SE {
	namespace Core {
		GUIManager::GUIManager(ResourceHandler::IResourceHandler * resourceHandler, Graphics::IRenderer* renderer, const EntityManager & entityManager)
			:resourceHandler(resourceHandler), renderer(renderer), entityManager(entityManager)
		{
			_ASSERT(resourceHandler);
			_ASSERT(renderer);

			amountOfFonts = renderer->CreateTextFont(Utilz::GUID("moonhouse.spritefont"), resourceHandler);

			StopProfile;
		}

		GUIManager::~GUIManager()
		{

		}

		void GUIManager::CreateRenderableText(const Entity& entity, Graphics::TextGUI inTextInfo)
		{
			StartProfile;

			// Check if the entity is alive
			if (!entityManager.Alive(entity))
				ProfileReturnVoid;

			if (!(inTextInfo.fontID <= amountOfFonts))
				ProfileReturnVoid;
			
			entID[entity] = loadedTexts.size();
			ent.push_back(entity);
			loadedTexts.push_back(inTextInfo);
			ProfileReturnVoid;	
		}

		void GUIManager::ToggleRenderableText(const Entity & entity, bool show)
		{
			StartProfile;
			// chexk if entity exist in text 
			auto fileLoaded = entID.find(entity);
			if (fileLoaded != entID.end())
			{
				if (show)
				{
					renderer->EnableTextRendering(loadedTexts[entID[entity]]);
				}
				else
				{
					renderer->DisableTextRendering(loadedTexts[entID[entity]]);
				}
				ProfileReturnVoid;
			}
			StopProfile;
		}

		void GUIManager::Frame()
		{
			GarbageCollection();
		}

		int GUIManager::CreateTextFont(Utilz::GUID fontFile)
		{
			StartProfile;
			amountOfFonts = renderer->CreateTextFont(fontFile, resourceHandler);
			ProfileReturnConst(0);
		}

		int GUIManager::Create2D(Utilz::GUID texFile)
		{
			StartProfile;
			auto fileLoaded = textureGUID.find(texFile);
			if (fileLoaded == textureGUID.end())
			{
				textureGUID[texFile].textureHandle = -1;
				resourceHandler->LoadResource(texFile, ResourceHandler::LoadResourceDelegate::Make<GUIManager, &GUIManager::LoadTexture>(this)); 
				ProfileReturnConst(-1);
			}
			else if (textureGUID[texFile].textureHandle != -1)
			{
				ProfileReturn(textureGUID[texFile].textureHandle);
			}
			ProfileReturnConst(-1);
		}

		int GUIManager::Bind2D(const Entity & entity, Utilz::GUID texFile, Graphics::GUITextureInfo & texInfo)
		{
			StartProfile;
			auto fileLoaded = textureGUID.find(texFile);
			if (fileLoaded != textureGUID.end())
			{
				// Check if the entity is alive
				if (!entityManager.Alive(entity))
					ProfileReturnConst(-1);

				entTextureID[entity].ID = textureInfo.size();
				entTextureID[entity].GUID = texFile;
				textureGUID[texFile].refCount++;
				textureEnt.push_back(entity);
				textureInfo.push_back(texInfo);
				textureInfo[textureInfo.size() - 1].textureID = textureGUID[texFile].textureHandle;
				ProfileReturnConst(0);
			}
			ProfileReturnConst(-1);
		}

		void GUIManager::ToggleRenderableTexture(const Entity & entity, bool show)
		{
			StartProfile;
			// chexk if entity exist in text 
			auto fileLoaded = entTextureID.find(entity);
			if (fileLoaded != entTextureID.end())
			{
				if (show)
				{
					renderer->EnableTextureRendering(textureInfo[entTextureID[entity].ID]);
				}
				else
				{
					renderer->DisableTextureRendering(textureInfo[entTextureID[entity].ID]);
				}
				ProfileReturnVoid;
			}
			StopProfile;
		}

		void GUIManager::Shutdown()
		{
			entID.clear();
			loadedTexts.clear();
		}

		void GUIManager::DestroyText(size_t index)
		{
			StartProfile;
			// Temp variables
			size_t last = loadedTexts.size() - 1;
			const Entity& entity = ent[index];
			const Entity& last_entity = ent[last];

			// Copy the data
			ent[index] = last_entity;
			loadedTexts[index] = loadedTexts[last];
			entID[last_entity] = entID[entity];

			// Remove last spot 
			entID.erase(entity);
			loadedTexts.pop_back();
			ent.pop_back();

			StopProfile;	
		}

		void GUIManager::DestroyTexture(size_t index)
		{
			StartProfile;
			// Temp variables
			size_t last = textureInfo.size() - 1;
			const Entity& entity = textureEnt[index];
			const Entity& last_entity = textureEnt[last];

			// Copy the data
			textureEnt[index] = last_entity;
			textureInfo[index] = textureInfo[last];
			textureGUID[entTextureID[entity].GUID].refCount--;
			entTextureID[last_entity].ID = entTextureID[entity].ID;

			// Remove last spot 
			entTextureID.erase(entity);
			textureInfo.pop_back();
			textureEnt.pop_back();

			StopProfile;
		}

		void GUIManager::GarbageCollection()
		{
			StartProfile;
			uint32_t alive_in_row = 0;
			if (garbage == false)
			{
				while (loadedTexts.size() > 0 && alive_in_row < 4U)
				{
					std::uniform_int_distribution<uint32_t> distribution(0U, loadedTexts.size() - 1U);
					uint32_t i = distribution(generator);
					if (entityManager.Alive(ent[i]))
					{
						alive_in_row++;
						continue;
					}
					alive_in_row = 0;
					renderer->DisableTextRendering(loadedTexts[entID[ent[i]]]);
					DestroyText(i);
				}
				garbage = true;
			}
			else
			{
				while (textureInfo.size() > 0 && alive_in_row < 4U)
				{
					std::uniform_int_distribution<uint32_t> distribution(0U, textureInfo.size() - 1U);
					uint32_t i = distribution(generator);
					if (entityManager.Alive(textureEnt[i]))
					{
						alive_in_row++;
						continue;
					}
					alive_in_row = 0;
					renderer->DisableTextureRendering(textureInfo[entTextureID[textureEnt[i]].ID]);
					DestroyTexture(i);
				}
				garbage = false;
			}
			StopProfile;
		}

		int GUIManager::LoadTexture(const Utilz::GUID & guid, void * data, size_t size)
		{
			StartProfile;
			Graphics::TextureDesc td;
			memcpy(&td, data, sizeof(td));
			/*Ensure the size of the raw pixel data is the same as the width x height x size_per_pixel*/
			if (td.width * td.height * 4 != size - sizeof(td))
				ProfileReturnConst(-1);
			void* rawTextureData = ((char*)data) + sizeof(td);
			auto handle = renderer->CreateTexture(rawTextureData, td);
			if (handle == -1)
				ProfileReturnConst(-1);
			textureGUID[guid].textureHandle = handle;
			textureGUID[guid].refCount = 0;

			ProfileReturnConst(0);
		}

	}
}