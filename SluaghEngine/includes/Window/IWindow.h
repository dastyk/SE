#ifndef SE_WINDOW_IWINDOW_H_
#define SE_WINDOW_IWINDOW_H_

#include <cstdint>
#include <string>
#include <Utilz/Delegator.h>
#include <functional>
namespace SE
{
	namespace Window
	{
		enum KeyCode
		{
			KeyEscape,
			KeyF1,
			KeyF2,
			KeyF3,
			KeyF4,
			KeyF5,
			KeyF6,
			KeyF7,
			KeyF8,
			KeyF9,
			KeyF10,
			KeyF11,
			KeyF12,
			KeyF13,
			KeyF14,
			KeyF15,
			KeyF16,
			KeyF17,
			KeyF18,
			KeyF19,
			KeyPrint,
			KeyScrollLock,
			KeyBreak,

			KeySpace = 0x0020,
/*
			KeyApostrophe = 0x0027,
			KeyComma = 0x002c,
			KeyMinus = 0x002d,
			KeyPeriod = 0x002e,
			KeySlash = 0x002f,
			*/
			Key0 = 0x0030,
			Key1 = 0x0031,
			Key2 = 0x0032,
			Key3 = 0x0033,
			Key4 = 0x0034,
			Key5 = 0x0035,
			Key6 = 0x0036,
			Key7 = 0x0037,
			Key8 = 0x0038,
			Key9 = 0x0039,
/*
			KeySemicolon = 0x003b,
			KeyLess = 0x003c,
			KeyEqual = 0x003d,
			*/
			KeyA = 0x0041,
			KeyB = 0x0042,
			KeyC = 0x0043,
			KeyD = 0x0044,
			KeyE = 0x0045,
			KeyF = 0x0046,
			KeyG = 0x0047,
			KeyH = 0x0048,
			KeyI = 0x0049,
			KeyJ = 0x004a,
			KeyK = 0x004b,
			KeyL = 0x004c,
			KeyM = 0x004d,
			KeyN = 0x004e,
			KeyO = 0x004f,
			KeyP = 0x0050,
			KeyQ = 0x0051,
			KeyR = 0x0052,
			KeyS = 0x0053,
			KeyT = 0x0054,
			KeyU = 0x0055,
			KeyV = 0x0056,
			KeyW = 0x0057,
			KeyX = 0x0058,
			KeyY = 0x0059,
			KeyZ = 0x005a,
/*
			KeyBracketLeft = 0x005b,
			KeyBackslash = 0x005c,
			KeyBracketRight = 0x005d,

			KeyGrave = 0x0060,
			*/
			KeyLeft,
			KeyRight,
			KeyUp,
			KeyDown,
			/*
			KeyInsert,
			KeyHome,
			KeyDelete,
			KeyEnd,
			KeyPageUp,
			KeyPageDown,
			
			KeyNumLock,
			KeyKpEqual,
			KeyKpDivide,
			KeyKpMultiply,
			KeyKpSubtract,
			KeyKpAdd,
			KeyKpEnter,
			KeyKpInsert, // 0
			KeyKpEnd, // 1
			KeyKpDown, // 2
			KeyKpPageDown, // 3
			KeyKpLeft, // 4
			KeyKpBegin, // 5
			KeyKpRight, // 6
			KeyKpHome, // 7
			KeyKpUp, // 8
			KeyKpPageUp, // 9
			KeyKpDelete, // ,
			*/
			KeyBackSpace,
			KeyTab,
			KeyReturn,
			KeyCapsLock,
			KeyShiftL,
			KeyCtrlL,
			//KeySuperL,
			KeyAltL,
			KeyAltR,
		//	KeySuperR,
		//	KeyMenu,
			KeyCtrlR,
			KeyShiftR,
/*
			KeyBack,
			KeySoftLeft,
			KeySoftRight,
			KeyCall,
			KeyEndcall,
			KeyStar,
			KeyPound,
			KeyDpadCenter,
			KeyVolumeUp,
			KeyVolumeDown,
			KeyPower,
			KeyCamera,
			KeyClear,
			KeySymbol,
			KeyExplorer,
			KeyEnvelope,
			KeyEquals,
			KeyAt,
			KeyHeadsethook,
			KeyFocus,
			KeyPlus,
			KeyNotification,
			KeySearch,
			KeyMediaPlayPause,
			KeyMediaStop,
			KeyMediaNext,
			KeyMediaPrevious,
			KeyMediaRewind,
			KeyMediaFastForward,
			KeyMute,
			KeyPictsymbols,
			KeySwitchCharset,

			KeyForward,
			KeyExtra1,
			KeyExtra2,
			KeyExtra3,
			KeyExtra4,
			KeyExtra5,
			KeyExtra6,
			KeyFn,

			KeyCircumflex,
			KeySsharp,
			KeyAcute,
			KeyAltGr,
			KeyNumbersign,
			KeyUdiaeresis,
			KeyAdiaeresis,
			KeyOdiaeresis,
			KeySection,
			KeyAring,
			KeyDiaeresis,
			KeyTwosuperior,
			KeyRightParenthesis,
			KeyDollar,
			KeyUgrave,
			KeyAsterisk,
			KeyColon,
			KeyExclam,

			KeyBraceLeft,
			KeyBraceRight,
			KeySysRq,
			*/
			MouseLeft,
			MouseRight,
			KeyCount_
		};

		
		typedef Utilz::Delegate<void(int x, int y)> MouseClickCallBack;
		typedef Utilz::Delegate<void(int xRelative, int yRelative, int xPos, int yPos)> MouseMotionCallBack;
		typedef std::function<void()> KeyCallback;
		/**
		*
		* @brief The interface to any concrete window class. The window interface handles the window as well as messages sent to window. In other words it also handles keyboard/mouse input.
		*
		*
		*
		* @sa WindowSDL
		*
		**/
		class IWindow
		{
		public:
			struct InitializationInfo
			{
				std::string windowTitle;
				bool fullScreen = false;
				uint32_t width = 1280;
				uint32_t height = 720;
			};

