#include "WindowSDL.h"
#include <SDL2/SDL_syswm.h>
#include <exception>
#include <Profiler.h>
#include <random>
#include <string>
#include <ctime>
#include <iostream>
#include <experimental\filesystem>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")


SE::Window::WindowSDL::WindowSDL() : window(nullptr), width(1280), height(720), fullScreen(false), windowTitle(""), hwnd(nullptr), curMouseX(0), curMouseY(0), relMouseX(0), relMouseY(0)
{
	currentFrameStrategy = &WindowSDL::RegFrame;
}

SE::Window::WindowSDL::~WindowSDL()
{
}

int SE::Window::WindowSDL::Initialize(const InitializationInfo& info)
{
	StartProfile;
	width = info.width;
	height = info.height;
	fullScreen = info.fullScreen;
	windowTitle = info.windowTitle;
	PlaybackFile = info.file;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		throw std::exception("Failed to initialize SDL subsystem");
	uint32_t createFlags = SDL_WINDOW_SHOWN | (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, createFlags);
	if (window == nullptr)
		throw std::exception("Failed to create window.");

	SDL_SysWMinfo sdlInfo;
	SDL_VERSION(&sdlInfo.version);
	SDL_GetWindowWMInfo(window, &sdlInfo);
	hwnd = sdlInfo.info.win.window;

	/* Annoying to type abstraction of SDL... */
	keyMapping = {
		{ KeyCode::Key0, SDLK_0},
		{ KeyCode::Key1, SDLK_1 },
		{ KeyCode::Key2, SDLK_2 },
		{ KeyCode::Key3, SDLK_3 },
		{ KeyCode::Key4, SDLK_4 },
		{ KeyCode::Key5, SDLK_5 },
		{ KeyCode::Key6, SDLK_6 },
		{ KeyCode::Key7, SDLK_7 },
		{ KeyCode::Key8, SDLK_8 },
		{ KeyCode::Key9, SDLK_9 },
		{ KeyCode::KeyA, SDLK_a },
		{ KeyCode::KeyB, SDLK_b },
		{ KeyCode::KeyC, SDLK_c },
		{ KeyCode::KeyD, SDLK_d },
		{ KeyCode::KeyE, SDLK_e },
		{ KeyCode::KeyF, SDLK_f },
		{ KeyCode::KeyG, SDLK_g },
		{ KeyCode::KeyH, SDLK_h },
		{ KeyCode::KeyI, SDLK_i },
		{ KeyCode::KeyJ, SDLK_j },
		{ KeyCode::KeyK, SDLK_k },
		{ KeyCode::KeyL, SDLK_l },
		{ KeyCode::KeyM, SDLK_m },
		{ KeyCode::KeyN, SDLK_n },
		{ KeyCode::KeyO, SDLK_o },
		{ KeyCode::KeyP, SDLK_p },
		{ KeyCode::KeyQ, SDLK_q },
		{ KeyCode::KeyR, SDLK_r },
		{ KeyCode::KeyS, SDLK_s },
		{ KeyCode::KeyT, SDLK_t },
		{ KeyCode::KeyU, SDLK_u },
		{ KeyCode::KeyV, SDLK_v },
		{ KeyCode::KeyW, SDLK_w },
		{ KeyCode::KeyX, SDLK_x },
		{ KeyCode::KeyY, SDLK_y },
		{ KeyCode::KeyEscape, SDLK_ESCAPE},
		{ KeyCode::KeyF1, SDLK_F1 },
		{ KeyCode::KeyF2, SDLK_F2 },
		{ KeyCode::KeyF3, SDLK_F3 },
		{ KeyCode::KeyF4, SDLK_F4 },
		{ KeyCode::KeyF5, SDLK_F5 },
		{ KeyCode::KeyF6, SDLK_F6 },
		{ KeyCode::KeyF7, SDLK_F7},
		{ KeyCode::KeyF8, SDLK_F8 },
		{ KeyCode::KeyF9, SDLK_F9 },
		{ KeyCode::KeyF10, SDLK_F10 },
		{ KeyCode::KeyF11, SDLK_F11 },
		{ KeyCode::KeyF12, SDLK_F12 },
		{ KeyCode::KeySpace, SDLK_SPACE },
		{ KeyCode::KeyLeft, SDLK_LEFT},
		{ KeyCode::KeyRight, SDLK_RIGHT },
		{ KeyCode::KeyUp, SDLK_UP },
		{ KeyCode::KeyDown, SDLK_DOWN },
		{ KeyCode::KeyBackSpace, SDLK_BACKSPACE},
		{ KeyCode::KeyTab ,SDLK_TAB},
		{ KeyCode::KeyReturn, SDLK_RETURN},
		{ KeyCode::KeyCapsLock, SDLK_CAPSLOCK},
		{ KeyCode::KeyShiftL,   SDLK_LSHIFT},
		{ KeyCode::KeyCtrlL,	   SDLK_LCTRL},
		{ KeyCode::KeyAltL,	   SDLK_LALT},
		{ KeyCode::KeyAltR,	   SDLK_RALT},
		{ KeyCode::KeyCtrlR,	   SDLK_RCTRL},
		{ KeyCode::KeyShiftR,   SDLK_RSHIFT},
		{ KeyCode::MouseLeft, SDL_BUTTON_LEFT},
		{ KeyCode::MouseRight, SDL_BUTTON_RIGHT}
	};

	if (info.winState == WindowState::Record)
	{
		StartRecording();
	}
	else if (info.winState == WindowState::Playback)
	{
		LoadRecording(PlaybackFile);
	}
	else
	{
		std::srand(std::time(nullptr));
	}

	frame = 0;
	time.Tick();
	ProfileReturnConst(0);
}

