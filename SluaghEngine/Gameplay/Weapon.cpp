#include <Weapon.h>
#include "CoreInit.h"
#include <Items.h>
#include <array>
#include <Stats.h>
#include <EventStructs.h>

struct WeaponInfo
{
	SE::Utilz::GUID icon;
	SE::Utilz::GUID selectedIcon;
	SE::Utilz::GUID backTex;
	SE::Utilz::GUID mesh;
	SE::Utilz::GUID mat;
	SE::Utilz::GUID shader;
	SE::Utilz::GUID attAnim;
	SE::Utilz::GUID equipJoint;
	int slotIndex;
	DirectX::XMFLOAT3 equipPos;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 equipRot;
	SE::Gameplay::DamageType damageType;
};

static const std::array<WeaponInfo, 3> weaponInfo = { {
	{ "Sword_black.png", "Sword2.png", "PhysicalPickup.png", "Sword.mesh", "Sword.mat", "SimpleLightPS.hlsl",  "TopSwordAttackAnim_MCModell.anim", "LHand", 0,{ 0.2f, -0.1f, -0.5f },{ 1,1,1 },{ 3.0f, -0.4f, 1.3f }, SE::Gameplay::DamageType::PHYSICAL },
	{ "Crossbow_black.png", "Crossbow_silver.png", "RangedPickup.png", "Crossbow.mesh", "Crossbow.mat", "SimpleLightPS.hlsl",  "TopCrossbowAttackAnim_MCModell.anim" , "LHand", 0,{ 0, -0.02f, -0.25f },{ 0.15f,0.15f,0.15f },{ 1.6f, 1.2f,0.0f }, SE::Gameplay::DamageType::RANGED },
	{ "Wand_black.png","Wand_silver.png", "MagicPickup.png", "WandPivotEnd.mesh", "WandPivotEnd.mat", "SimpleLightPS.hlsl",  "TopWandAttackAnim_MCModell.anim", "LHand", 0,{ 0.05f, 0.0f, -0.2f },{ 1,1,1 },{ 3.0f, -0.4f, 1.3f } , SE::Gameplay::DamageType::MAGIC }
	} };

struct ProjectileInfo
{
	float damageMod;
	SE::Utilz::GUID projectile;
};

static const std::vector<std::vector<ProjectileInfo>> projectiles = 
{
	{
		{ 0.16f, "playerMeleeProjectiles.SEP" },
		{ 0.16f, "playerMeleeProjectiles.SEP" }
	},

	{
		{ 0.1f, "CrossbowAttack.SEP" },
		{ 2.0f, "BoomerangArrow.SEP" },
		{ 0.09f, "TripleArrow.SEP" }
	},
	{
		{ 0.1f, "WandAttack.SEP" },
		{ 0.09f, "TronWand.SEP" },
		{ 0.08f, "WandVortex.SEP" }
	}
};

SE::Core::Entity SE::Gameplay::Item::Weapon::Create()
{
	return Create(Weapon::Type(std::rand() % int(Weapon::Type::NONE)), false);
}

SE::Core::Entity SE::Gameplay::Item::Weapon::Create(Weapon::Type type, bool base, int32_t projectileID)
{
	auto wep = CoreInit::managers.entityManager->Create();
	CoreInit::managers.dataManager->SetValue(wep, "Item", int32_t(ItemType::WEAPON));
	CoreInit::managers.dataManager->SetValue(wep, "Type", int32_t(type));
	CoreInit::managers.dataManager->SetValue(wep, "DamageType", int32_t(weaponInfo[size_t(type)].damageType));
	CoreInit::managers.dataManager->SetValue(wep, "Health", base ? 0 : Stats::GetRandHealth());
	CoreInit::managers.dataManager->SetValue(wep, "Str", base ? 0 : Stats::GetRandStr());
	CoreInit::managers.dataManager->SetValue(wep, "Agi", base ? 0 : Stats::GetRandAgi());
	CoreInit::managers.dataManager->SetValue(wep, "Wis", base ? 0 : Stats::GetRandWil());
	
	CoreInit::managers.dataManager->SetValue(wep, "Element", int32_t(Stats::GetRandomDamageType()));
	CoreInit::managers.dataManager->SetValue(wep, "AttAnim", weaponInfo[size_t(type)].attAnim.id);
	if (projectileID == -1)
	{
		auto randProjectile = std::rand() % projectiles[size_t(type)].size();
		CoreInit::managers.dataManager->SetValue(wep, "Damage", int32_t((base ? 10 : Stats::GetRandDamage()) * projectiles[size_t(type)][randProjectile].damageMod));
		CoreInit::managers.dataManager->SetValue(wep, "AttProj", projectiles[size_t(type)][randProjectile].projectile.id);
	}
	else
	{
		CoreInit::managers.dataManager->SetValue(wep, "Damage", int32_t((base ? 10 : Stats::GetRandDamage()) * projectiles[size_t(type)][projectileID].damageMod));
		CoreInit::managers.dataManager->SetValue(wep, "AttProj", projectiles[size_t(type)][projectileID].projectile.id);
	}
	CreateMeta(wep);


	return wep;
}

