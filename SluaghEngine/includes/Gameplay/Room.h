#ifndef _SE_GAMEPLAY_ROOM_H_
#define _SE_GAMEPLAY_ROOM_H_
#include <vector>
#include "FlowField.h"
#include "Projectile.h"
#include <Utilz\GUID.h>
#include <Core\IEngine.h>
#include <Gameplay/PlayerUnit.h>
#include "Fog.h"
namespace SE
{
	namespace Gameplay
	{
		class EnemyUnit;
		/**
		*
		* @brief The class definint the rooms as used by Gameplay
		*
		* @details The Room class is used to save the representation of a room as seen and used in game. This representation consists of:
		* A flowfield map (Pointing to the player or the (last known) exist closest to the player's position.
		* A map representation for the room
		* The enemies in the room
		* A list of adjacent rooms
		*
		* @warning Not fully implemented! A lot of the functionality simply isn't there yet! A shell to start out from.
		*
		* @sa Here the enemy class and algorithms to create the room will be linked
		*
		**/
		class Room
		{
		protected:
			
			struct HpBar
			{
				Core::Entity bar; // the moving bar representing the hp
				Core::Entity frame; // the frame around the bar
			};

			Utilz::GUID myRoomFile;
			Room* adjacentRooms[4] = {nullptr};
			std::vector<EnemyUnit*> enemyUnits;
			FlowField* roomField;
			
			std::vector<Core::Entity> itemsInRoom;
			std::vector<HpBar> hpBars;
			Fog fog;
			const float fogCycleDuration = 12.f;
			const float fogFadeDuration = 2.f;
			float time = fogCycleDuration + fogFadeDuration;
			bool IsOutside = false;
			enum class PropTypes
			{
				TABLES,
				CHAIRS,
				TORCHES_WALL,
				BUSHES,
				BIGPROPS,
				GENERIC,
				MEDIUM, 
				TREE
			};

			enum class Meshes {
				HighWall,
				Bush,
				Chair,
				Passage,
				OneSide,
				Corner,
				Top,
				ThreeSides,
				Door,
				Floor,
				Torch,
				Pillar_short,
				Table_long,
				Table_small,
				Table_round,
				Grass,
				FloorTorch,
				TableGroup1,
				Candlestick_tri,
				PotGroup1,
				Potatosack_closed,
				Potatosack_open,
				Fireplace,
				Fireplace_open,
				Painting,
				Window,
				Window_open,
				Window_closed,
				Tree,
				Well,
				Fireplace_set1,
				Fireplace_set2,
				MiniTable_set1,
				Potatobag_set1,
				PotFloorTorch_set1,
				BagPipeTable_set1,
				DinnerTable_set1,
				Smalltable_crossbowAndBat_set1,
				Smalltable_weapons_set1,
				statue
			};
			enum class Materials {
				Stone,
				FloorStone,
				FloorWood,
				DoorMat,
				WallStone,
				WallWood,
				Bush,
				Dirt,
				Grass,
				Wood, 
				OutsideWall,
				StoneFloor2,
				WoodFloor,
				FanzyWall,
				LightStoneWall,
				LightStoneWallWood,
				Window,
				Fireplace,
				Fireplace_open,
				Pillar,
				PotatosackOpen,
				PotatosackClosed,
				Well,
				julWall, 
				DarkStoneWall,
				Fireplace_set1,
				Fireplace_set2,
				MiniTable_set1,
				Potatobag_set1,
				PotFloorTorch_set1,
				BagPipeTable_set1,
				DinnerTable_set1,
				Smalltable_crossbowAndBat_set1,
				Smalltable_weapons_set1,
				statue
			};

			struct CreationArguments
			{
				SE::Core::Entity ent;
				int x;
				int y;
				int doorCounter;
				Core::IMaterialManager::CreateInfo mat;
				SE::Utilz::GUID wallMat;
				SE::Utilz::GUID floorMat;
			};

			struct Prop {
				SE::Utilz::GUID guid;
				SE::Utilz::GUID matGuid;
			};