void SE::Window::WindowSDL::Shutdown()
{
	StopRecording();
	if (playRecord.playback)
	{
		playRecord.playbackData.clear();
	}
	SDL_Quit();
}

void SE::Window::WindowSDL::Frame()
{
	(*this.*currentFrameStrategy)();
}

void SE::Window::WindowSDL::RegFrame()
{
	StartProfile;
	time.Tick();
	for(auto& ks : actionToKeyState)
	{
		ks.second = (ks.second & KeyState::DOWN);
	}
	SDL_Event ev;
	while(SDL_PollEvent(&ev))
	{
		for (auto& onEvent : onEventCallbacks)
			onEvent(&ev, SE::Window::WindowImplementation::WINDOW_IMPLEMENTATION_SDL);
		EventSwitch(ev);
	}
	StopProfile;
}

void SE::Window::WindowSDL::RecordFrame()
{
	StartProfile;
	time.Tick();
	for (auto& ks : actionToKeyState)
	{
		ks.second = (ks.second & KeyState::DOWN);
	}
	SDL_Event ev;
	inputRecData inData;
	inData.dTime = time.GetDelta<std::ratio<1, 1>>();
	while (SDL_PollEvent(&ev))
	{
		for (auto& onEvent : onEventCallbacks)
			onEvent(&ev, SE::Window::WindowImplementation::WINDOW_IMPLEMENTATION_SDL);
		EventSwitch(ev);
		inData.events.push_back(ev);
	}
	inData.nrOfEvent = inData.events.size();
	record.circFiFo.push(inData);

	StopProfile;
}

void SE::Window::WindowSDL::PlaybackFrame()
{
	if (!firstFrame)
		frame++;
	else
		firstFrame = false;
	StartProfile;
	for (auto& ks : actionToKeyState)
	{
		ks.second = (ks.second & KeyState::DOWN);
	}

	if (frame < playRecord.playbackData.size())
	for (auto& ev : playRecord.playbackData[frame].events)
	{
		for (auto& onEvent : onEventCallbacks)
			onEvent(&ev, SE::Window::WindowImplementation::WINDOW_IMPLEMENTATION_SDL);
		EventSwitch(ev);
	}	
	
	StopProfile;
}

