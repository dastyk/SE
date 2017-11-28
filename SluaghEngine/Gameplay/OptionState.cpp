#include "OptionState.h"
#include <Profiler.h>
#include "CoreInit.h"

using namespace SE;
using namespace Gameplay;

OptionState::OptionState()
{
	StartProfile;
	fileParser.entityIndex = 0;
	fileParser.ParseFiles("OptionsMenu.HuD");
	fileParser.InitiateTextures();

	fullscreen = CoreInit::subSystems.optionsHandler->GetOptionBool("Window", "fullScreen", fullscreen);
	masterVol = CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Audio", "masterVolume", masterVol);
	effectVol = CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Audio", "effectVolume", effectVol);
	voiceVol = CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Audio", "voiceVolume", voiceVol);
	bakgroundVol = CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Audio", "bakgroundVolume", bakgroundVol);


	auto returnToMain = [this]()->void
	{
		this->CurrentState = State::MAIN_MENU_STATE;
	}; std::function<void()> back = returnToMain;

	auto setNewOptions = [this]()->void
	{
		CoreInit::subSystems.optionsHandler->SetOptionBool("Window", "fullScreen", fullscreen);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "masterVolume", masterVol);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "effectVolume", effectVol);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "voiceVolume", voiceVol);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "bakgroundVolume", bakgroundVol);
		CoreInit::subSystems.optionsHandler->Trigger();
	}; std::function<void()> newOptions = setNewOptions;

	auto setDefaultOptions = [this]()->void
	{
		CoreInit::subSystems.optionsHandler->SetOptionBool("Window", "fullScreen", false);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "masterVolume", 100);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "effectVolume", 50);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "voiceVolume", 60);
		CoreInit::subSystems.optionsHandler->SetOptionUnsignedInt("Audio", "bakgroundVolume", 20);
		CoreInit::subSystems.optionsHandler->Trigger();

	}; std::function<void()> defaultOptions = setDefaultOptions;


	auto setVVp1 = [this]()->void
	{
		if (voiceVol < 100)
		{
			voiceVol++;
			CoreInit::managers.textManager->SetText(buttonEnt["voiceVolumeBtn"], std::to_wstring(voiceVol));
		}
	}; std::function<void()> vVplus1 = setVVp1;
	auto setVVp10 = [this]()->void
	{
		if (voiceVol <= 90)
		{
			voiceVol += 10;
			CoreInit::managers.textManager->SetText(buttonEnt["voiceVolumeBtn"], std::to_wstring(voiceVol));
		}
	}; std::function<void()> vVplus10 = setVVp10;
	auto setVVn1 = [this]()->void
	{
		if (voiceVol > 0)
		{
			voiceVol--;
			CoreInit::managers.textManager->SetText(buttonEnt["voiceVolumeBtn"], std::to_wstring(voiceVol));
		}
	}; std::function<void()> vVminus1 = setVVn1;
	auto setVVn10 = [this]()->void
	{
		if (voiceVol >= 10)
		{
			voiceVol -= 10;
			CoreInit::managers.textManager->SetText(buttonEnt["voiceVolumeBtn"], std::to_wstring(voiceVol));
		}
	}; std::function<void()> vVminus10 = setVVn10;
	auto setBVp1 = [this]()->void
	{
		if (bakgroundVol < 100)
		{
			bakgroundVol++;
			CoreInit::managers.textManager->SetText(buttonEnt["backgroundVolumeBtn"], std::to_wstring(bakgroundVol));
		}
	}; std::function<void()> bVplus1 = setBVp1;
	auto setBVp10 = [this]()->void
	{
		if (bakgroundVol <= 90)
		{
			bakgroundVol += 10;
			CoreInit::managers.textManager->SetText(buttonEnt["backgroundVolumeBtn"], std::to_wstring(bakgroundVol));
		}
	}; std::function<void()> bVplus10 = setBVp10;
	auto setBVn1 = [this]()->void
	{
		if (bakgroundVol > 0)
		{
			bakgroundVol--;
			CoreInit::managers.textManager->SetText(buttonEnt["backgroundVolumeBtn"], std::to_wstring(bakgroundVol));
		}
	}; std::function<void()> bVminus1 = setBVn1;
	auto setBVn10 = [this]()->void
	{
		if (bakgroundVol >= 10)
		{
			bakgroundVol -= 10;
			CoreInit::managers.textManager->SetText(buttonEnt["backgroundVolumeBtn"], std::to_wstring(bakgroundVol));
		}
	}; std::function<void()> bVminus10 = setBVn10;
	auto setEVp1 = [this]()->void
	{
		if (effectVol < 100)
		{
			effectVol++;
			CoreInit::managers.textManager->SetText(buttonEnt["EffectVolumeBtn"], std::to_wstring(effectVol));
		}
	}; std::function<void()> eVplus1 = setEVp1;
	auto setEVp10 = [this]()->void
	{
		if (effectVol <= 90)
		{
			effectVol += 10;
			CoreInit::managers.textManager->SetText(buttonEnt["EffectVolumeBtn"], std::to_wstring(effectVol));
		}
	}; std::function<void()> eVplus10 = setEVp10;
	auto setEVn1 = [this]()->void
	{
		if (effectVol > 0)
		{
			effectVol--;
			CoreInit::managers.textManager->SetText(buttonEnt["EffectVolumeBtn"], std::to_wstring(effectVol));
		}
	}; std::function<void()> eVminus1 = setEVn1;
	auto setEVn10 = [this]()->void
	{
		if (effectVol >= 10)
		{
			effectVol -= 10;
			CoreInit::managers.textManager->SetText(buttonEnt["EffectVolumeBtn"], std::to_wstring(effectVol));
		}
	}; std::function<void()> eVminus10 = setEVn10;
	auto setMVp1 = [this]()->void
	{
		if (masterVol < 100)
		{
			masterVol++;
			CoreInit::managers.textManager->SetText(buttonEnt["masterVolumeBtn"], std::to_wstring(masterVol));
		}
	}; std::function<void()> mVplus1 = setMVp1;
	auto setMVp10 = [this]()->void
	{
		if (masterVol <= 90)
		{
			masterVol += 10;
			CoreInit::managers.textManager->SetText(buttonEnt["masterVolumeBtn"], std::to_wstring(masterVol));
		}
	}; std::function<void()> mVplus10 = setMVp10;
	auto setMVn1 = [this]()->void
	{
		if (masterVol > 0)
		{
			masterVol--;
			CoreInit::managers.textManager->SetText(buttonEnt["masterVolumeBtn"], std::to_wstring(masterVol));
		}
	}; std::function<void()> mVminus1 = setMVn1;
	auto setMVn10 = [this]()->void
	{
		if (masterVol >= 10)
		{
			masterVol -= 10;
			CoreInit::managers.textManager->SetText(buttonEnt["masterVolumeBtn"], std::to_wstring(masterVol));
		}
	}; std::function<void()> mVminus10 = setMVn10;

	auto setFull = [this]()->void
	{
		if (fullscreen == false)
		{
			CoreInit::managers.textManager->SetText(buttonEnt["fullScreenBtn"], L"TRUE");
			fullscreen = true;
		}
		else
		{
			CoreInit::managers.textManager->SetText(buttonEnt["fullScreenBtn"], L"FALSE");
			fullscreen = false;
		}
	}; std::function<void()> setFullScreen = setFull;

	auto doNothing = [this]()->void
	{
		
	}; std::function<void()> nothing = doNothing;

	for (auto& button : fileParser.ButtonVector)
	{
		button.PositionY += 20;
		if (button.rectName == "voiceVolumeBtnUpOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, vVplus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "voiceVolumeBtnDownOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, vVminus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "voiceVolumeBtnUpTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, vVplus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "voiceVolumeBtnDownTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, vVminus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "backGroundVolumeBtnUpOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, bVplus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "backGroundVolumeBtnDownOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, bVminus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "backGroundVolumeBtnUpTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, bVplus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "backGroundVolumeBtnDownTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, bVminus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "EffectVolumeBtnUpOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, eVplus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "EffectVolumeBtnDownOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, eVminus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "EffectVolumeBtnUpTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, eVplus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "EffectVolumeBtnDownTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, eVminus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "masterVolumeBtnUpOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, mVplus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "masterVolumeBtnDownOne")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, mVminus1, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "masterVolumeBtnUpTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, mVplus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "masterVolumeBtnDownTen")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, mVminus10, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "BackBtn")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, back, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "DefaultBtn")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, defaultOptions, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "AcceptBtn")
		{
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, newOptions, button.textName, button.hoverTex, button.PressTex);
		}
		else if (button.rectName == "voiceVolumeBtn")
		{	
			boxNames[0] = CoreInit::managers.entityManager->Create();
			SE::Core::ITextManager::CreateInfo cInfo;
			cInfo.font = "Knights.spritefont";
			cInfo.info.colour = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
			cInfo.info.effect = Graphics::Effect::NoEffect;
			cInfo.info.posX = button.PositionX + 30;
			cInfo.info.posY = button.PositionY - 38;
			cInfo.info.width = button.Width;
			cInfo.info.height = button.Height;
			cInfo.info.layerDepth = 0;
			cInfo.info.text = L"RÖSTVOLYM";
			cInfo.info.hashString = std::hash<std::wstring>()(cInfo.info.text);
			cInfo.info.anchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.screenAnchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.rotation = 0;
			cInfo.info.scale = DirectX::XMFLOAT2(0.6, 0.6);

			CoreInit::managers.textManager->Create(boxNames[0], cInfo);
			CoreInit::managers.textManager->ToggleRenderableText(boxNames[0], true);
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, nothing, button.textName, button.hoverTex, button.PressTex, std::to_string(voiceVol));
		}
		else if (button.rectName == "backgroundVolumeBtn")
		{
			boxNames[1] = CoreInit::managers.entityManager->Create();
			SE::Core::ITextManager::CreateInfo cInfo;
			cInfo.font = "Knights.spritefont";
			cInfo.info.colour = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
			cInfo.info.effect = Graphics::Effect::NoEffect;
			cInfo.info.posX = button.PositionX + 30;
			cInfo.info.posY = button.PositionY - 38;
			cInfo.info.width = button.Width;
			cInfo.info.height = button.Height;
			cInfo.info.layerDepth = 0;
			cInfo.info.text = L"MUSIKVOLYM";
			cInfo.info.hashString = std::hash<std::wstring>()(cInfo.info.text);
			cInfo.info.anchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.screenAnchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.rotation = 0;
			cInfo.info.scale = DirectX::XMFLOAT2(0.6, 0.6);

			CoreInit::managers.textManager->Create(boxNames[1], cInfo);
			CoreInit::managers.textManager->ToggleRenderableText(boxNames[1], true);
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, nothing, button.textName, button.hoverTex, button.PressTex, std::to_string(bakgroundVol));
		}
		else if (button.rectName == "EffectVolumeBtn")
		{
			boxNames[2] = CoreInit::managers.entityManager->Create();
			SE::Core::ITextManager::CreateInfo cInfo;
			cInfo.font = "Knights.spritefont";
			cInfo.info.colour = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
			cInfo.info.effect = Graphics::Effect::NoEffect;
			cInfo.info.posX = button.PositionX + 30;
			cInfo.info.posY = button.PositionY - 38;
			cInfo.info.width = button.Width;
			cInfo.info.height = button.Height;
			cInfo.info.layerDepth = 0;
			cInfo.info.text = L"EFFEKTVOLYM";
			cInfo.info.hashString = std::hash<std::wstring>()(cInfo.info.text);
			cInfo.info.anchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.screenAnchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.rotation = 0;
			cInfo.info.scale = DirectX::XMFLOAT2(0.6, 0.6);

			CoreInit::managers.textManager->Create(boxNames[2], cInfo);
			CoreInit::managers.textManager->ToggleRenderableText(boxNames[2], true);
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, nothing, button.textName, button.hoverTex, button.PressTex, std::to_string(effectVol));
		}
		else if (button.rectName == "masterVolumeBtn")
		{
			boxNames[3] = CoreInit::managers.entityManager->Create();
			SE::Core::ITextManager::CreateInfo cInfo;
			cInfo.font = "Knights.spritefont";
			cInfo.info.colour = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
			cInfo.info.effect = Graphics::Effect::NoEffect;
			cInfo.info.posX = button.PositionX + 30;
			cInfo.info.posY = button.PositionY - 38;
			cInfo.info.width = button.Width;
			cInfo.info.height = button.Height;
			cInfo.info.layerDepth = 0;
			cInfo.info.text = L"MÄSTARVOLYM";
			cInfo.info.hashString = std::hash<std::wstring>()(cInfo.info.text);
			cInfo.info.anchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.screenAnchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.rotation = 0;
			cInfo.info.scale = DirectX::XMFLOAT2(0.6, 0.6);

			CoreInit::managers.textManager->Create(boxNames[3], cInfo);
			CoreInit::managers.textManager->ToggleRenderableText(boxNames[3], true);
			fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, nothing, button.textName, button.hoverTex, button.PressTex, std::to_string(masterVol));
		}
		else if (button.rectName == "fullScreenBtn")
		{
			boxNames[4] = CoreInit::managers.entityManager->Create();
			SE::Core::ITextManager::CreateInfo cInfo;
			cInfo.font = "Knights.spritefont";
			cInfo.info.colour = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
			cInfo.info.effect = Graphics::Effect::NoEffect;
			cInfo.info.posX = button.PositionX + 30;
			cInfo.info.posY = button.PositionY - 38;
			cInfo.info.width = button.Width;
			cInfo.info.height = button.Height;
			cInfo.info.layerDepth = 0;
			cInfo.info.text = L"FULLSKÄRM";
			cInfo.info.hashString = std::hash<std::wstring>()(cInfo.info.text);
			cInfo.info.anchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.screenAnchor = DirectX::XMFLOAT2(0, 0);
			cInfo.info.rotation = 0;
			cInfo.info.scale = DirectX::XMFLOAT2(0.6, 0.6);

			CoreInit::managers.textManager->Create(boxNames[4], cInfo);
			CoreInit::managers.textManager->ToggleRenderableText(boxNames[4], true);
			if (fullscreen == false)
			{
				fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, setFullScreen, button.textName, button.hoverTex, button.PressTex, "false");
			}
			else
			{
				fileParser.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, setFullScreen, button.textName, button.hoverTex, button.PressTex, "true");
			}
		}
	}

	

	fileParser.GUIButtons.DrawButtons();

	for (auto& button : fileParser.GUIButtons.Buttons)
	{
		if (button.rectName == "voiceVolumeBtn")
		{
			buttonEnt["voiceVolumeBtn"] = button.textEntityID;
		}
		else if (button.rectName == "backgroundVolumeBtn")
		{
			buttonEnt["backgroundVolumeBtn"] = button.textEntityID;
		}
		else if (button.rectName == "EffectVolumeBtn")
		{
			buttonEnt["EffectVolumeBtn"] = button.textEntityID;
		}
		else if (button.rectName == "masterVolumeBtn")
		{
			buttonEnt["masterVolumeBtn"] = button.textEntityID;
		}
		else if (button.rectName == "fullScreenBtn")
		{
			buttonEnt["fullScreenBtn"] = button.textEntityID;
		}
	}


	ProfileReturnVoid;
}

SE::Gameplay::OptionState::~OptionState()
{
	StartProfile;
	fileParser.GUIButtons.DeleteButtons();
	ProfileReturnVoid;
}

IGameState::State SE::Gameplay::OptionState::Update(void *& passableInfo)
{
	StartProfile;
	bool pressed = CoreInit::subSystems.window->ButtonDown(uint32_t(GameInput::ACTION));
	bool released = CoreInit::subSystems.window->ButtonUp(uint32_t(GameInput::ACTION));
	int mousePosX, mousePosY;
	CoreInit::subSystems.window->GetMousePos(mousePosX, mousePosY);


	fileParser.GUIButtons.ButtonHover(mousePosX, mousePosY, pressed, released);
	ProfileReturn(CurrentState);
}