			std::map<PropTypes, std::vector<Prop>> propVectors;
			std::map<unsigned char, std::function<void(CreationArguments&)>> propItemToFunction;
			std::map<Meshes, SE::Utilz::GUID> Meshes;
			std::map<Materials, SE::Utilz::GUID> Materials;


			static const char id_Props    = 137;
			static const char id_Torch    = 203;
			static const char id_Floor    = 0;
			static const char id_DeadArea = 76;
			static const char id_Door1    = 22;
			static const char id_Door2    = 48;
			static const char id_Wall     = 255;
			static const char id_Pillar   = 225;
			static const char id_Bush     = 13;
			static const char id_Tree	  = 35; 
			static const char id_Window = 180;
			
			/*Needed:
			 * Representation of the room module(s) that build the room
			 * The enemies that are represented in the room
			 * FlowField map and calculations
			 * Function(s) to build the room
			 */	

			struct LinePoint
			{
				float x, y;

				LinePoint(float xPos, float yPos)
				{
					x = xPos;
					y = yPos;
				}
			};

		public:
			void ToggleRenderingOfWallsAndFloor(bool toggle);
			enum class DirectionToAdjacentRoom
			{

				DIRECTION_ADJACENT_ROOM_NORTH,	/**<The room lies to the North (0) */
				DIRECTION_ADJACENT_ROOM_EAST,	/**<The room lies to the East (1) */
				DIRECTION_ADJACENT_ROOM_SOUTH,	/**<The room lies to the South (2) */
				DIRECTION_ADJACENT_ROOM_WEST,	/**<The room lies to the West (3) */
				DIRECTION_ADJACENT_ROOM_NONE	/**<There is no room (2) */

			};

			void CloseDoor(DirectionToAdjacentRoom DoorNr);
			void OpenDoor(DirectionToAdjacentRoom DoorNr);
			/*@brief store values from raw file*/
			/*@warning may replace "char map" ????*/

			/**
			* @brief	Reverse the direction for the enumerator that defines the connection between two rooms.
			*
			* @details	This help function is used to reverse the enumerator that defines the connection between two rooms.
			* Mainly, this is used when we need to update the flow fields in adjacent rooms; since the connection from
			* current room -> adjacent room is the reverse of adjacent room -> current room, this function can be called
			* on the direction the current room has defined for the connection.
			*
			* For instance, if we send in DIRECTION_ADJACENT_ROOM_NORTH to this function, we will get
			* DIRECTION_ADJACENT_ROOM_SOUTH back.
			*
			* @param[in] currentDirection The direction for the connection that we want to know the reverse of.
			*
			*
			* @retval DIRECTION_ADJACENT_ROOM_NORTH The reverse of the inputed direction is North.
			* @retval DIRECTION_ADJACENT_ROOM_EAST	The reverse of the inputed direction is East.
			* @retval DIRECTION_ADJACENT_ROOM_SOUTH The reverse of the inputed direction is South.
			* @retval DIRECTION_ADJACENT_ROOM_WEST	The reverse of the inputed direction is West.
			*
			* @warning If the enum "DirectionToAdjacentRoom" is modified, this function will no longer work!
			*
			* Example code:
			* @code
			* for(int i = 0; i < 4; i++)
			* {
			*	DirectionToAdjacentRoom directionToRoom = DirectionToAdjacentRoom(i); //Works because the enum is 0->3
			*	if(adjacentRooms[directionToRoom])
			* 		adjacentRooms[directionToRoom]->UpdateFlowField(ReverseDirection(directionToRoom));
			* }
			* @endcode
			*/
			inline static DirectionToAdjacentRoom ReverseDirection(DirectionToAdjacentRoom currentDirection)
			{
				return DirectionToAdjacentRoom((int(currentDirection) + 2) % 4);
			}
			inline Core::Entity GetEntity()const
			{
				return roomEntity;
			}
			inline std::vector<EnemyUnit*> GetEnemiesInRoom()const
			{
				return enemyUnits;
			}
		protected:

			struct DoorData
			{
				float posX;
				float posY;
				size_t tileX;
				size_t tileY;
				bool active = false;
			};
			DoorData DoorArr[4];

			char tileValues[25][25];
			std::vector<Core::Entity> roomEntities[25][25];
			Core::Entity roomEntity;
			SE::Utilz::GUID wallTexture;
			SE::Utilz::GUID floorTexture;

			virtual void UpdateHpBars(float playerX, float playerY);

			/**
			* @brief	Update the Flowfield of a room, given a point that should be used for attraction.
			*
			* @details	Update the Flowfield of a certain room (Always the "current" room!), given a point (character position) that should be used
			* as center of attraction.
			*
			* This function is used once a frame (may be modified!, update this section to reflect that in that case) to make sure that the enemies
			* can move towards the player.
			*
			* @param[in] playerX The x position of the player
			* @param[in] playerY The y position of the player
			*
			* @retval void No return value
			*
			* @warning Due to this function not being implemented ("Cell/Point" not defined), the param[in] macro
			* will not work and thus hasn't been documented!
			*
			* Example code:
			* @code
			*	To be written
			* @endcode
			*/
			void UpdateFlowField(float playerX, float playerY);

			/**
			* @brief	Update the Flowfield of a room, given the direction of the exit that should be used as point for the flowfield.
			*
			* @details	This function is used to update the flow field of adjacent rooms to the current room. This function should be called
			* ONCE, when the player enters a new room and this room becomes the new "current".
			*
			* See UpdateFlowField for more information.
			*
			* @param[in] DirectionToAdjacentRoom The direction that the room to be UPDATED stores the CURRENT room in (the reverse from current room)
			*
			* @retval void No return value
			*
			* @warning Please note that the REVERSED direction is used. This is because the exit that will be used as a point for flowfield calculation
			* exists in the context of the room that the flowfield is calculated in.
			*
			* Example code:
			* @code
			*	To be written
			* @endcode
			*/
			void UpdateFlowField(DirectionToAdjacentRoom exit);

			

			/**
			* @brief	Update all the AIs in the room
			*
			* @details	This function is used to update all the AIs in a room. This will be used in two situations;
			* update the AI's in the current room, and to update the AI's in adjacent rooms.
			*
			* Not decided: A bool that turns animation updates off, as to make sure that animations in a room that will never be seen isn't done.
			*
			* @param[in] dt DeltaTime for the current frame
			*
			* @retval void No return value
			*
			* @warning Due to this function not being implemented (bool animations not decided), the param[in] macro
			* will not work and thus hasn't been documented!
			*
			* Example code:
			* @code
			for(int i = 0; i < 4; i++)
			* {
			*	DirectionToAdjacentRoom directionToRoom = DirectionToAdjacentRoom(i); //Works because the enum is 0->3
			*	if(adjacentRooms[directionToRoom])
			* 		adjacentRooms[directionToRoom]->UpdateAIs(...);
			* }
			* @endcode
			*/
			void UpdateAIs(float dt);

			/**
			* @brief	Function to gather all "Adjacent rooms" updates in one function.
			*
			* @details	Helperfunction used to gather all updates for adjacent rooms into a single function.
			* Should be part of the Update() function.
			* 
			* @param[in] dt DeltaTime for the current frame
			*
			* @retval void No return value
			*
			* @warning Might need to be revisited later; this function might become troublesomely big.
			*
			*/
			void UpdateAdjacentRooms(float dt);

			/**
			* @brief	Helper function for line intersection
			*/
			//bool OnSegment(float pX, float pY, float qX, float qY, float rX, float rY);
			bool OnSegment(LinePoint p, LinePoint q, LinePoint r);

			/**
			* @brief	Helper function for line intersection
			*/
			//int Orientation(float pX, float pY, float qX, float qY, float rX, float rY);
			int Orientation(LinePoint p, LinePoint q, LinePoint r);