void SE::Window::WindowSDL::StartRecording()
{
	StartProfile;
	if (record.recordState == false)
	{
		namespace fs = std::experimental::filesystem;
		fs::create_directory("Recordings");
		currentFrameStrategy = &WindowSDL::RecordFrame;
		std::time_t currentTime = std::time(nullptr);
		char timeChar[100];
		std::strftime(timeChar, sizeof(timeChar), "%A %c", std::localtime(&currentTime));
		std::string currentStringTime = timeChar;
		currentStringTime.erase(std::remove(currentStringTime.begin(), currentStringTime.end(), ':'), currentStringTime.end());
		record.recFile.open("Recordings/Recording" + currentStringTime + ".bin", /*std::ofstream::out |*/ std::ofstream::binary | std::ofstream::trunc);
		record.recFile.write(reinterpret_cast<const char*>(&currentTime), sizeof(std::time_t));
		record.recordState = true;
		int posX;
		int posY;
		SDL_GetMouseState(&posX, &posY);
		record.recFile.write(reinterpret_cast<const char*>(&posX), sizeof(int));
		record.recFile.write(reinterpret_cast<const char*>(&posY), sizeof(int));
		std::srand(currentTime);
		record.recThread = std::thread(&Window::WindowSDL::RecordToFile, this);
	}
	StopProfile;
}

void SE::Window::WindowSDL::StopRecording()
{
	if (record.recordState)
	{
		record.recordState = false;
		while (!record.recThread.joinable())
		{

		}
		record.recThread.join();
		record.recFile.close();
	}
}

void SE::Window::WindowSDL::LoadRecording(const std::string& file)
{
	StartProfile;
	playRecord.playbackfile.open(file,/* std::ifstream::in |*/ std::ifstream::binary);
	if (playRecord.playbackfile.is_open())
	{
		std::time_t currentTime;
		playRecord.playbackfile.read((char*)&currentTime, sizeof(std::time_t));
		std::srand(currentTime);
		int posX;
		int posY;
		playRecord.playbackfile.read((char*)&posX, sizeof(int));
		playRecord.playbackfile.read((char*)&posY, sizeof(int));
		SDL_WarpMouseInWindow(window, posX, posY);
		SDL_Event ev;
		char inDT[sizeof(float)];
		while (!playRecord.playbackfile.eof())
		{
			inputRecData tempRecData;
			//playRecord.playbackfile.precision(8);
			playRecord.playbackfile.read(reinterpret_cast<char*>(&tempRecData.dTime), sizeof(float));
			//playRecord.playbackfile.read(inDT, sizeof(float));//  reinterpret_cast<char*>(&tempRecData.dTime), sizeof(float));
			//tempRecData.dTime = std::strtof(inDT, NULL);
			playRecord.playbackfile.read(reinterpret_cast<char*>(&tempRecData.nrOfEvent), sizeof(size_t));
			for (int i = 0; i < tempRecData.nrOfEvent; i++)
			{
				playRecord.playbackfile.read(reinterpret_cast<char*>(&ev), sizeof(SDL_Event));
				tempRecData.events.push_back(ev);
			}
			playRecord.playbackData.push_back(tempRecData);
		}
		playRecord.playbackfile.close();
		playRecord.playback = true;
		currentFrameStrategy = &WindowSDL::PlaybackFrame;

		void OpenDTRead();
	}
	StopProfile;
}

void* SE::Window::WindowSDL::GetHWND()
{
	return static_cast<void*>(hwnd);
}

void * SE::Window::WindowSDL::GetWindowImplementation(WindowImplementation implementation)
{
	_ASSERT(implementation == WindowImplementation::WINDOW_IMPLEMENTATION_SDL);
	return window;
}

bool SE::Window::WindowSDL::ButtonDown(uint32_t actionButton) const
{
	return GetKeyState(actionButton) & DOWN;
}

bool SE::Window::WindowSDL::ButtonPressed(uint32_t actionButton) const
{
	return !(GetKeyState(actionButton) ^ PRESSED) || (GetKeyState(actionButton) & DOUBLE);
}