void SE::Gameplay::Item::Weapon::CreateMeta(SE::Core::Entity wep)
{
	auto wType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(wep, "Type", -1));
	CoreInit::managers.transformManager->Create(wep);
	CoreInit::managers.transformManager->SetScale(wep, weaponInfo[size_t(wType)].scale);
	CoreInit::managers.materialManager->Create(wep, { weaponInfo[size_t(wType)].shader, weaponInfo[size_t(wType)].mat });
	CoreInit::managers.renderableManager->CreateRenderableObject(wep, { weaponInfo[size_t(wType)].mesh , false, false, true });
	Core::IGUIManager::CreateInfo icon;
	icon.texture = weaponInfo[size_t(wType)].icon;
	icon.textureInfo.width = 50;
	icon.textureInfo.height = 50;
	icon.textureInfo.anchor = { 0.5f,0.5f };
	icon.textureInfo.screenAnchor = { 0, 1 };
	icon.textureInfo.posX = 10;
	icon.textureInfo.posY = -60;
	icon.textureInfo.layerDepth = 0.1;

	CoreInit::managers.guiManager->Create(wep, icon);
}

using namespace SE;

void SE::Gameplay::Item::Weapon::RenderItemInfo(Core::Entity item, Core::Entity compareWith)
{
	long posY = -100;
	long textHeigth = 35;
	Core::ITextManager::CreateInfo tci;
	auto damage = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Damage", 0));
	auto damageCW = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(compareWith, "Damage", 0));
	if (damage > damageCW)
		tci.info.colour = { 0, 1.0f, 0.0f, 1.0f };
	else if (damage < damageCW)
		tci.info.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	else
		tci.info.colour = { 1.0f, (165 / 255.0f), 0.0f, 1.0f };
	tci.font = "CloisterBlack.spritefont";
	tci.info.posX = -65;
	tci.info.posY = posY;
	tci.info.screenAnchor = { 0.5f, 0.5f };
	tci.info.anchor = { 1.0f,0.0f };
	tci.info.scale = { 0.4f, 1.0f };
	tci.info.height = textHeigth;
	tci.info.text = L"Skada";
	tci.info.layerDepth = 0.1;
	auto textEnt = CoreInit::managers.entityManager->Create();
	CoreInit::managers.textManager->Create(textEnt, tci);
	CoreInit::managers.textManager->ToggleRenderableText(textEnt, true);
	CoreInit::managers.eventManager->RegisterEntitytoEvent(textEnt, "StopRenderItemInfo");
	posY += textHeigth + 5;

	auto health = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Health", 0));
	auto healthCW = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(compareWith, "Health", 0));
	if (health > healthCW)
		tci.info.colour = { 0, 1.0f, 0.0f, 1.0f };
	else if (health < healthCW)
		tci.info.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	else
		tci.info.colour = { 1.0f, (165 / 255.0f), 0.0f, 1.0f };
	//tci.info.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	tci.info.posY = posY;
	tci.info.text = L"H�lsa";
	textEnt = CoreInit::managers.entityManager->Create();
	CoreInit::managers.textManager->Create(textEnt, tci);
	CoreInit::managers.textManager->ToggleRenderableText(textEnt, true);
	CoreInit::managers.eventManager->RegisterEntitytoEvent(textEnt, "StopRenderItemInfo");
	posY += textHeigth + 5;

	auto str = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Str", 0));
	auto strCW = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(compareWith, "Str", 0));
	if (str > strCW)
		tci.info.colour = { 0, 1.0f, 0.0f, 1.0f };
	else if (str < strCW)
		tci.info.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	else
		tci.info.colour = { 1.0f, (165 / 255.0f), 0.0f, 1.0f };
	//tci.info.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	tci.info.posY = posY;
	tci.info.text = L"Styrka";
	textEnt = CoreInit::managers.entityManager->Create();
	CoreInit::managers.textManager->Create(textEnt, tci);
	CoreInit::managers.textManager->ToggleRenderableText(textEnt, true);
	CoreInit::managers.eventManager->RegisterEntitytoEvent(textEnt, "StopRenderItemInfo");
	posY += textHeigth + 5;


	auto agi = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Agi", 0));
	auto agiCW = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(compareWith, "Agi", 0));
	if (agi > agiCW)
		tci.info.colour = { 0, 1.0f, 0.0f, 1.0f };
	else if (agi < agiCW)
		tci.info.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	else
		tci.info.colour = { 1.0f, (165 / 255.0f), 0.0f, 1.0f };
	//tci.info.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	tci.info.posY = posY;
	tci.info.text = L"Vighet";
	textEnt = CoreInit::managers.entityManager->Create();
	CoreInit::managers.textManager->Create(textEnt, tci);
	CoreInit::managers.textManager->ToggleRenderableText(textEnt, true);
	CoreInit::managers.eventManager->RegisterEntitytoEvent(textEnt, "StopRenderItemInfo");
	posY += textHeigth + 5;
	
	auto wis = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Wis", 0));
	auto wisCW = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(compareWith, "Wis", 0));
	if (agi > agiCW)
		tci.info.colour = { 0, 1.0f, 0.0f, 1.0f };
	else if (agi < agiCW)
		tci.info.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
	else
		tci.info.colour = { 1.0f, (165 / 255.0f), 0.0f, 1.0f };
	//tci.info.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	tci.info.posY = posY;
	tci.info.text = L"Vishet"; // "F�rst�nd
	textEnt = CoreInit::managers.entityManager->Create();
	CoreInit::managers.textManager->Create(textEnt, tci);
	CoreInit::managers.textManager->ToggleRenderableText(textEnt, true);
	CoreInit::managers.eventManager->RegisterEntitytoEvent(textEnt, "StopRenderItemInfo");
	posY += textHeigth + 5;

	Core::IGUIManager::CreateInfo ciback;
	auto type = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(item, "Type", -1));

	ciback.texture = weaponInfo[type].backTex;
	ciback.textureInfo.width = 150;
	ciback.textureInfo.height = 200;
	ciback.textureInfo.posX = -40;
	ciback.textureInfo.posY = 0;
	ciback.textureInfo.screenAnchor = { 0.5f, 0.5f };
	ciback.textureInfo.anchor = { 1.0f, 0.5f };
	ciback.textureInfo.layerDepth = 0.11;
	auto weaponBack = CoreInit::managers.entityManager->Create();
	CoreInit::managers.guiManager->Create(weaponBack, ciback);

	CoreInit::managers.guiManager->ToggleRenderableTexture(weaponBack, true);

	CoreInit::managers.eventManager->RegisterEntitytoEvent(weaponBack, "StopRenderItemInfo");

}

