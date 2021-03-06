#ifndef _SE_GAMEPLAY_PLAY_STATE_H
#define _SE_GAMEPLAY_PLAY_STATE_H

#include <vector>
#include "IGameState.h"
#include "PlayerUnit.h"
#include "Room.h"
#include <Gameplay\ProjectileManager.h>
#include <Gameplay\Projectile.h>
#include <Gameplay\ProjectileData.h>
#include "GameBlackboard.h"
#include "EnemyFactory.h"
#include <Gameplay\HUD_Parsing.h>
#include <Gameplay\HUD_Functionality.h>
#include <optional>

namespace SE 
{
	namespace Gameplay
	{
		/**
		*
		* @brief The Play state Class
		*
		* @details class used for the play state, more to be added later, inherits from Gamestate baseclass.
		*
		* @warning Not complete
		*
		* @sa IGameState.
		*
		**/
		class PlayState : public IGameState
		{
		public:
			PlayState();
			PlayState(Window::IWindow* Input, SE::Core::IEngine* engine, void* passedInfo);
			~PlayState();
			/**
			* @brief	The update loop for the Play state process.
			*
			* @details	An update loop to be used during the Play state phase, more info added later
			*
			* @param[in,out] passableInfo is a void pointer which lets us send information inbetween the states.
			*
			* @retval State return value to which other state to switch to when done.
			*
			* @warning Not fully implemented.
			*
			*/
			State Update(void*& passableInfo);

			void ToggleFlowField(bool showFlowField);

			
		private:

			struct SkillIndicator
			{
				Core::Entity frame;
				Core::Entity Image;
				float currentCooldown = 0;
				float maxCooldown;
			}skillIndicators[2];

			struct MiniMap {

				Core::Entity map;
				Core::Entity frame;
			};

			struct RoomContainer {

				Room* room = nullptr;
				Core::Entity symbol;
				bool visited = false;
			};

			void InitializeRooms();
			void InitializeEnemies();
			void InitializePlayer(void* playerInfo);
			void InitializeOther();
			void InitWeaponPickups();

			void CreateFlowFieldRendering();
			void DestroyFlowFieldRendering();
			void UpdateFlowFieldRendering();

			void CreateMiniMap();

			void UpdateInput(PlayerUnit::MovementInput &movement, PlayerUnit::ActionInput &action);
			void UpdateProjectiles(std::vector<ProjectileData>& newProjectiles);
			void CheckForRoomTransition();
			void UpdateHUD(float dt);

			std::wstring GenerateDeathMessage();
			void InitializeDeathSequence();
			void UpdateDeathCamera(float dt, float rotValue, float zoomValue, float zoomLimit);

			HUDParser playStateGUI;
			std::string OptionalButtons[2]
			{
				"EnemyHp.HuD",
				"EnemyHpFrame.HuD"
			};
			int healthBarPos;
			Core::Entity cam;
			Core::Entity dummy;
			Core::Entity usePrompt;
			Core::Entity returnPrompt;
			Core::Entity aimDecal;
			uint32_t dummyBoxJobID;
			PlayerUnit* player;

			uint8_t worldWidth;
			uint8_t worldHeight;

			void UpdateAimDecal();

			inline std::optional<std::reference_wrapper<RoomContainer>> GetRoom(int x, int y)
			{
				if (x < worldWidth && x >= 0 && y < worldHeight && y >= 0  && rooms[x* worldHeight + y].room)
					return rooms[x* worldHeight + y];
				else
					return std::nullopt;
			}


			void LoadAdjacentRooms(int x, int y, int sx, int sy);
			void UnloadAdjacentRooms(int x, int y, int sx, int sy);
			void CloseDoorsToRoom(int x, int y);
			void OpenDoorsToRoom(int x, int y);

			RoomContainer* rooms;
			Room* currentRoom; 
			int currentRoomX = 0;
			int currentRoomY = 0;
			int sluaghRoomX = 0;
			int sluaghRoomY = 0;
			bool sluaghDoorsOpen = false;
			int enemiesInEachRoom = 2;
			int numberOfFreeFrames = 15;
			Gameplay::GameBlackboard blackBoard;
			Gameplay::EnemyFactory eFactory;

			ProjectileManager* projectileManager;

			/* game sounds*/
			SE::Core::Entity soundEnt;
			uint8_t currentSound;
			float soundTime = 0.0f;
			SE::Utilz::GUID sounds[3];

			MiniMap miniMap;
			bool noShow = false;
			bool deathSequence = false;
			float deathTimer = 0.0f;
			Core::Entity deathText;
			Core::Entity cameraDummy;
			bool showFlowField = false;
			std::vector<SE::Core::Entity> flowFieldEntities;


		protected:

		};
	}
}
#endif 
