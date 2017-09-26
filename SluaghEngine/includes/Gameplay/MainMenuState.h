#ifndef _SE_GAMEPLAY_MAIN_MENU_STATE_H
#define _SE_GAMEPLAY_MAIN_MENU_STATE_H
#include "IGameState.h"

namespace SE
{
	namespace Gameplay 
	{


		/**
		*
		* @brief The Main Menu Class
		*
		* @details class used for the main menu, more to be added later, inherits from Gamestate baseclass.
		*
		* @warning Not complete
		*
		* @sa GameState.
		*
		**/
		class MainMenuState : public GameState
		{

		public:

			MainMenuState();
			~MainMenuState();

			/**
			* @brief	The update loop for the Main menu process.
			*
			* @details	An update loop to be used during the Main menu phase, more info added later
			*
			* @param[in] Input is a pointer to the Window class which lets us use user input within these update loops
			* @param[in,out] passableInfo is a void pointer which lets us send information inbetween the states.
			*
			* @retval State return value to which other state to switch to when done.
			*
			* @warning Not fully implemented.
			*
			*/
			State Update(Window::IWindow* Input,void* &passableInfo);

		private:

		protected:



		};

	}
}



#endif