			/**
			* @brief	Function for checking if a line p1, p2 intersects another line q1,q2
			*/
			//bool LineCollision(float p1X, float p1Y, float p2X, float p2Y, float q1X, float q1Y, float q2X, float q2Y);
			bool LineCollision(LinePoint p1, LinePoint q1, LinePoint p2, LinePoint q2);

			/**
			* @brief	Function for checking if a projectile has hit any wall
			*/
			void ProjectileAgainstWalls(Projectile& projectile);

			int PointCollisionWithEnemy(float x, float y, Projectile& projectile);

			/**
			* @brief	Function for checking if a projectile has hit any enemy
			*/
			virtual bool ProjectileAgainstEnemies(Projectile& projectile);

			/**
			* @brief	Function for loding in a raw file to the rooms
			*/
			void loadfromFile(Utilz::GUID fileName); 

			/**
			 * @brief Check if a point is inside a wall
			 */
			inline bool PointInsideWall(float x, float y);

			/**
			* @brief Creates enteties for the room 
			*/
			void CreateEntities();

			/**
			* @brief Places enemies in the room on free tiles
			*/
			void CreateEnemies();
			
			/**
			* @brief Creates wall ent for the room
			*/
			bool CreateWall(SE::Core::Entity ent, int x, int y);

			void RandomizeWallAndFloorTexture(SE::Utilz::GUID &wallGuid, SE::Utilz::GUID &floorGuid);

		public:

			Room(Utilz::GUID fileName);
			virtual ~Room();

			void Load();
			void Unload();

			void InitializeAdjacentFlowFields();

			float FloorCheck(int x, int y); 
			
			/**
			* @brief Sets the enteties in the room to render or not
			*/
			void RenderRoom(bool render);

			/**
			* @brief	This function is used to see if the player should transition between rooms
			*
			* @details	This function will check if the player is close enough to a door, and if the user clicked close enough to the door with the mouse
			* if they are then a corresponding value will be returned indicating what room to transition into
			*
			*
			* @retval What direction the room to transition into is, if DIRECTION_ADJACENT_ROOM_NONE then no transition should be done
			*
			*/
			DirectionToAdjacentRoom CheckForTransition(float playerX, float playerY);

			/**
			* @brief	This function will allow the user to add a reference to an adjacent room into this room.
			*
			* @details	Adds a refernce to an adjacent room to the specified room's adjacency list. This list will later be used
			* when decisions about moving the player and enemies between rooms, but also to update flowfields and enemies in
			* adjacent rooms.
			*
			* Note: Only one room can be placed in each direction!
			* Note: Room is not RESPONSIBLE for the rooms placed in it's adjacency list.
			* Note: If room A is adjacent to room B, then room B is adjacent to room A. This means that A needs to be in B's list, and B be in A's.
			*
			*
			* @param[in] direction The direction (noted by an DirectionToAdjacentRoom enum) to place the new room in.
			* @param[in] roomToAdd A pointer to the room. This will be placed in the adjacency list.
			*
			* @retval void No return value.
			*
			* @warning Only one room can be placed in each direction. This means that any room already placed
			* in that direction will be overwritten.
			*
			* Example code:
			* @code
			*	Room* firstRoom = new Room();
			*	Room* secondRoom = new Room();
			*
			*	firstRoom->AddAdjacentRoomByDirection(DirectionToAdjacentRoom::ADJACENT_ROOM_NORTH, secondRoom);
			*	secondRoom->AddAdjacentRoomByDirection(DirectionToAdjacentRoom::ADJACENT_ROOM_SOUTH, firstRoom);
			* @endcode
			*/
			inline void AddAdjacentRoomByDirection(DirectionToAdjacentRoom direction, Room* roomToAdd)
			{
				DoorArr[size_t(direction)].active = true;
				tileValues[DoorArr[size_t(direction)].tileX][DoorArr[size_t(direction)].tileY] = id_Door1;
				adjacentRooms[size_t(direction)] = roomToAdd;
			}

