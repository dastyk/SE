#include "PlayState.h"
#include <Profiler.h>
#include <Utilz\Tools.h>
#include "CoreInit.h"
#include "EnemyFactory.h"
#include <GameBlackboard.h>

#ifdef _DEBUG
#pragma comment(lib, "UtilzD.lib")
#else
#pragma comment(lib, "Utilz.lib")
#endif

using namespace SE;
using namespace Gameplay;
using namespace DirectX;
PlayState::PlayState()
{

}

PlayState::PlayState(Window::IWindow* Input, SE::Core::IEngine* engine, void* passedInfo)
{
	StartProfile;
	this->input = Input;
	this->engine = engine;
	playStateGUI.ParseFiles("PlayStateGui.HuD");
	playStateGUI.InitiateTextures();
	int tempPos = 0;
	for (auto& button : playStateGUI.ButtonVector)
	{
		if (button.rectName == "HealthBar")
		{
			// here's the health bar.
			playStateGUI.GUIButtons.CreateButton(button.PositionX, button.PositionY, button.Width, button.Height, button.layerDepth, button.rectName, NULL, button.textName, button.hoverTex, button.PressTex);
			healthBarPos = tempPos;
		}

		tempPos++;
	}
	playStateGUI.GUIButtons.CreateButton(50,650,50, 50,0,"tempButton",NULL);
	playStateGUI.GUIButtons.CreateButton(125, 650, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.CreateButton(200, 650, 50, 50, 0, "tempButton", NULL);

	playStateGUI.GUIButtons.CreateButton(930, 650, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.CreateButton(1030, 650, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.CreateButton(1130, 650, 50, 50, 0, "tempButton", NULL);

	playStateGUI.GUIButtons.CreateButton(930, 575, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.CreateButton(1030, 575, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.CreateButton(1130, 575, 50, 50, 0, "tempButton", NULL);
	playStateGUI.GUIButtons.DrawButtons();

	InitializeFogDependencies();
	InitializeRooms();
	InitializePlayer(passedInfo);
	InitializeEnemies();
	InitializeOther();

	/* Play sounds */
	currentSound = 0u;
	sounds[0] = Utilz::GUID("BLoop.wav");
	sounds[1] = Utilz::GUID("BLoop2.wav");
	sounds[2] = Utilz::GUID("BLoop3.wav");

	soundEnt = CoreInit::managers.entityManager->Create();
	for (int i = 0; i < 3; ++i)
	{
		CoreInit::managers.audioManager->Create(soundEnt, { sounds[i], SE::Audio::BakgroundLoopSound });
	}
	CoreInit::managers.audioManager->PlaySound(soundEnt, sounds[0]);

	BehaviourPointers temp;
	temp.currentRoom = &currentRoom;
	temp.player = player;
	
	projectileManager = new ProjectileManager(temp);


	InitWeaponPickups();
	ProfileReturnVoid;
}

PlayState::~PlayState()
{
	StartProfile;
	delete projectileManager;
	delete player;
	//delete currentRoom;
	for (auto room : rooms)
		delete room;
	ProfileReturnVoid;
}

void PlayState::UpdateInput(PlayerUnit::MovementInput &movement, PlayerUnit::ActionInput &action)
{
	StartProfile;
	if (input->ButtonDown(uint32_t(GameInput::UP)))
	{
		movement.upButton = true;
	}
	if (input->ButtonDown(uint32_t(GameInput::DOWN)))
	{
		movement.downButton = true;
	}
	if (input->ButtonDown(uint32_t(GameInput::LEFT)))
	{
		movement.leftButton = true;
	}
	if (input->ButtonDown(uint32_t(GameInput::RIGHT)))
	{
		movement.rightButton = true;
	}

	int mX, mY;
	input->GetMousePos(mX, mY);

	DirectX::XMVECTOR rayO = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMVECTOR rayD;


	auto width = CoreInit::subSystems.optionsHandler->GetOptionInt("Window", "width", 800);
	auto height = CoreInit::subSystems.optionsHandler->GetOptionInt("Window", "height", 640);
	CoreInit::managers.cameraManager->WorldSpaceRayFromScreenPos(mX, mY, width, height, rayO, rayD);

	float distance = DirectX::XMVectorGetY(rayO) / -XMVectorGetY(rayD);

	auto clickPos = rayO + rayD*distance;

	movement.mousePosX = DirectX::XMVectorGetX(clickPos);
	movement.mousePosY = DirectX::XMVectorGetZ(clickPos);


	if (input->ButtonPressed(uint32_t(GameInput::SKILL1)))
	{
		action.skill1Button = true;
	}

	if (input->ButtonPressed(uint32_t(GameInput::SKILL2)))
	{
		action.skill2Button = true;
	}

	if (input->ButtonDown(uint32_t(GameInput::ACTION)))
	{
		action.actionButton = true;
	}
	ProfileReturnVoid;
}

void SE::Gameplay::PlayState::UpdateProjectiles(std::vector<ProjectileData>& newProjectiles)
{
	projectileManager->AddProjectiles(newProjectiles);

	projectileManager->UpdateProjectilePositions(input->GetDelta());
	currentRoom->CheckProjectileCollision(projectileManager->GetAllProjectiles());
	projectileManager->UpdateProjectileActions(input->GetDelta());


}

void SE::Gameplay::PlayState::CheckForRoomTransition()
{
	if (input->ButtonPressed(uint32_t(GameInput::INTERACT)))
	{
		SE::Gameplay::Room::DirectionToAdjacentRoom dir = currentRoom->CheckForTransition(player->GetXPosition(), player->GetYPosition());

		if (dir != SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NONE)
		{
			currentRoom->RenderRoom(false);

			if (dir == SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_SOUTH)
			{
				currentRoom = rooms[currentRoomIndex + sqrt(rooms.size())];
				currentRoomIndex = currentRoomIndex + sqrt(rooms.size());
				float xToSet, yToSet;
				xToSet = yToSet = -999999;
				currentRoom->GetPositionOfActiveDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NORTH, xToSet, yToSet);
				player->PositionEntity(xToSet, yToSet - 1);
			}
			else if (dir == SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NORTH)
			{
				currentRoom = rooms[currentRoomIndex - sqrt(rooms.size())];
				currentRoomIndex = currentRoomIndex - sqrt(rooms.size());
				float xToSet, yToSet;
				xToSet = yToSet = -999999;
				currentRoom->GetPositionOfActiveDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_SOUTH, xToSet, yToSet);
				player->PositionEntity(xToSet, yToSet + 1);
			}
			else if (dir == SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_WEST)
			{
				currentRoom = rooms[currentRoomIndex - 1];
				currentRoomIndex = currentRoomIndex - 1;
				float xToSet, yToSet;
				xToSet = yToSet = -999999;
				currentRoom->GetPositionOfActiveDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_EAST, xToSet, yToSet);
				player->PositionEntity(xToSet + 1, yToSet);
			}
			else if (dir == SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_EAST)
			{
				currentRoom = rooms[currentRoomIndex + 1];
				currentRoomIndex = currentRoomIndex + 1;
				float xToSet, yToSet;
				xToSet = yToSet = -999999;
				currentRoom->GetPositionOfActiveDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_WEST, xToSet, yToSet);
				player->PositionEntity(xToSet - 1, yToSet);
			}

			currentRoom->RenderRoom(true);
			projectileManager->RemoveAllProjectiles();

			char newMap[25][25];
			currentRoom->GetMap(newMap);

			char** tempPtr = new char*[25];

			for (int i = 0; i < 25; i++)
			{
				tempPtr[i] = new char[25];
				for (int j = 0; j < 25; j++)
				{
					tempPtr[i][j] = newMap[i][j];
				}
			}

			player->UpdateMap(tempPtr);
			currentRoom->InitializeAdjacentFlowFields();
			for (int i = 0; i < 25; i++)
			{
				delete tempPtr[i];
			}

			delete tempPtr;

			/**
			*	Must be put in change room once the function is done!
			*/
			blackBoard.currentRoom = currentRoom;
			blackBoard.roomFlowField = currentRoom->GetFlowFieldMap();
		}
	}
}

void SE::Gameplay::PlayState::UpdateHUD(float dt)
{
	CoreInit::managers.guiManager->SetTextureDimensions(playStateGUI.GUIButtons.ButtonEntityVec[healthBarPos], playStateGUI.GUIButtons.Buttons[healthBarPos].Width * player->GetHealth() / player->GetMaxHealth(), playStateGUI.GUIButtons.Buttons[healthBarPos].Height);
}

void PlayState::InitializeRooms()
{
	StartProfile;
	uint32_t nrOfRooms = 0;
	Utilz::GUID* RoomArr;
	auto subSystem = engine->GetSubsystems();
	int sideLength = 3;
	int nrOfRoomsToCreate = sideLength * sideLength;
	int nrOfRoomsCreated = 0;
	

	subSystem.resourceHandler->LoadResource("RoomGeneration.txt", [&nrOfRooms, &RoomArr](auto GUID, auto data, auto size)
	{
		nrOfRooms = *(uint32_t *)data;
		RoomArr = new Utilz::GUID[nrOfRooms];
		memcpy(RoomArr, (char*)data + sizeof(uint32_t), sizeof(Utilz::GUID) * nrOfRooms);
		return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM;
	});

	while (nrOfRoomsCreated < nrOfRoomsToCreate)
	{
		int random = CoreInit::subSystems.window->GetRand() % nrOfRooms;
		
		Gameplay::Room* temp = new Gameplay::Room(RoomArr[random]);

		rooms.push_back(temp);
		nrOfRoomsCreated++;
		temp->RenderRoom(false);
	}

	for (int i = 0; i < nrOfRoomsToCreate; i++)
	{
		if (i < sideLength) // top row
		{
			rooms[i]->CloseDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NORTH);
		}
		else
		{
			rooms[i]->AddAdjacentRoomByDirection(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NORTH, rooms[i - sideLength]);
		}

		if (i % sideLength == sideLength - 1) // right side
		{
			rooms[i]->CloseDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_EAST);
		}
		else
		{
			rooms[i]->AddAdjacentRoomByDirection(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_EAST, rooms[i + 1]);
		}

		if (i % sideLength == 0) // left side
		{
			rooms[i]->CloseDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_WEST);
		}
		else
		{
			rooms[i]->AddAdjacentRoomByDirection(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_WEST, rooms[i - 1]);
		}

		if (i >= sideLength * (sideLength - 1)) // bottom row
		{
			rooms[i]->CloseDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_SOUTH);
		}
		else
		{
			rooms[i]->AddAdjacentRoomByDirection(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_SOUTH, rooms[i + sideLength]);
		}



	}

	currentRoomIndex = 0;
	blackBoard.currentRoom = currentRoom = rooms[0];
	blackBoard.roomFlowField = currentRoom->GetFlowFieldMap();
	currentRoom->RenderRoom(true);
	currentRoom->InitializeAdjacentFlowFields();
	delete[] RoomArr;
	ProfileReturnVoid;
}

