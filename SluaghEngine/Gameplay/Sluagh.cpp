#include "..\includes\Gameplay\Sluagh.h"
#include "ProjectileData.h"
#include <Profiler.h>
#include "CoreInit.h"
#include "SluaghRoom.h"
#include "Items.h"

#include <fstream>

SE::Gameplay::Sluagh::Sluagh(PlayerUnit * thePlayer, SluaghRoom* room)
	:thePlayer(thePlayer)
{
	char map[25][25];
	room->GetMap(map);

	this->room = room;
	theSluagh = nullptr;
	distanceToPlayer = 0.f;
}

SE::Gameplay::Sluagh::~Sluagh()
{
	delete theSluagh;
}

void SE::Gameplay::Sluagh::Update(float dt, std::vector<ProjectileData>& projectilesOut)
{
	StartProfile;
	PlayerUnit::MovementInput moveInput(false, false, false, false, false, 0.f, 0.f);
	PlayerUnit::ActionInput actionInput;
	/*Utility function calls here*/
	DecideActions(dt, moveInput, actionInput);

	/*Mouse pos = player pos => always follow the player*/
	moveInput.mousePosX = thePlayer->GetXPosition();
	moveInput.mousePosY = thePlayer->GetYPosition();


	/*Resolve player update*/
	theSluagh->Update(dt, moveInput, projectilesOut, actionInput);

	previousMovement = moveInput;
	previousAction = actionInput;

	for(auto &projectile : projectilesOut)
	{
		projectile.target = ValidTarget::PLAYER;
	}

	ProfileReturnVoid;
}

bool SE::Gameplay::Sluagh::CollisionAgainstProjectile(float projectileX, float projectileY, float projectileRadius)
{
	StartProfile;
	float xDistance = projectileX - theSluagh->GetXPosition();
	float yDistance = projectileX - theSluagh->GetYPosition();
	static const float sluaghRadius = sqrtf(0.25f*0.25f*2);
	ProfileReturn((sqrtf(xDistance*xDistance + yDistance*yDistance) < projectileRadius+sluaghRadius));
}

void SE::Gameplay::Sluagh::ToggleRendering(bool render)
{
	CoreInit::managers.eventManager->ToggleVisible(theSluagh->GetEntity(), render);
	CoreInit::managers.eventManager->ToggleVisible(theSluagh->GetCurrentItem(), render);
}