			/**
			* @brief	This function is used to add an enemy to the room.
			*
			* @details	This function can be used to add an enemy to a specific room either during room construction
			* or when an enemy moves between two rooms. The flag "ignorePowerLevel" can be used to make sure that
			* an enemy is added to a room, no matter the current powerlevel of that room.
			*
			* @param[in] enemyToAdd The enemy to be added into the room.
			* @param[in] exitDirection The direction the enemy LEFT the previous room through.
			*
			* @retval true The enemy has been added to the room
			* @retval false The powerlevel of the room is to high for the current enemy to be added to the room.
			*
			* @warning Note that the room WILL take ownership of the AIs it contains. This means that they will delete them!
			*
			* Example code:
			* @code
			*	To be added when function is implemented
			* @endcode
			*/
			bool AddEnemyToRoom(SE::Gameplay::EnemyUnit *enemyToAdd, DirectionToAdjacentRoom exitDirection);

			/**
			* @brief	This function is used to add an enemy to the room.
			*
			* @details	This function can be used to add an enemy to a specific room either during room construction
			* or when an enemy moves between two rooms. The flag "ignorePowerLevel" can be used to make sure that
			* an enemy is added to a room, no matter the current powerlevel of that room.
			*
			* @param[in] enemyToAdd The enemy to be added into the room.
			*
			* @retval true The enemy has been added to the room
			* @retval false The powerlevel of the room is to high for the current enemy to be added to the room.
			*
			* @warning Note that the room WILL take ownership of the AIs it contains. This means that they will delete them!
			*
			* Example code:
			* @code
			*	To be added when function is implemented
			* @endcode
			*/
			bool AddEnemyToRoom(SE::Gameplay::EnemyUnit *enemyToAdd);
			void RemoveEnemyFromRoom(SE::Gameplay::EnemyUnit *enemyToRemove);

			inline const FlowField *GetFlowFieldMap() const
			{
				return roomField;
			};

			inline Room* GetAdjacentRoomByDirection(DirectionToAdjacentRoom direction) const
			{
				return adjacentRooms[int(direction)];
			}

			/**
			* @brief	Sets the reference paramters to the position of the door, the return value indicates if the door is active or not
			*/
			bool GetPositionOfActiveDoor(DirectionToAdjacentRoom door, float &posX, float &posY);

			/**
			* @brief	Update the room
			*
			* @details	Update the room and the adjacent rooms. This includes AIs, Flowfields, and any
			* logic that needs to be updated.
			*
			* @param[in] dt DeltaTime for the current frame
			* @param[in] playerX The x position of the player
			* @param[in] playerY The y position of the player
			*
			* @retval void No return value
			*
			*/
			virtual void Update(float dt, float playerX, float playerY);

			/**
			* @brief	Check for collision (2D-Plan)
			*
			* @details	Given a 2D collision box, will the object collide with any static object in the room
			* 
			* For testing of movement collision were a collision vector is needed, see below function.
			*
			* @param[in] xCenterPosition The x position of the center for the collision box
			* @param[in] yCenterPosition The y position of the center for the collision box
			* @param[in] xExtent The xExtent for the 2D collision box
			* @param[in] yExtent The yExtent for the 2D collision box
			*
			* @retval false No collision occours
			* @retval true Collision occours 
			*
			*/
			bool CheckCollisionInRoom(float xCenterPosition, float yCenterPosition, float xExtent, float yExtent);

			/**
			* @brief	Check for collision (2D-Plane)
			*
			* @details	Given a 2D collision box, will the object collide with any static object in the room
			*
			* For testing of movement collision were a collision vector is needed, see below function.
			*
			* @param[in] xCenterPositionBefore The x position of the center before the movement
			* @param[in] yCenterPositionBefore The y position of the center before the movement
			* @param[in] xCenterPositionAfter The x position of the center after the movement
			* @param[in] yCenterPositionAfter The y position of the center after the movement
			* @param[in] xExtent The xExtent for the 2D collision box
			* @param[in] yExtent The yExtent for the 2D collision box
			* @param[out] xCollision -1 indicates left collision, 1 right
			* @param[out] yCollision -1 indicates down collision, 1 up
			*
			*
			* @retval false No collision occours
			* @retval true Collision occours
			*
			*/
			bool CheckCollisionInRoom(float xCenterPositionBefore, float yCenterPositionBefore, float xCenterPositionAfter, float yCenterPositionAfter, float xExtent, float yExtent, int &xCollision, int &yCollision);