void SE::Gameplay::PlayState::InitializeFogDependencies()
{
	auto subSystem = engine->GetSubsystems();


	ResourceHandler::Callbacks vertexShaderCallbacks;
	vertexShaderCallbacks.loadCallback = [subSystem](auto guid, auto data, auto size, auto udata, auto usize)
	{
		*usize = size;
		int res = subSystem.renderer->GetPipelineHandler()->CreateVertexShader(guid, data, size);
		if (res < 0)
			return ResourceHandler::LoadReturn::FAIL;
		return ResourceHandler::LoadReturn::SUCCESS;
	};
	vertexShaderCallbacks.invokeCallback = [](auto guid, auto data, auto size) {
		return ResourceHandler::InvokeReturn::SUCCESS;
	};
	vertexShaderCallbacks.destroyCallback = [](auto guid, auto data, auto size) {

	};
	int res = subSystem.resourceHandler->LoadResource("FogVS.hlsl", vertexShaderCallbacks, ResourceHandler::LoadFlags::LOAD_FOR_VRAM | ResourceHandler::LoadFlags::IMMUTABLE);
	if (res)
		throw std::exception("Could not load fog vertex shader");



	ResourceHandler::Callbacks pixelShaderCallbacks;
	pixelShaderCallbacks.loadCallback = [subSystem](auto guid, auto data, auto size, auto udata, auto usize)
	{
		*usize = size;
		int res = subSystem.renderer->GetPipelineHandler()->CreatePixelShader(guid, data, size);
		if (res < 0)
			return ResourceHandler::LoadReturn::FAIL;
		return ResourceHandler::LoadReturn::SUCCESS;
	};
	pixelShaderCallbacks.invokeCallback = [](auto guid, auto data, auto size) {
		return ResourceHandler::InvokeReturn::SUCCESS;
	};
	pixelShaderCallbacks.destroyCallback = [](auto guid, auto data, auto size) {

	};
	res = subSystem.resourceHandler->LoadResource("FogPS.hlsl", pixelShaderCallbacks, ResourceHandler::LoadFlags::LOAD_FOR_VRAM | ResourceHandler::LoadFlags::IMMUTABLE);
	if (res)
		throw std::exception("Could not load fog pixel shader");



	ResourceHandler::Callbacks albedoTextureCallbacks;
	albedoTextureCallbacks.loadCallback = [subSystem](auto guid, auto data, auto size, auto udata, auto usize)
	{
		Graphics::TextureDesc td;
		memcpy(&td, data, sizeof(td));
		*usize = size - sizeof(td);
		// Ensure the size of the raw pixel data is the same as the width x height x size_per_pixel
		if (td.width * td.height * 4 != size - sizeof(td))
			return ResourceHandler::LoadReturn::FAIL;

		void* rawTextureData = ((char*)data) + sizeof(td);
		auto result = subSystem.renderer->GetPipelineHandler()->CreateTexture(guid, rawTextureData, td.width, td.height);
		if (result < 0)
			return ResourceHandler::LoadReturn::FAIL;
		return ResourceHandler::LoadReturn::SUCCESS;
	};

	albedoTextureCallbacks.invokeCallback = [](auto guid, auto data, auto size) {
		return ResourceHandler::InvokeReturn::SUCCESS;
	};
	albedoTextureCallbacks.destroyCallback = [subSystem](auto guid, auto data, auto size) {
		subSystem.renderer->GetPipelineHandler()->DestroyTexture(guid);
	};

	res = subSystem.resourceHandler->LoadResource("Fog_AlbedoTexture.png", albedoTextureCallbacks, ResourceHandler::LoadFlags::LOAD_FOR_VRAM | ResourceHandler::LoadFlags::IMMUTABLE);
	if (res)
		throw std::exception("Could not load fog albedo texture");


	ResourceHandler::Callbacks normalTextureCallbacks;
	normalTextureCallbacks.loadCallback = [subSystem](auto guid, auto data, auto size, auto udata, auto usize)
	{
		Graphics::TextureDesc td;
		memcpy(&td, data, sizeof(td));
		*usize = size - sizeof(td);
		// Ensure the size of the raw pixel data is the same as the width x height x size_per_pixel
		if (td.width * td.height * 4 != size - sizeof(td))
			return ResourceHandler::LoadReturn::FAIL;

		void* rawTextureData = ((char*)data) + sizeof(td);
		auto result = subSystem.renderer->GetPipelineHandler()->CreateTexture(guid, rawTextureData, td.width, td.height);
		if (result < 0)
			return ResourceHandler::LoadReturn::FAIL;
		return ResourceHandler::LoadReturn::SUCCESS;
	};

	normalTextureCallbacks.invokeCallback = [](auto guid, auto data, auto size) {
		return ResourceHandler::InvokeReturn::SUCCESS;
	};
	normalTextureCallbacks.destroyCallback = [subSystem](auto guid, auto data, auto size) {
		subSystem.renderer->GetPipelineHandler()->DestroyTexture(guid);
	};

	res = subSystem.resourceHandler->LoadResource("Fog_NormalTexture.png", normalTextureCallbacks, ResourceHandler::LoadFlags::LOAD_FOR_VRAM | ResourceHandler::LoadFlags::IMMUTABLE);
	if (res)
		throw std::exception("Could not load fog normal texture");



	Graphics::RenderTarget fogRenderTarget;

	fogRenderTarget.clearColor[0] = 0; fogRenderTarget.clearColor[1] = 0; fogRenderTarget.clearColor[2] = 0; fogRenderTarget.clearColor[3] = 1;
	fogRenderTarget.format = Graphics::TextureFormat::R8G8B8A8_UNORM;
	fogRenderTarget.width = subSystem.optionsHandler->GetOptionUnsignedInt("Window", "width", 1280);
	fogRenderTarget.height = subSystem.optionsHandler->GetOptionUnsignedInt("Window", "height", 720);
	fogRenderTarget.bindAsShaderResource = true;

	subSystem.renderer->GetPipelineHandler()->CreateRenderTarget("FogRt", fogRenderTarget);



	Graphics::SamplerState fogSampler;

	fogSampler.addressU = Graphics::AddressingMode::WRAP;
	fogSampler.addressV = Graphics::AddressingMode::WRAP;
	fogSampler.addressW = Graphics::AddressingMode::WRAP;

	fogSampler.filter = Graphics::Filter::ANISOTROPIC;
	fogSampler.maxAnisotropy = 4;

	subSystem.renderer->GetPipelineHandler()->CreateSamplerState("FogSampler", fogSampler);
}
void SE::Gameplay::PlayState::InitializeEnemies()
{
	StartProfile;
	char map[25][25];

	EnemyCreationStruct eStruct;
	EnemyUnit** enemies = new EnemyUnit*[enemiesInEachRoom];
	for(auto& room : rooms)
	{
		room->GetMap(map);
		eStruct.information.clear();

		for (int i = 0; i < enemiesInEachRoom; i++)
		{
			pos enemyPos;
			do
			{
				enemyPos.x = CoreInit::subSystems.window->GetRand() % 25;
				enemyPos.y = CoreInit::subSystems.window->GetRand() % 25;
			} while (map[int(enemyPos.x)][int(enemyPos.y)]);

			EnemyCreationData data;
			data.type = ENEMY_TYPE_RANDOM;
			data.startX = enemyPos.x;
			data.startY = enemyPos.y;
			data.useVariation = true;
			eStruct.information.push_back(data);
		}
		
		eFactory.CreateEnemies(eStruct, &blackBoard, enemies);

		for (int i = 0; i < enemiesInEachRoom; i++)
		{
			room->AddEnemyToRoom(enemies[i]);
		}

	}
	delete[] enemies;
	ProfileReturnVoid;
}