void SE::Gameplay::Sluagh::InitializeSluagh()
{
	StartProfile;
	char* appdataBuffer;
	size_t bcount;
	if (_dupenv_s(&appdataBuffer, &bcount, "APPDATA"))
		throw std::exception("Failed to retrieve path to appdata.");
	std::string appdata(appdataBuffer);
	free(appdataBuffer);
	appdata += "\\Sluagh\\";
	std::ifstream in(appdata + "sluaghFile.sluagh", std::ios::in|std::ios::binary);
	

	/*Create copy of the player*/
	char roomMap[25][25]; room->GetMap(roomMap);
	if(in.is_open())
	{
		/*Create sluagh from data*/
		theSluagh = new PlayerUnit(in, 15, 15, roomMap);
		theSluagh->ToggleAsSluagh(true);
		in.close();

	}
	else
	{
		/*Copy the current player*/
		theSluagh = new PlayerUnit(&thePlayer->GetAllSkills()[0], nullptr,nullptr, 15, 15, roomMap);
		theSluagh->ToggleAsSluagh(true);
		auto playerItems = thePlayer->GetAllItems();
		for(int i = 0; i < 5; i++)
		{
			auto item = Item::Copy(playerItems[i]);
			auto itemType = ItemType(std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Item", 4)));
			if(itemType == ItemType::WEAPON)
			{
				auto weaponType = ItemType(std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Type", 4)));
				auto newItem = Item::Weapon::Create(Item::Weapon::Type(itemType), true, 0);
				theSluagh->AddItem(newItem, i);
			}
			else
			{
				theSluagh->AddItem(item, i);
			}
		}
		

	}
	float rotX, rotY;
	thePlayer->GetRotation(rotX, rotY);
	theSluagh->UpdatePlayerRotation(rotX, rotY);
	theSluagh->SetZPosition(0.9f);
	theSluagh->PositionEntity(15.5f, 15.5f);
}

bool SE::Gameplay::Sluagh::SluaghMovedPreviousFrame()
{
	StartProfile;
	if (previousMovement.upButton == previousMovement.downButton)
	{
		if (previousMovement.leftButton == previousMovement.upButton)
		{
			ProfileReturnConst(false);
		}
		else if (previousMovement.leftButton || previousMovement.downButton)
			ProfileReturnConst(true);
	}
	ProfileReturnConst(true);
}

void SE::Gameplay::Sluagh::DecideActions(float dt, PlayerUnit::MovementInput &movement, PlayerUnit::ActionInput &action)
{
	StartProfile;
	ClearMap();

	CalculateWeaponChangeUtilities(dt);
	CalculateSkillUtilities(dt);
	CalculateUseItemUtilities(dt);
	CalculateMovementUtilities(dt);
	DecideActionInput(dt, action);
	DecideMovementInput(dt, movement, action);
	

	StopProfile;
}

float SE::Gameplay::Sluagh::UtilityForUsingACertainSkill(float dt, Skill & skillToCheck)
{
	StartProfile;
	if (skillToCheck.currentCooldown > 0.f)
		ProfileReturnConst(0.f);

	if (skillToCheck.atkType == DamageSources::DAMAGE_SOURCE_MELEE && distanceToPlayer < 2.f)
		ProfileReturnConst(0.f);

	float utilityValue = CalculateSkillDamageAfterResistances(thePlayer, skillToCheck)*20.5f/skillToCheck.cooldown;
	
	ProfileReturnConst(utilityValue);
}

float SE::Gameplay::Sluagh::UtilityForChangingWeapon(float dt, int & weaponToSwapFor)
{
	StartProfile;

	float maxUtility = 0.f;
	auto playerWeapon = thePlayer->GetCurrentItem();

	auto type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(playerWeapon, "Type", -1));
	Item::Weapon::Type playerWeaponType = Item::Weapon::Type::NONE;
	if (type != int32_t(-1))
		playerWeaponType = Item::Weapon::Type(type);
	
	auto sluaghWeapons = theSluagh->GetAllItems();

	for(int i = 0; i < 5; i++)
	{
		if((type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(sluaghWeapons[i], "Type", -1))) != -1)
		{
			float swapUtility = 0.f;
			auto sluaghWeaponType = Item::Weapon::Type(type);
			switch(sluaghWeaponType)
			{
			case Item::Weapon::Type::SWORD:
				
				if (playerWeaponType == Item::Weapon::Type::CROSSBOW || playerWeaponType == Item::Weapon::Type::WAND)
				{
					/*Disadvantage for the Sluagh*/
					swapUtility -= 0.5f;
				}
				swapUtility += CalculateWeaponDamageAfterResistances(thePlayer, sluaghWeapons[i]) /2.f;
				swapUtility += CalculateTimeToLiveDifference(sluaghWeapons[i]) / 5.0f;
				if (distanceToPlayer > 2.0f)
				{
					swapUtility = 0.f;
				}
				else
					swapUtility += 1.f;

				break;
				
			case Item::Weapon::Type::CROSSBOW: /*Fall through, only checking ranged*/
			case Item::Weapon::Type::WAND:
				if (distanceToPlayer < 2.0f)
				{
					swapUtility -= 0.5f;
				}
				if (playerWeaponType == Item::Weapon::Type::SWORD)
				{
					/*Advantage for the Sluagh*/
					swapUtility += 0.5f;
				}
				swapUtility += CalculateWeaponDamageAfterResistances(thePlayer, sluaghWeapons[i]) / 2.f;
				swapUtility += CalculateTimeToLiveDifference(sluaghWeapons[i]) / 5.0f;
				break;

			default: ;
			}
			if(maxUtility < swapUtility)
			{
				maxUtility = swapUtility;
				weaponToSwapFor = i;
			}
			
		}
	}


	ProfileReturnConst(maxUtility);
}

