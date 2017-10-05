#ifndef SE_WINDOW_WINDOWSDL_H_
#define SE_WINDOW_WINDOWSDL_H_
#include "IWindow.h"
#include <SDL2/SDL.h>
#include <Windows.h>
#include <map>
#include <vector>
#include <Utilz\CircularFiFo.h>
#include <fstream>
#include <thread>
#include <Utilz\Delegate.h>
namespace SE
{
	namespace Window
	{
		
		class WindowSDL : public IWindow
		{
		public:
			WindowSDL();
			~WindowSDL();

			int Initialize(const InitializationInfo& info) override;
			void Shutdown() override;

			void Frame() override;
			void RegFrame();
			void RecordFrame();
			void PlaybackFrame();

			

			void* GetHWND() override;

			bool ButtonDown(uint32_t actionButton) const override;
			bool ButtonPressed(uint32_t actionButton) const override;
			bool ButtonUp(uint32_t actionButton) const override;

			void GetMousePos(int& x, int& y) const override;

			void MapActionButton(uint32_t actionButton, KeyCode key) override;


			void BindMouseClickCallback(uint32_t actionButton, const MouseClickCallback& callback) override;
			void BindMouseMotionCallback(const MouseMotionCallback& callback) override;
			void BindKeyPressCallback(uint32_t actionButton, const KeyCallback& callback) override;
			void BindKeyDownCallback(uint32_t actionButton, const KeyCallback& callback) override;
			void BindKeyUpCallback(uint32_t actionButton, const KeyCallback& callback) override;
			void UnbindCallbacks() override;
		
			bool SetWindow(int height, int width, bool inFullscreen) override;
		private:
			

			/*Window related things*/
			SDL_Window* window;
			uint32_t width;
			uint32_t height;
			bool fullScreen;
			std::string windowTitle;
			HWND hwnd;

			/*input related things*/
			int curMouseX;
			int curMouseY;
			int relMouseX;
			int relMouseY;

			bool mouseLeftDown;
			bool mouseRightDown;

			
			std::map<uint32_t, uint32_t> keyToAction;
			/**<Maps generic keys defined in IWindow.h to key codes of SDL implementation*/
			std::map<KeyCode, uint32_t> keyMapping;
			
			std::map<uint32_t, std::vector<KeyCallback>> actionToKeyPressCallback;
			std::map<uint32_t, std::vector<KeyCallback>> actionToKeyDownCallback;
			std::map<uint32_t, std::vector<KeyCallback>> actionToKeyUpCallback;
			std::map<uint32_t, std::vector<MouseClickCallback>> actionToMouseClickCallback;
			std::vector<MouseMotionCallback> mouseMotionCallbacks;

			enum KeyState : uint32_t
			{
				NIL = 0,
				DOWN = 1 << 1,
				PRESSED = 3,
				UP = 4
			};
			/**<Maps action button to keystate (up, down, pressed)*/
			std::map<uint32_t, uint32_t> actionToKeyState;
			uint32_t GetKeyState(uint32_t actionButton) const;

			//record stuff
			struct inputRecData
			{
				size_t frame;
				SDL_Event recEvent;
			};

			void RecordToFile();
			std::ofstream recFile;
			std::ifstream playbackfile;
			Utilz::CircularFiFo<inputRecData, 256> circFiFo;
			bool recording = false;
			bool playback = false;
			inputRecData* playbackData;
			std::thread recThread;
			size_t frame;

			size_t arrayPos;

			Utilz::Delegate<void()> actualFrame;

			void StartRecording();
			void LoadRecording();
		};


	}
}



#endif

