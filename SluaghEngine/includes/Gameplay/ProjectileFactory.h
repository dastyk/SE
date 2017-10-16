#ifndef _SE_GAMEPLAY_PROJECTILE_FACTORY_H_
#define _SE_GAMEPLAY_PROJECTILE_FACTORY_H_

#include "GameUnit.h"
#include <Gameplay\Projectile.h>
#include <Gameplay\ProjectileData.h>
#include <Gameplay\Room.h>
#include <Gameplay\PlayerUnit.h>

namespace SE
{
	namespace Gameplay
	{

		/**
		*
		* @brief The class that holds and manages projectiles
		*
		* @details This class holds and updates all the projectiles in the game
		*
		**/

		struct BehaviourPointers
		{
			Room** currentRoom;
			PlayerUnit* player;
		};

		class ProjectileFactory
		{
		private:

			/**
			* @brief	Enum over what a function should be added to (continuous, on collision, on death, etc)
			*/
			enum class TypeOfFunction
			{
				CONTINUOUS,
				ON_COLLISION,
				ON_DEATH
			};

			BehaviourPointers ptrs;

		public:

			/**
			* @brief	Creates a new projectile based on the projectile data and data read from a file created by an editor
			*
			* @param [in] data Data created by the unit from which the need for the projectile originates
			*
			* @retval Projectile The newly created projectile
			*
			*/
			SE::Gameplay::Projectile CreateNewProjectile(ProjectileData data);


		private:
			ProjectileFactory(const ProjectileFactory& other) = delete;
			ProjectileFactory(const ProjectileFactory&& other) = delete;
			ProjectileFactory& operator=(const ProjectileFactory& rhs) = delete;

			/**
			* @brief	Adds bounce behaviour to the projectile
			*/
			void AddBounce(Projectile& projectile, TypeOfFunction type);

			/**
			* @brief	Adds speed changing behaviour to the projectile
			*/
			void AddSpeedModifier(Projectile& projectile, TypeOfFunction type, float speedModifier);

			/**
			* @brief	Adds rotation changing behaviour to the projectile
			*/
			void AddRotationModifier(Projectile& projectile, TypeOfFunction type, float rotationModifier);

			/**
			* @brief	Adds rotation invertion behaviour to the projectile (acts weird, not fully functioning)
			*/
			void AddRotationInvertion(Projectile& projectile, TypeOfFunction type, float intervall);

			/**
			* @brief	Adds lifetime behaviour to the projectile
			*/
			void AddLifeTime(Projectile& projectile, TypeOfFunction type, float timeToIncrease);

			/**
			* @brief	Adds Targeting behaviour to the projectile
			*/
			void AddTargeting(Projectile& projectile, TypeOfFunction type, float rotPerSecond);

			/**
			* @brief	Helper function for adding the behaviour to the correct function vector of the projectile
			*/
			void AddBehaviourToProjectile(Projectile& p, TypeOfFunction type, std::function<bool(Projectile* projectile, float dt)> func);


		public:

			ProjectileFactory();
			~ProjectileFactory();

			inline void SetBehaviourPtrs(BehaviourPointers bPtrs)
			{
				ptrs = bPtrs;
			};

		};

	}
}




#endif