float SE::Gameplay::Sluagh::UtilityForUsingItem(float dt, int & item)
{
	StartProfile;
	
	auto sluaghWeapons = theSluagh->GetAllItems();
	float maxUtility = 0.f;
	int32_t type;
	for (int i = 0; i < 5; i++)
	{
		ItemType itemType;
		if ((type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(sluaghWeapons[i], "Type", -1))) != -1)
		{
			float swapUtility = 0.f;
			auto itemType = ItemType(type);
			switch (itemType)
			{
			case ItemType::CONSUMABLE:
				if(theSluagh->GetHealth() < theSluagh->GetMaxHealth()/2.f)
					swapUtility += 2.0f*std::get<int32_t>(CoreInit::managers.dataManager->GetValue(sluaghWeapons[i], "Health", 0)) / 5.f;
				break;

			default:
				break;
			}

			if (maxUtility < swapUtility)
			{
				maxUtility = swapUtility;
				item = i;
			}

		}
	}


	ProfileReturnConst(0.f);
}

float SE::Gameplay::Sluagh::UtilityForMoveInDirection(float dt, MovementDirection dir)
{
	StartProfile;
	/*CHECK COLLISION YOU TWAT! Utility 0 if you collide!*/
	float utilityValue = 0.f;

	float xPos = theSluagh->GetXPosition();
	float yPos = theSluagh->GetYPosition();
		

	auto type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(thePlayer->GetCurrentItem(), "Type", -1));
	Item::Weapon::Type playerWeaponType = Item::Weapon::Type::NONE;
	if (type != int32_t(-1))
		playerWeaponType = Item::Weapon::Type(type);

	type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(theSluagh->GetCurrentItem(), "Type", -1));
	Item::Weapon::Type sluaghWeaponType = Item::Weapon::Type::NONE;
	if (type != int32_t(-1))
		sluaghWeaponType = Item::Weapon::Type(type);

	PlayerUnit::MovementInput moveEvaluate(false, false, false, false, false, thePlayer->GetXPosition(), thePlayer->GetYPosition());

	switch(dir)
	{
	case MovementDirection::UP: moveEvaluate.upButton = true; break;
	case MovementDirection::DOWN: moveEvaluate.downButton = true; break;
	case MovementDirection::LEFT: moveEvaluate.leftButton = true; break;
	case MovementDirection::RIGHT: moveEvaluate.rightButton = true; break;
	default: ;
	}
	bool lineOfSightBeforeMove = room->CheckLineOfSightBetweenPoints(theSluagh->GetXPosition(), 
		theSluagh->GetYPosition(),
		thePlayer->GetXPosition(), 
		thePlayer->GetYPosition());
	theSluagh->UpdateMovement(dt, moveEvaluate);
	bool lineOfSightAfterMove = room->CheckLineOfSightBetweenPoints(theSluagh->GetXPosition(),
		theSluagh->GetYPosition(),
		thePlayer->GetXPosition(),
		thePlayer->GetYPosition());


	/*Check the weapon combinations, adapt the behaviour after that*/
	if(sluaghWeaponType == Item::Weapon::Type::SWORD && playerWeaponType == Item::Weapon::Type::SWORD) /*Both wields melee weapon -> Sluagh moves towards the player*/
	{
		float xDifference = theSluagh->GetXPosition() - thePlayer->GetXPosition();
		float yDifference = theSluagh->GetYPosition() - thePlayer->GetYPosition();
		float distAfterMove = sqrtf(xDifference*xDifference + yDifference*yDifference);

		if (distanceToPlayer > distAfterMove)
		{
			utilityValue += abs(distanceToPlayer - distAfterMove)*20.f;

			if (lineOfSightAfterMove)
				utilityValue += 1.f;
		}
		else if (distAfterMove > 2.0f && !lineOfSightAfterMove)
			utilityValue = 0.f;

		if (distAfterMove < 1.5f && distanceToPlayer > distAfterMove)
			utilityValue = 0.f;
		else if (distAfterMove < 1.0f)
			utilityValue = 1.25f;
		
	}
	else if((sluaghWeaponType == Item::Weapon::Type::CROSSBOW || sluaghWeaponType == Item::Weapon::Type::WAND) && playerWeaponType == Item::Weapon::Type::SWORD) /*Player Melee, Sluagh Ranged*/
	{
		float xDifference = theSluagh->GetXPosition() - thePlayer->GetXPosition();
		float yDifference = theSluagh->GetYPosition() - thePlayer->GetYPosition();
		float distAfterMove = sqrtf(xDifference*xDifference + yDifference*yDifference);
		
		if (distanceToPlayer < distAfterMove && lineOfSightAfterMove)
			utilityValue += abs(distanceToPlayer - distAfterMove)*20.f;
		else if (distAfterMove < 2.0f)
		{
			utilityValue -= 0.5f;
		}
		if (lineOfSightBeforeMove && !lineOfSightAfterMove) /*Moved behing object... not good*/
			utilityValue -= 0.5f;
		else if (!lineOfSightBeforeMove && lineOfSightAfterMove) /*Moved so we could see, pretty good*/
			utilityValue += 0.5f;
		else if (!lineOfSightAfterMove)
			utilityValue -= 0.5f;
		else
			utilityValue += 0.75f;
	}
	else if(sluaghWeaponType == Item::Weapon::Type::SWORD && (playerWeaponType == Item::Weapon::Type::CROSSBOW || playerWeaponType == Item::Weapon::Type::WAND)) /*Player Ranged, Sluagh Melee*/
	{
		float xDifference = theSluagh->GetXPosition() - thePlayer->GetXPosition();
		float yDifference = theSluagh->GetYPosition() - thePlayer->GetYPosition();
		float distAfterMove = sqrtf(xDifference*xDifference + yDifference*yDifference);

		if (distanceToPlayer > distAfterMove)
		{
			utilityValue += abs(distanceToPlayer - distAfterMove)*20.f;

			if (lineOfSightAfterMove)
				utilityValue += 1.f;
		}

		if (distAfterMove < 1.5f && distanceToPlayer > distAfterMove)
			utilityValue = 0.f;
		else if (distAfterMove < 1.0f)
			utilityValue = 1.25f;
	}
	else if(playerWeaponType == Item::Weapon::Type::NONE) /*CONFUSED SCREAMING*/
	{
		float xDifference = theSluagh->GetXPosition() - thePlayer->GetXPosition();
		float yDifference = theSluagh->GetYPosition() - thePlayer->GetYPosition();
		float distAfterMove = sqrtf(xDifference*xDifference + yDifference*yDifference);

		if (distanceToPlayer > distAfterMove)
		{
			utilityValue += abs(distanceToPlayer - distAfterMove)*20.f;

			if (lineOfSightAfterMove)
				utilityValue += 1.f;
		}
		if (distAfterMove < 1.5f && distanceToPlayer > distAfterMove)
			utilityValue = 0.f;
		else if (distAfterMove < 1.0f)
			utilityValue = 1.25f;
	}
	else /*Player Ranged, Sluagh Ranged*/
	{
		float xDifference = theSluagh->GetXPosition() - thePlayer->GetXPosition();
		float yDifference = theSluagh->GetYPosition() - thePlayer->GetYPosition();
		float distAfterMove = sqrtf(xDifference*xDifference + yDifference*yDifference);

		if (distanceToPlayer < distAfterMove && lineOfSightAfterMove)
			utilityValue += abs(distanceToPlayer - distAfterMove)*20.f;
		else if (distAfterMove < 2.0f)
		{
			utilityValue -= 0.5f;
		}
		if (lineOfSightBeforeMove && !lineOfSightAfterMove) /*Moved behing object... not good*/
			utilityValue -= 0.5f;
		else if (!lineOfSightBeforeMove && lineOfSightAfterMove) /*Moved so we could see, pretty good*/
			utilityValue += 0.5f;
		else if (!lineOfSightAfterMove)
			utilityValue -= 0.5f;
		else
			utilityValue += 0.75f;

		if(abs(xPos-thePlayer->GetXPosition()) < abs(yPos-thePlayer->GetYPosition()))
		{
			if (abs(yDifference) < abs(yPos - thePlayer->GetYPosition()))
				utilityValue += 0.75f;
		}
		else
		{
			if (abs(xDifference) < abs(xPos - thePlayer->GetXPosition()))
				utilityValue += 0.75f;			
		}
	}

	theSluagh->PositionEntity(xPos, yPos);

	ProfileReturnConst(utilityValue);
}