			/**
			* @brief	Checks the wall to rotate the torch in that direction
			*/
			float WallCheck(int x, int y, float &retX, float &retZ);

			/**
			* @brief	Generates random props
			*/
			Prop GenerateRandomProp(int x, int y, CreationArguments &args, float &rot);

			/**
			* @brief
			*/
			void CreateBush(CreationArguments &args);

			/**
			* @brief
			*/
			void CreateTree(CreationArguments &args);

			/**
			* @brief	
			*/
			void CreateFloor(CreationArguments &args);
			/**
			* @brief
			*/
			void CreateTorch(CreationArguments &args);
			/**
			* @brief
			*/
			void CreatePillar(CreationArguments &args);
			/**
			* @brief
			*/
			void CreateProp(CreationArguments &args);
			/**
			* @brief	Code for creating the actual walls, not the calculations. Not to be confused with createWalls() ! 
			*/
			void CreateWall2(CreationArguments &args);
			/**
			* @brief
			*/
			void CreateDoor(CreationArguments &args);
			/**
			* @brief	Checks collision for the projectiles against both the walls and the enemies
			*/
			void CheckProjectileCollision(std::vector<Projectile>& projectiles);

			/**
			* @brief	finds the closest enemy to xPos and yPos nad sets xReturn and yReturn to that enemies position, if no enemies exist then false is returned
			*/
			virtual bool GetClosestEnemy(float xPos, float yPos, float& xReturn, float& yReturn);
			bool GetClosestEnemy(float xPos, float yPos, EnemyUnit* &closestUnit);

			/**
			 * @brief Check if line of sight is blocked between two points
			 */
			bool CheckLineOfSightBetweenPoints(float startX, float startY, float endX, float endY) const;

			/**
			 * @brief Get distance to closest wall (VERY GREEDY AS OF NOW!)
			 */
			float DistanceToClosestWall(float startX, float startY, float &distX, float &distY);

			/**
			 * @brief Get distance to all enemies
			 */
			void DistanceToAllEnemies(float startX, float startY, std::vector<float> &returnVector);

			/**
			 * @brief Get distance to closest (open) door
			 */
			float DistanceToClosestDoor(float startX, float startY, DirectionToAdjacentRoom &direction) const;

			float DistanceToDoorInDirection(float startX, float startY, DirectionToAdjacentRoom direction) const;

			/**
			* @brief Resets the tilevalues from 100 to 0
			*
			* @details When we find two tiles next to each other we create an object for both of those tiles even though we are still on only one of the tiles.
			* We then need to set that other tile value to 100. This is a temporary number so it gets ignored on the next loop though the tileValues array.
			*/
			void ResetTempTileValues();

			/**
			* @brief Checks if a prop is against a wall
			@details Uses the x and y coordinates to check for walls on either side. Dir is the axis you want to want to look for walls.
			* Returns true if walls are found. Also sets appropriate rotation.
			*/
			bool CheckPropAgainstWall(int x, int y, int propId, std::string dir, float & rot);
		
			float RotatePainting(int x, int y);

			void CreateFire(int x, int y);

			bool TorchOnWall(int x, int y);

			void CreateWindows(CreationArguments &args);
			/**
			* @brief set Room door pointer to values
			*/
			
			inline void GetMap(char toReturn[25][25])
			{
				for (int i = 0; i < 25; i++)
				{
					for (int j = 0; j < 25; j++)
					{
						toReturn[i][j] = tileValues[i][j] == id_Torch ? id_Floor : tileValues[i][j];
					}
				}
			}

			bool IsWall(int x, int y) const;

			bool beingRendered = false;
			bool loaded = false;
			inline int NumberOfEnemiesInRoom() { return enemyUnits.size(); };
		};

	}
}


#endif
