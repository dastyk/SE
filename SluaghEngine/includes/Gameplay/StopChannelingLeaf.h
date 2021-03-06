#ifndef SE_GAMEPLAY_STOP_CHANNELING_LEAF_H
#define SE_GAMEPLAY_STOP_CHANNELING_LEAF_H
#include "IBehaviour.h"
#include "EnemyBlackboard.h"

namespace SE
{
	namespace Gameplay
	{
		class StopChannelingLeaf : public IBehaviour
		{
		private:
			StopChannelingLeaf() = delete;
		public:
			StopChannelingLeaf(EnemyBlackboard* enemyBlackboard, GameBlackboard* gameBlackboard) :
				IBehaviour(enemyBlackboard, gameBlackboard)
			{
				myStatus = Status::BEHAVIOUR_SUCCESS;
			};
			~StopChannelingLeaf()
			{

			};

			Status Update() override
			{
				enemyBlackboard->channeling = false;
				return myStatus;
			};

			inline IBehaviour* CopyBehaviour(GameBlackboard* gameBlackboard, EnemyBlackboard* enemyBlackboard) const override
			{
				return new StopChannelingLeaf(enemyBlackboard, gameBlackboard);
			};
		};
	}
}


#endif