bool SE::Window::WindowSDL::ButtonUp(uint32_t actionButton) const
{
	return GetKeyState(actionButton) & UP;
}

bool SE::Window::WindowSDL::ButtonDouble(uint32_t actionButton) const
{
	return GetKeyState(actionButton) & DOUBLE;
}

void SE::Window::WindowSDL::GetMousePos(int& x, int& y) const
{
	x = curMouseX;
	y = curMouseY;
}

void SE::Window::WindowSDL::MapActionButton(uint32_t actionButton, KeyCode key)
{
	keyToAction[keyMapping[key]].push_back(actionButton);
}

void SE::Window::WindowSDL::BindMouseClickCallback(uint32_t actionButton, const MouseClickCallback& callback)
{
	actionToMouseClickCallback[actionButton].push_back(callback);
}

void SE::Window::WindowSDL::BindMouseMotionCallback(const MouseMotionCallback& callback)
{
	mouseMotionCallbacks.push_back(callback);
}

void SE::Window::WindowSDL::BindKeyPressCallback(uint32_t actionButton, const KeyCallback& callback)
{
	actionToKeyPressCallback[actionButton].push_back(callback);
}

void SE::Window::WindowSDL::BindKeyDownCallback(uint32_t actionButton, const KeyCallback& callback)
{
	actionToKeyDownCallback[actionButton].push_back(callback);
}

void SE::Window::WindowSDL::BindKeyUpCallback(uint32_t actionButton, const KeyCallback& callback)
{
	actionToKeyUpCallback[actionButton].push_back(callback);
}

void SE::Window::WindowSDL::UnbindCallbacks()
{
	actionToKeyPressCallback.clear();
	actionToKeyDownCallback.clear();
	actionToKeyUpCallback.clear();
	actionToMouseClickCallback.clear();
	mouseMotionCallbacks.clear();
}


uint32_t SE::Window::WindowSDL::GetKeyState(uint32_t actionButton) const
{
	//Find which KeyCode actionbutton is mapped to
	const auto k = actionToKeyState.find(actionButton);
	if (k == actionToKeyState.end())
		return 0;
	return k->second;
}

bool SE::Window::WindowSDL::RegisterOnEventCallback(const OnEventCallback & callback)
{
	/*@TODO Add operator== to delegates so it can check for duplicates. The same callback shouldnt be registered twice unless the user does something he shouldn't.*/
	onEventCallbacks.push_back(callback);
	return true;
}

int SE::Window::WindowSDL::Width() const
{
	int w;
	int h;
	SDL_GetWindowSize(window, &w, &h);
	return w;
	return width;
}

int SE::Window::WindowSDL::Height() const
{
	int w;
	int h;
	SDL_GetWindowSize(window, &w, &h);
	return h;
	return height;
}