void PlayState::InitializePlayer(void* playerInfo)
{
	StartProfile;
	char map[25][25];
	currentRoom->GetMap(map);
	PlayStateData* tempPtr = (PlayStateData*)playerInfo;

	for (int x = 0; x < 25; x++)
	{
		for (int y = 0; y < 25; y++)
		{
			if (map[x][y] == (char)22)
			{
				float rotation = ceilf((currentRoom->FloorCheck(x, y) * (180 / 3.1416) - 270) - 0.5f);
				int xOffset = 0, yOffset = 0;
				if (rotation == 0)
				{
					yOffset = 1;
				}
				else if (rotation == 90)
				{
					xOffset = 1;
				}
				else if (rotation == 180)
				{
					yOffset = -1;
				}
				else if (rotation == 270)
				{
					xOffset = -1;
				}
				player = new Gameplay::PlayerUnit(tempPtr->skills, nullptr, x + (0.5f + xOffset), y + (0.5f + yOffset), map);

				player->SetZPosition(0.9f);
				player->PositionEntity(x + (0.5f + xOffset), y + (0.5f + yOffset));
				break;
			}
		}
	}
	ProfileReturnVoid;
}

void SE::Gameplay::PlayState::InitializeOther()
{
	StartProfile;
	//Setup camera to the correct perspective and bind it to the players position
	Core::ICameraManager::CreateInfo cInfo;
	cInfo.aspectRatio = (float)CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Window", "width", 800) / (float)CoreInit::subSystems.optionsHandler->GetOptionUnsignedInt("Window", "height", 640);
	cam = CoreInit::managers.cameraManager->GetActive();
	CoreInit::managers.cameraManager->UpdateCamera(cam, cInfo);

	float cameraRotationX = DirectX::XM_PI / 3;
	float cameraRotationY = DirectX::XM_PI / 3;

	auto cameraRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(cameraRotationX, cameraRotationY, 0);

	auto cameraTranslation = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), cameraRotationMatrix);

	player->UpdatePlayerRotation(cameraRotationX, cameraRotationY);
	CoreInit::managers.transformManager->BindChild(player->GetEntity(), cam, false, true);
	CoreInit::managers.transformManager->Move(cam, -5 * cameraTranslation);
	CoreInit::managers.transformManager->SetRotation(cam, cameraRotationX, cameraRotationY, 0);

	//Create a default light

	dummy = CoreInit::managers.entityManager->Create();
	CoreInit::managers.transformManager->Create(dummy, { 0.0f, 2.05f, 0.0f });
	CoreInit::managers.transformManager->BindChild(player->GetEntity(), dummy, false, true);
	//CoreInit::managers.renderableManager->CreateRenderableObject(dummy, { "Placeholder_Block.mesh" });
	//CoreInit::managers.renderableManager->ToggleRenderableObject(dummy, true);

	SE::Core::ILightManager::CreateInfo lightInfo;
	lightInfo.pos = { 0.0f, 0.0f, 0.0f };
	lightInfo.color = { 0.74f, 0.92f, 0.95f };
	lightInfo.radius = 15.0f;

	CoreInit::managers.lightManager->Create(dummy, lightInfo);
	CoreInit::managers.lightManager->ToggleLight(dummy, true);
	CoreInit::managers.lightManager->SetShadowCaster(dummy);
	ProfileReturnVoid;
}
#include <Items.h>