void SE::Gameplay::Item::Weapon::Equip(Core::Entity wep, Core::Entity to)
{
	auto wType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(wep, "Type", -1));

	CoreInit::managers.guiManager->SetTexture(wep, weaponInfo[wType].selectedIcon);
	CoreInit::managers.transformManager->SetPosition(wep, weaponInfo[wType].equipPos);
	CoreInit::managers.transformManager->SetRotation(wep, weaponInfo[wType].equipRot.x, weaponInfo[wType].equipRot.y, weaponInfo[wType].equipRot.z);
	CoreInit::managers.animationManager->AttachToEntity(to, wep, weaponInfo[wType].equipJoint, weaponInfo[wType].slotIndex);
	CoreInit::managers.renderableManager->ToggleRenderableObject(wep, true);
	CoreInit::managers.dataManager->SetValue(wep, "Equipped", true);
}

void SE::Gameplay::Item::Weapon::UnEquip(Core::Entity wep, Core::Entity from)
{
	auto wType = std::get<int32_t>(CoreInit::managers.dataManager->GetValue(wep, "Type", -1));

	CoreInit::managers.guiManager->SetTexture(wep, weaponInfo[wType].icon);
	CoreInit::managers.animationManager->DettachFromEntity(from, weaponInfo[wType].slotIndex);
	CoreInit::managers.renderableManager->ToggleRenderableObject(wep, false);
	CoreInit::managers.dataManager->SetValue(wep, "Equipped", false);
}