void SE::Gameplay::Sluagh::ClearMap()
{
	utilityMap.clear();
}

float SE::Gameplay::Sluagh::CalculateWeaponDamageAfterResistances(PlayerUnit * unit, const Core::Entity &weaponEntity)
{
	StartProfile;
	auto ElementalType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Element", -1));
	if(ElementalType != int32_t(-1))
	{
		auto unitStats = unit->GetNewStat();
		auto weaponDamage = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Damage", 0.f));
		switch(DamageType(ElementalType))
		{
		case DamageType::PHYSICAL: ProfileReturnConst(weaponDamage*(1.0f - unitStats.physicalResistance));
		case DamageType::FIRE: ProfileReturnConst(weaponDamage*(1.0f - unitStats.fireResistance));
		case DamageType::WATER: ProfileReturnConst(weaponDamage*(1.0f - unitStats.waterResistance));
		case DamageType::NATURE: ProfileReturnConst(weaponDamage*(1.0f - unitStats.natureResistance));
		case DamageType::RANGED: ProfileReturnConst(weaponDamage*(1.0f - unitStats.physicalResistance));
		case DamageType::MAGIC: ProfileReturnConst(weaponDamage*(1.0f - unitStats.magicResistance));
		default: ProfileReturnConst(0.f);
		}
	}
	ProfileReturnConst(0.f);
}