void SE::Gameplay::PlayState::InitWeaponPickups()
{
	StartProfile;
	auto pe = player->GetEntity();

	Core::IEventManager::EventCallbacks pickUpEvent;
	pickUpEvent.triggerCallback = [this](const Core::Entity ent, void* data) {

		CoreInit::subSystems.devConsole->Print("Picked up item %s.", std::get<std::string>(CoreInit::managers.dataManager->GetValue(ent, "Name", "NaN"s)).c_str());

		if (CoreInit::subSystems.window->ButtonDouble(GameInput::ONE))
		{
			player->AddItem(ent, 0);
		}
		else if (CoreInit::subSystems.window->ButtonDouble(GameInput::TWO))
		{
			player->AddItem(ent, 1);
		}
		else if (CoreInit::subSystems.window->ButtonDouble(GameInput::THREE))
		{
			player->AddItem(ent, 2);
		}
		else if (CoreInit::subSystems.window->ButtonDouble(GameInput::FOUR))
		{
			player->AddItem(ent, 3);
		}
		else if (CoreInit::subSystems.window->ButtonDouble(GameInput::FIVE))
		{
			player->AddItem(ent, 4);
		}
	};

	pickUpEvent.triggerCheck = [pe](const Core::Entity ent, void* data) {
		if (CoreInit::subSystems.window->ButtonDouble(GameInput::PICKUP))
		{
			return CoreInit::managers.collisionManager->CheckCollision(ent, pe);
		}
		return false;
	};

	Core::IEventManager::EventCallbacks startrenderWIC;
	startrenderWIC.triggerCheck = [pe](const Core::Entity ent, void* data)
	{
		auto vis = std::get<bool>(CoreInit::managers.dataManager->GetValue(pe, "WICV", false));
		if (vis && !CoreInit::subSystems.window->ButtonPressed(GameInput::PICKUP))
			return false;
		if (!CoreInit::subSystems.window->ButtonDown(GameInput::SHOWINFO))
			return false;
		return CoreInit::managers.collisionManager->CheckCollision(ent, pe);
	};
	
	startrenderWIC.triggerCallback = [pe, this](const Core::Entity ent, void*data)
	{
		CoreInit::managers.dataManager->SetValue(pe, "WICV", true);
		Item::ToggleRenderPickupInfo(ent);
	};

	Core::IEventManager::EventCallbacks stoprenderWIC;
	stoprenderWIC.triggerCheck = [pe](const Core::Entity ent, void* data)
	{
		if (auto parent = std::get_if<Core::Entity>(&CoreInit::managers.dataManager->GetValue(ent, "Parent", false)))
		{
			if (!CoreInit::managers.collisionManager->CheckCollision(*parent, pe)) {
				return true;
			}	
		}

		return (!CoreInit::subSystems.window->ButtonDown(GameInput::SHOWINFO)) || CoreInit::subSystems.window->ButtonPressed(GameInput::PICKUP);
	};

	stoprenderWIC.triggerCallback = [pe](const Core::Entity ent, void*data)
	{
		CoreInit::managers.entityManager->DestroyNow(ent);	
		auto parent = std::get<Core::Entity>(CoreInit::managers.dataManager->GetValue(ent, "Parent", Core::Entity()));
		CoreInit::managers.dataManager->SetValue(pe, "WICV", false);
	};
	CoreInit::managers.eventManager->RegisterEventCallback("StartRenderWIC", startrenderWIC);
	CoreInit::managers.eventManager->RegisterEventCallback("StopRenderWIC", stoprenderWIC);
	CoreInit::managers.eventManager->RegisterEventCallback("WeaponPickUp", pickUpEvent);
	ProfileReturnVoid;
}

