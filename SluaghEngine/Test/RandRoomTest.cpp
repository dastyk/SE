#include "RandRoomTest.h"
#include <Core/IEngine.h>
#include <Gameplay/EnemyUnit.h>
#include <Gameplay/Room.h>
#include <Gameplay/ProjectileData.h>
#include <Gameplay/ProjectileManager.h>
#include <Gameplay/Game.h>
#include <chrono>
#include <Gameplay/PlayerUnit.h>
#include <Profiler.h>
#include <Utilz\Tools.h>
#include "Gameplay/GameBlackboard.h"
#include "Gameplay/EnemyFactory.h"

#ifdef _DEBUG
#pragma comment(lib, "coreD.lib")
#else
#pragma comment(lib, "core.lib")
#endif
#ifdef _DEBUG
#pragma comment(lib, "gameplayD.lib")
#else
#pragma comment(lib, "gameplay.lib")
#endif



SE::Test::RandRoomTest::RandRoomTest()
{

}

SE::Test::RandRoomTest::~RandRoomTest()
{

}

bool SE::Test::RandRoomTest::Run(DevConsole::IConsole* console)
{
	StartProfile;
	using namespace DirectX;
	auto engine = Core::CreateEngine();
	Gameplay::Game game;
	engine->Init();
	game.Initiate(engine);
	auto managers = engine->GetManagers();
	auto subSystem = engine->GetSubsystems();
	

	//srand((int)time(NULL)); 

	float width = subSystem.optionsHandler->GetOptionUnsignedInt("Window", "width", 800);
	float height = subSystem.optionsHandler->GetOptionUnsignedInt("Window", "height", 600);

	auto floor = managers.entityManager->Create();
	const int numberOfBlocks = 25 * 25;
	SE::Core::Entity entities[numberOfBlocks];
	SE::Core::Entity arrows[numberOfBlocks];

	for (int i = 0; i < numberOfBlocks; i++)
	{
		entities[i] = managers.entityManager->Create();
		arrows[i] = managers.entityManager->Create();
	}
	 managers.transformManager->Create(floor);
	 managers.transformManager->SetPosition(floor, DirectX::XMFLOAT3(12.5f, 0.0f, 12.5f));

	for (int i = 0; i < numberOfBlocks; i++)
	{
		 managers.transformManager->Create(entities[i]);
		 managers.transformManager->Create(arrows[i]);
	}

	managers.renderableManager->CreateRenderableObject(floor, {"Placeholder_Floor.mesh"});
	managers.renderableManager->ToggleRenderableObject(floor, true);

	managers.collisionManager->CreateBoundingHierarchy(floor, {"Placeholder_Floor.mesh"});


	auto Block = SE::Utilz::GUID{"HighWall.mesh"};
	auto Arrow = SE::Utilz::GUID{"Placeholder_Arrow.mesh"};
	auto Door = SE::Utilz::GUID{"Door.mesh"};

	int numberOfEntitesPlaced = 0;
	int numberOfArrows = 0;
	uint32_t nrOfRooms;
	Utilz::GUID* RoomArr;

	subSystem.resourceHandler->LoadResource("RoomGeneration.txt", [&nrOfRooms, &RoomArr](auto GUID, auto data, auto size)
	{
		nrOfRooms = *(uint32_t *)data;
		RoomArr = new Utilz::GUID[nrOfRooms];
		memcpy(RoomArr, (char*)data + sizeof(uint32_t), sizeof(Utilz::GUID) * nrOfRooms);
		return ResourceHandler::InvokeReturn::SUCCESS | ResourceHandler::InvokeReturn::DEC_RAM;
	});
	
	int random = subSystem.window->GetRand() % nrOfRooms;

	Gameplay::Room* testRoom = new Gameplay::Room(RoomArr[random]);

	Gameplay::PlayerUnit* player = nullptr;
	
	if (player == nullptr)
	{
		delete testRoom;
		delete player;
		delete[] RoomArr;
		game.Shutdown();
		engine->Release();
		delete engine;
		ProfileReturnConst(false);
	}

	 managers.renderableManager->CreateRenderableObject(player->GetEntity(), {"MCModell.mesh"});

	 managers.renderableManager->ToggleRenderableObject(player->GetEntity(), true);
	 managers.transformManager->SetRotation(player->GetEntity(), 0, 0, 0);

	SE::Core::Entity camera =managers.entityManager->Create();

	Core::ICameraManager::CreateInfo cInfo;
	cInfo.aspectRatio = (float)subSystem.optionsHandler->GetOptionUnsignedInt("Window", "width", 800) / (float)subSystem.optionsHandler->GetOptionUnsignedInt("Window", "height", 640);
	
	managers.cameraManager->Create(camera, cInfo);
	managers.cameraManager->SetActive(camera);

	float cameraRotationX = DirectX::XM_PI / 3;
	float cameraRotationY = DirectX::XM_PI / 3;

	auto cameraRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(cameraRotationX, cameraRotationY, 0);

	auto cameraTranslation = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), cameraRotationMatrix);

	player->UpdatePlayerRotation(cameraRotationX, cameraRotationY);
	managers.transformManager->BindChild(player->GetEntity(), camera, false, true);
	auto cameraPos = managers.transformManager->GetPosition(camera);
	managers.transformManager->Move(camera, -5 * cameraTranslation);
	managers.transformManager->SetRotation(camera, cameraRotationX, cameraRotationY, 0);

	Gameplay::BehaviourPointers initBP;

	initBP.player = player;
	initBP.currentRoom = &testRoom;

	SE::Gameplay::ProjectileManager* projectileManager = new SE::Gameplay::ProjectileManager(initBP);

	char map[25][25];
	testRoom->GetMap(map);

	for (int x = 0; x < 25; x++)
	{
		for (int y = 0; y < 25; y++)
		{
			if (map[x][y] == 10)
			{
				managers.renderableManager->CreateRenderableObject(entities[numberOfEntitesPlaced], { Block });
				 managers.renderableManager->ToggleRenderableObject(entities[numberOfEntitesPlaced], true);
				 managers.transformManager->SetPosition(entities[numberOfEntitesPlaced], DirectX::XMFLOAT3(x + 0.5f, 0.5f, y + 0.5f));
				numberOfEntitesPlaced++;
			}
			else if (map[x][y] == 2 || map[x][y] == 1)
			{
				managers.renderableManager->CreateRenderableObject(entities[numberOfEntitesPlaced], { Door });
				 managers.renderableManager->ToggleRenderableObject(entities[numberOfEntitesPlaced], true);
				 managers.transformManager->SetPosition(entities[numberOfEntitesPlaced], DirectX::XMFLOAT3(x + 0.5f, 0.5f, y + 0.5f));

				 managers.transformManager->SetRotation(entities[numberOfEntitesPlaced], 0.0f, testRoom->FloorCheck(x,y) , 0.0f);
				numberOfEntitesPlaced++;
			}
			else if (map[x][y] == 0)
			{
				managers.renderableManager->CreateRenderableObject(arrows[numberOfArrows], { Arrow });
				 managers.renderableManager->ToggleRenderableObject(arrows[numberOfArrows], true);
				float xMagnitude = 0.0f;
				float zMagnitude = 0.0f;
				pos enemyPos;
				enemyPos.x = x + 0.5f;
				enemyPos.y = y + 0.5f;
				testRoom->GetFlowFieldMap()->SampleFromMap(enemyPos, xMagnitude, zMagnitude);
				 managers.transformManager->SetPosition(arrows[numberOfArrows], DirectX::XMFLOAT3(x + 0.5f, 0.5f, y + 0.5f));
				 managers.transformManager->SetScale(arrows[numberOfArrows], 0.25f);
				if (xMagnitude == 1.0f)
				{
					 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, -DirectX::XM_PIDIV2, 0.0f);
				}
				else if (xMagnitude == -1.0f)
				{
					 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, DirectX::XM_PIDIV2, 0.0f);
				}
				else if (zMagnitude == 1.0f)
				{
					 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, DirectX::XM_PI, 0.0f);
				}
				else if (zMagnitude == -1.0f)
				{
					 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, 0.0f, 0.0f);

				}
				else if (xMagnitude == 0.707f)
				{
					if (zMagnitude == 0.707f)
					{
						 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, DirectX::XM_PI + DirectX::XM_PIDIV4, 0.0f);
					}
					else if (zMagnitude == -0.707f)
					{
						 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, -DirectX::XM_PIDIV4, 0.0f);
					}
				}
				else if (xMagnitude == -0.707f)
				{
					if (zMagnitude == 0.707f)
					{
						 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, DirectX::XM_PI - DirectX::XM_PIDIV4, 0.0f);
					}
					else if (zMagnitude == -0.707f)
					{
						 managers.transformManager->SetRotation(arrows[numberOfArrows], 0.0f, DirectX::XM_PIDIV4, 0.0f);
					}
				}

				numberOfArrows++;
			}
		}
	}
	Gameplay::EnemyFactory eFactory;
	Gameplay::EnemyCreationStruct eStruct;
	Gameplay::GameBlackboard blackBoard;
	blackBoard.roomFlowField = testRoom->GetFlowFieldMap();

	const int enemiesSize = 100;
	Gameplay::EnemyUnit** enemies;

	enemies = new Gameplay::EnemyUnit*[enemiesSize];

	for (int i = 0; i < enemiesSize; i++)
	{
		pos enemyPos;
		do
		{
			enemyPos.x = subSystem.window->GetRand() % 25;
			enemyPos.y = subSystem.window->GetRand() % 25;
		} while (map[int(enemyPos.x)][int(enemyPos.y)]);

		Gameplay::EnemyCreationData data;
		data.type = Gameplay::EnemyType::ENEMY_TYPE_GLAISTIG;
		data.startX = enemyPos.x;
		data.startY = enemyPos.y;
		data.useVariation = true;
		eStruct.information.push_back(data);
	}



	eFactory.CreateEnemies(eStruct, &blackBoard, enemies);

	for (int i = 0; i < enemiesSize; i++)
	{
		testRoom->AddEnemyToRoom(enemies[i]);
	}

	delete[] enemies;
	subSystem.window->MapActionButton(0, Window::KeyEscape);
	subSystem.window->MapActionButton(1, Window::Key1);
	subSystem.window->MapActionButton(2, Window::Key2);

	enum MoveDir
	{
		UP = 3,
		RIGHT = 4,
		DOWN = 5,
		LEFT = 6,
		RIGHT_MOUSE = 7,
		SPACE = 8,
		CONSOLE = 9
	};


	subSystem.window->MapActionButton(UP, Window::KeyW);
	subSystem.window->MapActionButton(RIGHT, Window::KeyD);
	subSystem.window->MapActionButton(DOWN, Window::KeyS);
	subSystem.window->MapActionButton(LEFT, Window::KeyA);
	subSystem.window->MapActionButton(RIGHT_MOUSE, Window::MouseRight);
	subSystem.window->MapActionButton(SPACE, Window::KeySpace);
	subSystem.window->MapActionButton(CONSOLE, Window::Key1);

	pos playerPos;
	playerPos.x = player->GetXPosition();
	playerPos.y = player->GetYPosition();

	std::vector<Gameplay::ProjectileData> newProjectiles;

	DirectX::XMFLOAT3 tPos =  managers.transformManager->GetPosition(floor);
	DirectX::XMFLOAT3 tRot =  managers.transformManager->GetRotation(floor);
	DirectX::XMFLOAT3 tScale =  managers.transformManager->GetScale(floor);
	DirectX::XMMATRIX worldM = { tScale.x, 0, 0, 0,
		0, tScale.y, 0, 0,
		0, 0, tScale.z, 0,
		tPos.x, tPos.y, tPos.z, 1.0f };

	bool stepping = false;
	bool running = true;
	//unsigned char counter = 0;
	float dt = 1.0f / 60.0f;
	while (running)
	{
		engine->BeginFrame();
		newProjectiles.clear();

		Gameplay::PlayerUnit::MovementInput input(false, false, false, false, false, 0.0f, 0.0f);
		float movX = 0.0f;
		float movY = 0.0f;

		if (subSystem.window->ButtonDown(MoveDir::UP))
		{
			input.upButton = true;
			movY += 1.0f;
		}
		if (subSystem.window->ButtonDown(MoveDir::DOWN))
		{
			input.downButton = true;
			movY -= 1.0f;
		}
		if (subSystem.window->ButtonDown(MoveDir::RIGHT))
		{
			input.rightButton = true;
			movX += 1.0f;
		}
		if (subSystem.window->ButtonDown(MoveDir::LEFT))
		{
			input.leftButton = true;
			movX -= 1.0f;
		}
		if (subSystem.window->ButtonDown(MoveDir::RIGHT_MOUSE))
		{
			input.mouseRightDown = true;
		}
		if (subSystem.window->ButtonPressed(MoveDir::CONSOLE))
		{
			subSystem.devConsole->Toggle();
		}

		int mX = 0;
		int mY = 0;
		subSystem.window->GetMousePos(mX, mY);

		DirectX::XMVECTOR rayO = { 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMVECTOR rayD;
		managers.cameraManager->WorldSpaceRayFromScreenPos(mX, mY, width, height, rayO, rayD);
		
		float distance = XMVectorGetY(rayO) / -XMVectorGetY(rayD);
		
		auto clickPos = rayO + rayD*distance;

		input.mousePosX = DirectX::XMVectorGetX(clickPos);
		input.mousePosY = DirectX::XMVectorGetZ(clickPos);

		float totMov = abs(movX) + abs(movY);
		if (totMov != 0.f)
		{
			movX /= totMov;
			movY /= totMov;
		}

		Gameplay::PlayerUnit::ActionInput actionInput(false, false);
		if (subSystem.window->ButtonDown(MoveDir::SPACE))
		{
			actionInput.skill1Button = true;
		}
		/*Only thing needed right now*/
		blackBoard.deltaTime = dt;

		int arrowIndex = 0;
		for (int x = 0; x < 25; x++)
		{
			for (int y = 0; y < 25; y++)
			{
				if (map[x][y])
				{

				}
				else
				{

					float xMagnitude = 0.0f;
					float zMagnitude = 0.0f;
					pos enemyPos;
					enemyPos.x = x + 0.5f;
					enemyPos.y = y + 0.5f;
					testRoom->GetFlowFieldMap()->SampleFromMap(enemyPos, xMagnitude, zMagnitude);

					if (xMagnitude == 1.0f)
					{
						 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, -DirectX::XM_PIDIV2, 0.0f);
					}
					else if (xMagnitude == -1.0f)
					{
						 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, DirectX::XM_PIDIV2, 0.0f);
					}
					else if (zMagnitude == 1.0f)
					{
						 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, DirectX::XM_PI, 0.0f);
					}
					else if (zMagnitude == -1.0f)
					{
						 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, 0.0f, 0.0f);

					}
					else if (xMagnitude == 0.707f)
					{
						if (zMagnitude == 0.707f)
						{
							 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, DirectX::XM_PI + DirectX::XM_PIDIV4, 0.0f);
						}
						else if (zMagnitude == -0.707f)
						{
							 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, -DirectX::XM_PIDIV4, 0.0f);
						}
					}
					else if (xMagnitude == -0.707f)
					{
						if (zMagnitude == 0.707f)
						{
							 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, DirectX::XM_PI - DirectX::XM_PIDIV4, 0.0f);
						}
						else if (zMagnitude == -0.707f)
						{
							 managers.transformManager->SetRotation(arrows[arrowIndex], 0.0f, DirectX::XM_PIDIV4, 0.0f);
						}
					}

					arrowIndex++;
				}

			}

		}

		player->Update(dt, input, newProjectiles, actionInput);

		projectileManager->AddProjectiles(newProjectiles);

		projectileManager->UpdateProjectilePositions(dt);
		testRoom->CheckProjectileCollision(projectileManager->GetAllProjectiles());
		projectileManager->UpdateProjectileActions(dt);


		playerPos.x = player->GetXPosition();
		playerPos.y = player->GetYPosition();

		if (subSystem.window->ButtonPressed(0))
			running = false;
		if (subSystem.window->ButtonPressed(1))
		{
			stepping = !stepping;
		}

		if (stepping)
		{
			if (subSystem.window->ButtonDown(2))
			{
				testRoom->Update(dt, playerPos.x, playerPos.y);
			}
		}
		else {
			testRoom->Update(dt, playerPos.x, playerPos.y);
		}
		engine->EndFrame();


		/*	Utilz::TimeMap times;
		e.GetProfilingInformation(times);
		for (auto& t : times)
		console->Print("%s: %f\n", t.first.str, t.second);*/
	}

	delete projectileManager;

	delete testRoom;
	delete player;
	delete[] RoomArr; 

	game.Shutdown();
	engine->Release();
	delete engine;

	ProfileReturnConst(true)
}