float SE::Gameplay::Sluagh::CalculateSkillDamageAfterResistances(PlayerUnit * unit, Skill & skillToCheck)
{
	StartProfile;
	auto unitStats = unit->GetNewStat();
	switch (skillToCheck.damageType)
	{
	case DamageType::PHYSICAL: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.physicalResistance));
	case DamageType::FIRE: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.fireResistance));
	case DamageType::WATER: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.waterResistance));
	case DamageType::NATURE: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.natureResistance));
	case DamageType::RANGED: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.physicalResistance));
	case DamageType::MAGIC: ProfileReturnConst(skillToCheck.skillDamage*(1.0f - unitStats.magicResistance));
	default: ProfileReturnConst(0.f);
	}
	StopProfile;
}

float SE::Gameplay::Sluagh::CalculateTimeToLiveDifference(const Core::Entity &weaponEntity)
{
	StartProfile;
	float sluaghDamage = 0.f;
	float playerDamage = 0.f;
	auto playerStats = thePlayer->GetNewStat();
	auto sluaghStats = theSluagh->GetNewStat();

	auto type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(thePlayer->GetCurrentItem(), "Type", -1));
	Item::Weapon::Type playerWeaponType = Item::Weapon::Type::NONE;
	if (type != int32_t(-1))
		playerWeaponType = Item::Weapon::Type(type);

	type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Type", -1));
	Item::Weapon::Type sluaghWeaponType = Item::Weapon::Type::NONE;
	if (type != int32_t(-1))
		sluaghWeaponType = Item::Weapon::Type(type);

	if (sluaghWeaponType != Item::Weapon::Type::NONE)
	{
		auto ElementalType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Element", -1));
		if (ElementalType != int32_t(-1))
		{
			auto weaponDamage = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Damage", 0.f));
			switch (DamageType(ElementalType))
			{
			case DamageType::PHYSICAL: sluaghDamage = weaponDamage*(1.0f - playerStats.physicalResistance); break;
			case DamageType::FIRE: sluaghDamage = weaponDamage*(1.0f - playerStats.fireResistance); break;
			case DamageType::WATER: sluaghDamage = weaponDamage*(1.0f - playerStats.waterResistance); break;
			case DamageType::NATURE: sluaghDamage = weaponDamage*(1.0f - playerStats.natureResistance); break;
			case DamageType::RANGED: sluaghDamage = weaponDamage*(1.0f - playerStats.physicalResistance); break;
			case DamageType::MAGIC: sluaghDamage = weaponDamage*(1.0f - playerStats.magicResistance); break;
			default: break;
			}
		}
	}
	if (playerWeaponType != Item::Weapon::Type::NONE)
	{
		auto ElementalType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Element", -1));
		if (ElementalType != int32_t(-1))
		{
			auto weaponDamage = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(weaponEntity, "Damage", 0.f));
			switch (DamageType(ElementalType))
			{
			case DamageType::PHYSICAL: playerDamage = weaponDamage*(1.0f - sluaghStats.physicalResistance); break;
			case DamageType::FIRE: playerDamage = weaponDamage*(1.0f - sluaghStats.fireResistance); break;
			case DamageType::WATER: playerDamage = weaponDamage*(1.0f - sluaghStats.waterResistance); break;
			case DamageType::NATURE: playerDamage = weaponDamage*(1.0f - sluaghStats.natureResistance); break;
			case DamageType::RANGED: playerDamage = weaponDamage*(1.0f - sluaghStats.physicalResistance); break;
			case DamageType::MAGIC: playerDamage = weaponDamage*(1.0f - sluaghStats.magicResistance); break;
			default: break;
			}
		}
	}

	sluaghDamage /= sluaghStats.attackSpeed;
	playerDamage /= playerStats.attackSpeed;
	if (sluaghWeaponType == Item::Weapon::Type::SWORD && distanceToPlayer < 2.f)
	{
		sluaghDamage = 0.f;
	}

	if (playerWeaponType == Item::Weapon::Type::SWORD && distanceToPlayer < 2.f)
	{
		playerDamage = 0.f;
	}

	float sluaghLifeTime = 0.f;
	float playerLifeTime = 0.f;

	if(playerDamage >= 0.05f)
	{
		sluaghLifeTime = theSluagh->GetHealth() / playerDamage;
	}
	if(sluaghDamage >= 0.05f)
	{
		playerLifeTime = thePlayer->GetHealth() / sluaghDamage;
	}

	if(playerLifeTime <= 0.05f)
	{
		ProfileReturnConst(sluaghLifeTime * 10.f);
	}
	if(sluaghLifeTime <= 0.05f)
	{
		ProfileReturnConst(sluaghLifeTime);
	}
	ProfileReturnConst(sluaghLifeTime/playerLifeTime);
}