IGameState::State PlayState::Update(void*& passableInfo)
{
	StartProfile;
	rooms[0]->CloseDoor(SE::Gameplay::Room::DirectionToAdjacentRoom::DIRECTION_ADJACENT_ROOM_NORTH);
	IGameState::State returnValue = State::PLAY_STATE;
	PlayerUnit::MovementInput movementInput(false, false, false, false, false, 0.0f, 0.0f);
	PlayerUnit::ActionInput actionInput(false, false);
	std::vector<ProjectileData> newProjectiles;

	UpdateInput(movementInput, actionInput);

	projectileManager->CheckCollisionBetweenUnitAndProjectiles(player, Gameplay::ValidTarget::PLAYER);
	player->Update(input->GetDelta(), movementInput, newProjectiles, actionInput);

	UpdateProjectiles(newProjectiles);

	blackBoard.playerPositionX = player->GetXPosition();
	blackBoard.playerPositionY = player->GetYPosition();
	blackBoard.deltaTime = input->GetDelta();
	blackBoard.playerHealth = player->GetHealth();
	
	currentRoom->Update(input->GetDelta(), player->GetXPosition(), player->GetYPosition());

	projectileManager->AddProjectiles(blackBoard.enemyProjectiles);
	blackBoard.enemyProjectiles.clear();

	//-----sound update
	soundTime += input->GetDelta();
	if (soundTime > 60.0f)
	{
		uint8_t newSound = input->GetRand() % 3;
		if (currentSound != newSound)
		{
			CoreInit::managers.audioManager->StopSound(soundEnt, sounds[currentSound]);
			CoreInit::managers.audioManager->PlaySound(soundEnt, sounds[newSound]);
			currentSound = newSound;
		}
		soundTime = 0.0f;
	}
	//-----end sound update
	CheckForRoomTransition();
	UpdateHUD(input->GetDelta());

	if (!player->IsAlive())
		returnValue = State::GAME_OVER_STATE;

	ProfileReturn(returnValue);

}

