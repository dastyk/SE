//#include <Core/EntityManager.h>
//#include <iostream>
//
//#ifdef _DEBUG
//#pragma comment(lib, "coreD.lib")
//#else
//#pragma comment(lib, "core.lib")
//#endif

//using namespace SE::Core;
//int main(int argc, char** argv)
//{
//
//	EntityManager em;
//
//
//	Entity e[2048];
//
//	for (int i = 0; i < 2048; i++)
//	{
//		e[i] = em.Create();
//	}
//
//	for (int i = 0; i < 1024; i++)
//	{
//		em.Destroy(e[i]);
//	}
//	for (int i = 0; i < 1024; i++)
//	{
//		if (em.Alive(e[i]))
//			std::cout << i << ": " << e[i].id << "\n";
//	}
//	for (int i = 1024; i < 2048; i++)
//	{
//		if (!em.Alive(e[i]))
//			std::cout << "Dead: " << e[i].id << "\n";
//	}
//	for (int i = 0; i < 1024; i++)
//	{
//		e[i] = em.Create();
//	}
//
//	for (int i = 0; i < 2048; i++)
//	{
//		if (!em.Alive(e[i]))
//			std::cout << "Is dead: " << i << "\n";
//	}
//	for (int x = 0; x < 120; x++)
//	{
//		for (int i = 0; i < 2048; i++)
//		{
//			em.Destroy(e[i]);
//		}
//		for (int i = 0; i < 2048; i++)
//		{
//			e[i] = em.Create();
//		}
//	}
//
//	for (int i = 0; i < 2048; i++)
//	{
//		for (int j = i + 1; j < 2048; j++)
//		{
//			if (e[i].id == e[j].id)
//				std::cout << "Duplicate detected\n";
//		}
//	}
//
//
//
//	std::cin.get();
//
//	return 0;
//
//}


//////////  Window Test
#include <Window/InterfaceWindow.h>
#include <Window/Window.h>

#ifdef _DEBUG
#pragma comment(lib, "WindowD.lib")
#else
#pragma comment(lib, "Window.lib")
#endif

using namespace SE::Window;
int main(int argc, char** argv)
{
	InterfaceWindow* window = new Window();

	bool windowOpen = window->Initialise();
	if (windowOpen == true)
	{
		window->MapKeyToKeyboard(0, KeyQ);

		while (window->HandleMSG() == true && window->GetActionKeyState(0) == key_up)
		{

		}

	}
}