void SE::Gameplay::Sluagh::CalculateSkillUtilities(float dt)
{
	float xDist = thePlayer->GetXPosition() - theSluagh->GetXPosition();
	float yDist = thePlayer->GetYPosition() - theSluagh->GetYPosition();
	distanceToPlayer = sqrtf(xDist*xDist + yDist*yDist);

	auto skills = theSluagh->GetAllSkills();
	utilityMap[UtilityMapEnum::SKILL1] = UtilityForUsingACertainSkill(dt, skills[0]);
	utilityMap[UtilityMapEnum::SKILL2] = UtilityForUsingACertainSkill(dt, skills[1]);

}

void SE::Gameplay::Sluagh::CalculateWeaponChangeUtilities(float dt)
{
	int weaponSlot = -1;
	float weaponUtility = UtilityForChangingWeapon(dt, weaponSlot);

	switch (weaponSlot)
	{
	case 0:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_1] = weaponUtility;
		break;
	case 1:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_2] = weaponUtility;
		break;
	case 2:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_3] = weaponUtility;
		break;
	case 3:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_4] = weaponUtility;
		break;
	case 4:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_5] = weaponUtility;
		break;
	default:

		break;
	}

}

void SE::Gameplay::Sluagh::CalculateUseItemUtilities(float dt)
{
	int itemSlot = -1;
	float itemUtility = UtilityForUsingItem(dt, itemSlot);

	switch (itemSlot)
	{
	case 0:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_1] = itemUtility;
		break;
	case 1:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_2] = itemUtility;
		break;
	case 2:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_3] = itemUtility;
		break;
	case 3:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_4] = itemUtility;
		break;
	case 4:
		utilityMap[UtilityMapEnum::WEAPON_SLOT_5] = itemUtility;
		break;
	default:

		break;
	}
}

void SE::Gameplay::Sluagh::CalculateMovementUtilities(float dt)
{
	utilityMap[UtilityMapEnum::MOVE_UP] = UtilityForMoveInDirection(dt, MovementDirection::UP);
	utilityMap[UtilityMapEnum::MOVE_DOWN] = UtilityForMoveInDirection(dt, MovementDirection::DOWN);
	utilityMap[UtilityMapEnum::MOVE_LEFT] = UtilityForMoveInDirection(dt, MovementDirection::LEFT);
	utilityMap[UtilityMapEnum::MOVE_RIGHT] = UtilityForMoveInDirection(dt, MovementDirection::RIGHT);

}

