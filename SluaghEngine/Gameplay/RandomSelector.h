#ifndef SE_GAMEPLAY_RANDOM_SELECTOR_H
#define SE_GAMEPLAY_RANDOM_SELECTOR_H

#include "IComposite.h"

namespace SE
{
	namespace GAMEPLAY
	{
		/**
		*
		* @brief A class that selects and check its child in a random order.
		*
		* @details Inheriting from IComposite, the RandomSelector will shuffle the order the childs are ticked off.
		* If all childs return BEHAVIOUR_FAILURE, so does the RandomSelector.
		*
		* @sa IBehaviour for the basic Behaviour, IComposite for the basic composite class, and Selector for the non-random form of Selector
		*
		**/
		class RandomSelector : public IComposite
		{
			
			private:
				RandomSelector() = delete;
			protected:
				Behaviours::iterator currentChild;

				/**
				* @brief	Initialize the Random Selector for updating.
				*
				* @details	Shuffle the child vector, and sets the first child to "current" child.
				*
				* @retval void No return value
				*
				*
				*/
				virtual void OnInitialization() override;

				/**
				* @brief	Goes through the childs in a random order
				*
				* @details	Goes through the childs in the random order specificed by 
				* OnInitialization. If a child returns anything but BEHAVIOUR_FAILURE,
				* Update will return the same value.
				*
				*
				* @retval BEHAVIOUR_FAILURE The behaviour failed to complete.
				* @retval BEHAVIOUR_RUNNING The behaviour is executing, and may take multiple frames to finish.
				* @retval BEHAVIOUR_SUCCESS The behaviour is finished and succeeded in its execution.
				* @retval BEHAVIOUR_SUSPENDED The behaviour is suspended by another node, and should be checked in on later.
				* @retval BEHAVIOUR_INVALID Something is wrong, since this should only be a "default" state.
				*
				*/
				virtual Status Update() override;

			public:
				RandomSelector(EnemyBlackboard* enemyBlackboard, GameBlackboard* gameBlackboard);
				~RandomSelector();

		};
	}
}


#endif
