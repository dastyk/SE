#include "TextManager.h"
#include <Profiler.h>


namespace SE {
	namespace Core {
		TextManager::TextManager(const InitializationInfo & initInfo) : initInfo(initInfo)
		{
			_ASSERT(initInfo.resourceHandler);
			_ASSERT(initInfo.renderer);
			_ASSERT(initInfo.entityManager);

			auto ret = initInfo.resourceHandler->LoadResource("moonhouse.spritefont", [this](auto guid, auto data, auto size) {
				guidToFont[guid] = this->initInfo.renderer->CreateTextFont(data, size);
				return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM; });
			if (ret)
				throw std::exception("Could not load default font.");
		}

		TextManager::~TextManager()
		{

		}

		void TextManager::Create(const Entity& entity, const CreateInfo& info)
		{
			StartProfile;

			// Check if the entity is alive
			if (!initInfo.entityManager->Alive(entity))
				ProfileReturnVoid;
			auto font = info.font;
			if (info.font == Utilz::GUID())
			{
				font = "moonhouse.spritefont";
			}
			else
			{
				auto const findFont = guidToFont.find(info.font);
				if (findFont == guidToFont.end())
				{
					auto ret = initInfo.resourceHandler->LoadResource(info.font, { this, &TextManager::LoadFont });
					if (ret)
					{
						ProfileReturnVoid;
					}
				}
			}
			

			entID[entity].ID = loadedTexts.size();
			entID[entity].font = font;
			textEnt.push_back(entity);
			loadedTexts.push_back(info.info);
		/*	if (!loadedTexts[loadedTexts.size() - 1].anchor)
			{
				loadedTexts[loadedTexts.size() - 1].pos = DirectX::XMFLOAT2(loadedTexts[loadedTexts.size() - 1].pos.x / width, loadedTexts[loadedTexts.size() - 1].pos.y / height);
				loadedTexts[loadedTexts.size() - 1].scale = DirectX::XMFLOAT2(loadedTexts[loadedTexts.size() - 1].scale.x / width, loadedTexts[loadedTexts.size() - 1].scale.y / height);
			}*/
			ProfileReturnVoid;
		}

		void TextManager::ToggleRenderableText(const Entity & entity, bool show)
		{
			StartProfile;
			// chexk if entity exist in text 
			auto fileLoaded = entID.find(entity);
			if (fileLoaded != entID.end())
			{
				if (show && !fileLoaded->second.show)
				{
					fileLoaded->second.jobID = initInfo.renderer->EnableTextRendering({ guidToFont[fileLoaded->second.font], originalScreenWidth, originalScreenHeight,  loadedTexts[fileLoaded->second.ID] });
					textJobobToEnt[fileLoaded->second.jobID] = entity;
					fileLoaded->second.show = true;
				}
				else if (!show && fileLoaded->second.show)
				{
					size_t tempJobID = initInfo.renderer->DisableTextRendering(fileLoaded->second.jobID);
					fileLoaded->second.show = false;
					entID[textJobobToEnt[tempJobID]].jobID = fileLoaded->second.jobID;
					textJobobToEnt[fileLoaded->second.jobID] = textJobobToEnt[tempJobID];
					textJobobToEnt.erase(tempJobID);
				}
			}
			StopProfile;
		}

		int TextManager::MakeFont(const Utilz::GUID& fontFile)
		{
			StartProfile;
			auto ret = initInfo.resourceHandler->LoadResource(fontFile, [this](auto guid, auto data, auto size) {
				guidToFont[guid] = initInfo.renderer->CreateTextFont(data, size);
				return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM ; });

			ProfileReturnConst(0);
		}

		void TextManager::Frame(Utilz::TimeCluster * timer)
		{
			StartProfile;
			timer->Start(CREATE_ID_HASH( "GUIManager"));
			for (auto& dirt : dirtyEnt)
			{
				// Check if the entity is alive
				if (!initInfo.entityManager->Alive(dirt.first))
					continue;
				if (dirt.second == true && entID[dirt.first].show == true)
				{
					ToggleRenderableText(dirt.first, false);
					ToggleRenderableText(dirt.first, true);
				}
				dirt.second = false;
			}
			dirtyEnt.clear();
			GarbageCollection();
			timer->Stop(CREATE_ID_HASH("GUIManager"));
			StopProfile;
		}

		void TextManager::updateText()
		{
			StartProfile;
			if (loadedTexts.size() > 0)
			{
				for (auto& entity : textEnt)
				{
					if (entID[entity].show)
					{
						ToggleRenderableText(entity, false);
						ToggleRenderableText(entity, true);
					}
				}
			}
			StopProfile;
		}

		void TextManager::Destroy(size_t index)
		{
			StartProfile;
			// Temp variables
			size_t last = loadedTexts.size() - 1;
			const Entity entity = textEnt[index];
			const Entity last_entity = textEnt[last];

			// Copy the data
			textEnt[index] = last_entity;
			loadedTexts[index] = loadedTexts[last];
			entID[last_entity] = entID[entity];

			// Remove last spot 
			entID.erase(entity);
			loadedTexts.pop_back();
			textEnt.pop_back();

			StopProfile;
		}

		void TextManager::GarbageCollection()
		{
			StartProfile;
			uint32_t alive_in_row = 0;
			while (loadedTexts.size() > 0 && alive_in_row < 4U)
			{
				std::uniform_int_distribution<uint32_t> distribution(0U, loadedTexts.size() - 1U);
				uint32_t i = distribution(generator);
				if (initInfo.entityManager->Alive(textEnt[i]))
				{
					alive_in_row++;
					continue;
				}
				alive_in_row = 0;
				ToggleRenderableText(textEnt[i], false);
				Destroy(i);
			}
			StopProfile;
		}

		void TextManager::Destroy(const Entity & entity)
		{
			StartProfile;
			auto fileLoaded = entID.find(entity);
			if (fileLoaded != entID.end())
			{
				// Temp variables
				size_t last = loadedTexts.size() - 1;
				size_t index = entID[entity].ID;
				const Entity last_entity = textEnt[last];

				// Copy the data
				textEnt[index] = last_entity;
				loadedTexts[index] = loadedTexts[last];
				entID[last_entity] = entID[entity];

				// Remove last spot 
				entID.erase(entity);
				loadedTexts.pop_back();
				textEnt.pop_back();
			}

			StopProfile;
		}
	}
}