void SE::Gameplay::Sluagh::DecideActionInput(float dt, PlayerUnit::ActionInput& action)
{
	StartProfile;
	float maxUtility = -1.f;
	UtilityMapEnum choice;

	int currentEquipment = 0;
	auto currentEquipmentEntity = theSluagh->GetCurrentItem();
	auto allEquipments = theSluagh->GetAllItems();

	for (int i = 0; i < 5; i++)
	{
		if (currentEquipmentEntity == allEquipments[i])
		{
			currentEquipment = i;
			break;
		}
	}


	for (auto utility : utilityMap)
	{
		if (maxUtility < utility.second && int(utility.first) < 7)
		{
			choice = utility.first;
			maxUtility = utility.second;
		}
	}

	if (maxUtility <= 0.f) /*No choice*/
		ProfileReturnVoid;

	switch (choice)
	{
	case UtilityMapEnum::SKILL1: action.skill1Button = true; break;
	case UtilityMapEnum::SKILL2: action.skill2Button = true; break;
	case UtilityMapEnum::WEAPON_SLOT_1:
		if (currentEquipment == 0)
		{
			action.actionButton = true;
		}
		else
		{
			action.one = true;
		}
		break;
	case UtilityMapEnum::WEAPON_SLOT_2:
		if (currentEquipment == 1)
		{
			action.actionButton = true;
		}
		else
		{
			action.two = true;
		}
		break;
	case UtilityMapEnum::WEAPON_SLOT_3:
		if (currentEquipment == 2)
		{
			action.actionButton = true;
		}
		else
		{
			action.three = true;
		}
		break;
	case UtilityMapEnum::WEAPON_SLOT_4:
		if (currentEquipment == 3)
		{
			action.actionButton = true;
		}
		else
		{
			action.four = true;
		}
		break;
	case UtilityMapEnum::WEAPON_SLOT_5:
		if (currentEquipment == 4)
		{
			action.actionButton = true;
		}
		else
		{
			action.five = true;
		}
		break;


	default:;
	}
	/*if(distanceToPlayer < 2.5f)
		actionCooldown = 0.35f;*/
	StopProfile;
}

void SE::Gameplay::Sluagh::DecideMovementInput(float dt, PlayerUnit::MovementInput& movement,
	PlayerUnit::ActionInput& action)
{
	StartProfile;
	if (actionCooldown <= 0.f)
		actionCooldown = 0.f;
	else
	{
		actionCooldown -= dt;
		ProfileReturnVoid;
	}

	bool moved = false;
	if (0.50f < utilityMap[UtilityMapEnum::MOVE_UP])
	{
		movement.upButton = true;
		moved = true;
	}
	if (0.50f < utilityMap[UtilityMapEnum::MOVE_LEFT])
	{
		movement.leftButton = true;
		moved = true;
	}
	if (0.50f < utilityMap[UtilityMapEnum::MOVE_DOWN])
	{
		movement.downButton = true;
		moved = true;
	}
	if (0.50f < utilityMap[UtilityMapEnum::MOVE_RIGHT])
	{
		movement.rightButton = true;
		moved = true;
	}

	if(moved)
	{
		commitmentTime = 0.5f;
		previousMovement = movement;
		timeSinceMovement = 0.f;
	}
	else
	{
		if (SluaghMovedPreviousFrame())
			timeSinceMovement = 0.f;
		else
			timeSinceMovement += dt;
		/*Foce a bit of movement*/
		if(2.5 < timeSinceMovement)
		{
			for(int i = 0; i < 10; i++)
			{
				switch(CoreInit::subSystems.window->GetRand()%4)
				{
				case 0:
					previousMovement.upButton = !previousMovement.upButton;
					break;
				case 1:
					previousMovement.downButton = !previousMovement.downButton;
					break;
				case 2:
					previousMovement.rightButton = !previousMovement.rightButton;
					break;
				case 3:
					previousMovement.leftButton = !previousMovement.leftButton;
					break;
				}
			}
			commitmentTime = 0.5f;
		}

		movement = previousMovement;
		if (commitmentTime <= 0.f)
		{
			commitmentTime = 0.f;
		}
		else
			commitmentTime -= dt;

	}
	StopProfile;

}
