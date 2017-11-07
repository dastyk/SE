#ifndef SE_GAMEPLAY_ENEMY_FACTORY_H
#define SE_GAMEPLAY_ENEMY_FACTORY_H

#include <Utilz/GUID.h>
#include <map>
#include <ResourceHandler\IResourceHandler.h>

namespace SE
{
	namespace Gameplay
	{
		class BehaviouralTreeFactory;
		class EnemyUnit;
		struct GameBlackboard;


		enum EnemyType
		{
			ENEMY_TYPE_BODACH,
			ENEMY_TYPE_GLAISTIG,
			ENEMY_TYPE_NUCKELAVEE,
			ENEMY_TYPE_PECH_MELEE,
			ENEMY_TYPE_PECH_RANGED,
			ENEMY_TYPE_RANDOM
		};


		/**
		*
		* @brief The information for the enemy to be created
		*
		* @warning ALL DATA MUST BE SET!
		*
		**/
		struct EnemyCreationData
		{
			EnemyType type;
			float startX, startY;
			bool useVariation;
		};

		struct EnemyCreationStruct
		{
			std::vector<EnemyCreationData> information;
		};

		/**
		*
		* @brief The enemy factory to create enemies through.
		*
		* @warning LoadEnemyIntoMemory MUST be called before CreateEnemy.
		*
		**/
		class EnemyFactory
		{
		private:
			/**
			 * @brief Internal struct to save Enemy Descriptions
			 */
			struct EnemyDescriptionStruct
			{
				Utilz::GUID meshGUID;
				Utilz::GUID skeletonGUID;
				Utilz::GUID behaviouralTreeGUID;
				Utilz::GUID materialGUID;
				Utilz::GUID shaderGUID;
				
				int baseDamage;
				int baseDamageVariation;
				int baseHealth;
				int baseHealthVariation;
				/*Armour type must be added, but doesn't exist in the EventStruct yet.*/
				int physicalResistance;
				int magicalResistance;
				int natureResistance;
				int fireResistance;
				int waterResistance;
			};


			BehaviouralTreeFactory* SEBTFactory;
			std::map<EnemyType, EnemyDescriptionStruct> enemyData;
			std::map<Utilz::GUID, EnemyType, Utilz::GUID::Compare> enemyTypes;

			inline EnemyType GUIDToEnemyType(const Utilz::GUID& GUID)
			{
				return enemyTypes[GUID];
			}

			/**
			* @brief	Load the description of an enemy from file
			*
			* @details	Called from LoadEnemyIntoMemory to parse the enemy data and put into the map.
			*
			* @param[in] GUID The GUID of the Enemy to be parsed.
			* @param[in] data The data recieved from the file.
			* @param[out] size The size of the data recieved from the file.
			*
			*/
			ResourceHandler::InvokeReturn LoadEnemyFromResourceHandler(const Utilz::GUID& GUID, void* data, size_t size);
			
			
		public:



			/**
			* @brief	Create an enemy already loaded into the memory, using the GUID as identifier.
			*
			* @details  Create an enemy from the data previously read into memory. Bind a mesh,
			* set the entity as renderable. Does NOT set a position for the enemy!
			*
			*
			* @param[in] descriptions Descriptions for the enemies to be created
			* @param[in] gameBlackboard The blackboard for the game.
			* @param[out] unitArray The array to put all created units in.
			*
			* @warning ALL DESCRIPTIONS MUST BE COMPLETELY FILLED IN!
			* 
			* @sa EnemyCreationStruct.
			*
			*/
			void CreateEnemies(const EnemyCreationStruct &descriptions, GameBlackboard* gameBlackboard, EnemyUnit** unitArray);

			EnemyFactory();
			~EnemyFactory();

		};
	}
}


#endif