bool SE::Window::WindowSDL::SetWindow(int inHeight, int inWidth, bool inFullscreen)
{
	StartProfile;
	bool changed = false;
	if (inHeight != height)
	{
		height = inHeight;
		changed = true;
	}
	if (inWidth != width)
	{
		width = inWidth;
		changed = true;
	}
	fullScreen = inFullscreen;
	uint32_t createFlags = SDL_WINDOW_SHOWN | (fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SDL_SetWindowFullscreen(window, createFlags);

	if (changed == true && fullScreen == false)
	{
		SDL_SetWindowSize(window, width, height);
	}
	/*int w;
	int h;
	SDL_GetWindowSize(window, &w, &h);
	width = w;
	height = h;*/

	ProfileReturn(changed);
}

void SE::Window::WindowSDL::EventSwitch(SDL_Event ev)
{
	switch (ev.type)
	{
		case SDL_KEYUP:	// if type is KeyUp
		{
			const auto state = keyToAction.find(ev.key.keysym.sym);
			if (state != keyToAction.end())	// if key is bound sets its state
			{
				for (auto& k : state->second)
				{
					actionToKeyState[k] = UP;
					actionToKeyStateLastTime[k] = std::chrono::high_resolution_clock::now();
				}
					
			}
				
			break;
		}
		case SDL_KEYDOWN:	// if type is KeyDown
		{
			auto nt = std::chrono::high_resolution_clock::now();

			const auto state = keyToAction.find(ev.key.keysym.sym);
			if (state != keyToAction.end())	// if key is bound sets its state
			{
				for (auto& k : state->second)
				{
					if (!(actionToKeyState[k] & DOWN))
					{
						actionToKeyState[k] = PRESSED;
						
						auto const find = actionToKeyStateLastTime.find(k);
						if (find != actionToKeyStateLastTime.end())
						{
							auto diff = std::chrono::duration<float, std::milli>(nt - actionToKeyStateLastTime[k]).count();
							if (diff < 300)
							{
								actionToKeyState[k] |= DOUBLE;
							}
						}
						
						auto pressCallbacks = actionToKeyPressCallback.find(k);
						if (pressCallbacks != actionToKeyPressCallback.end())
						{
							for (auto& cb : pressCallbacks->second)
								cb();
						}
					}
					auto downCallbacks = actionToKeyDownCallback.find(k);
					if (downCallbacks != actionToKeyDownCallback.end())
					{
						for (auto& cb : downCallbacks->second)
							cb();
					}
				}
				
			}
			break;
		}
		case SDL_MOUSEMOTION:	// if type is MouseMotion
		{
			curMouseX = ev.motion.x;
			curMouseY = ev.motion.y;
			relMouseX = ev.motion.xrel;
			relMouseY = ev.motion.yrel;
			for (auto& cb : mouseMotionCallbacks)
				cb(relMouseX, relMouseY, curMouseX, curMouseY);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:	// if type is MouseButtonDown
		{
			if (ev.button.button == SDL_BUTTON_LEFT)
			{

				mouseLeftDown = true;
			}
			else if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				mouseRightDown = true;
			}
			const auto state = keyToAction.find(ev.button.button);
			if (state != keyToAction.end())
			{
				for (auto& k : state->second)
				if (!(actionToKeyState[k] & DOWN))
				{
					actionToKeyState[k] = PRESSED;
				}
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:	// if type is MouseButtonUP
		{
			const auto state = keyToAction.find(ev.button.button);
			if (state != keyToAction.end())
			{
				for (auto& k : state->second)
				{
					auto mouseClickCallbacks = actionToMouseClickCallback.find(k);
					if (mouseClickCallbacks != actionToMouseClickCallback.end())
					{
						for (auto& cb : mouseClickCallbacks->second)
							cb(curMouseX, curMouseY);
					}
					actionToKeyState[k] = UP;
				}
			
				
					
			}
			if (ev.button.button == SDL_BUTTON_LEFT)
			{
				mouseLeftDown = false;
			}
			else if (ev.button.button == SDL_BUTTON_RIGHT)
			{
				mouseRightDown = false;
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void SE::Window::WindowSDL::RecordToFile()
{
	while (record.recordState || !record.circFiFo.wasEmpty())
	{
		while (!record.circFiFo.wasEmpty())
		{
			const inputRecData& evData = record.circFiFo.top();
			//record.recFile.precision(8);
			record.recFile.write(reinterpret_cast<const char*>(&evData.dTime), sizeof(float));
			record.recFile.write(reinterpret_cast<const char*>(&evData.nrOfEvent), sizeof(size_t));
			record.recFile.write(reinterpret_cast<const char*>(evData.events.data()), sizeof(SDL_Event) * evData.events.size());
			record.circFiFo.pop();
		}
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(10ms);
	}
}

void SE::Window::WindowSDL::ToggleCursor(bool on)
{
	if (on)
		while (ShowCursor(on) < 0){}
	else
		while (ShowCursor(on) >= 0){}
	
}

void SE::Window::WindowSDL::SetWindowTitle(const std::string& title)
{
	SDL_SetWindowTitle(window, title.c_str());
}