			IWindow(){};
			virtual ~IWindow(){};

			/**
			* @brief Initializes the window based on the information in the struct sent to it. If no parameter is sent, default values will be used
			* @retval return_value_0 Returns 0 on success.
			* @sa InitializationInfo
			*/
			virtual int Initialize(const InitializationInfo& info = InitializationInfo()) = 0;

			/**
			* @brief Destroys the window
			*/
			virtual void Shutdown() = 0;

			/**
			* @brief Polls for input
			*/
			virtual void Frame() = 0;

			/**
			* @brief Returns a pointer to the window handle
			*/
			virtual void* GetHWND() = 0;

			/**
			* @brief Checks whether or no the bound action button is down or not. An action button must be bound with MapActionButton before this method is called
			* @retval true Returns true if the button is down
			* @retval false Returns false if the button is not down.
			* @sa MapActionButton
			*/
			virtual bool ButtonDown(uint32_t actionButton) const = 0;
			/**
			* @brief Checks whether or no the bound action button was pressed during this frame or not, i.e. if the button is down this frame but was not down last frame.
			* @retval true Returns true if the button was pressed.
			* @retval false Returns false if the button was not pressed.
			* @sa MapActionButton
			*/
			virtual bool ButtonPressed(uint32_t actionButton) const = 0;
			/**
			* @brief Checks whether or no the bound action button was lifted this frame
			* @retval true Returns true if the button was lifted
			* @retval false Returns false if the button was not lifted
			* @sa MapActionButton
			*/
			virtual bool ButtonUp(uint32_t actionButton) const = 0;

			
			virtual void BindMouseClickCallback(uint32_t actionButton, const MouseClickCallBack& callback) = 0;
			virtual void BindMouseMotionCallback(const MouseMotionCallBack& callback) = 0;
			virtual void BindKeyCallback(uint32_t actionButton, const KeyCallback& callback) = 0;

			/**
			* @brief Maps an action button to a certain key. Several action buttons can be mapped to the same key but an action button cannot be mapped to several keys. An action button is any user defined key represented as an unsigned integer.
			* * Example code:
			* @code
			*   enum{
			*       JUMP,
			*       SHOOT
			*   };
			*   
			*   window.MapActionButton(JUMP, KeySpace);
			*   window.Frame();
			*   if(window.ButtonDown(JUMP))
			*        character.jump();
			* @endcode
			*/
			virtual void MapActionButton(uint32_t actionButton, KeyCode key) = 0;

		private:

		};


	}
}


#endif

