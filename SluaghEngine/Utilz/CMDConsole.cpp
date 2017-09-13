
#include <CMDConsole.h>
#include <Windows.h>
#include <cstdarg>
#include <iostream>

SE::Utilz::CMDConsole::CMDConsole()
{
}


SE::Utilz::CMDConsole::~CMDConsole()
{
}

int SE::Utilz::CMDConsole::Initialize()
{
	return 0;
}

void SE::Utilz::CMDConsole::Shutdown()
{
}

void SE::Utilz::CMDConsole::Show()
{
	if (AllocConsole())
	{
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
	}

	Print("<----||Console Initialized||---->\n\n");

}

void SE::Utilz::CMDConsole::Hide()
{
	FreeConsole();
}

void SE::Utilz::CMDConsole::Print(const char * line, ...)
{
	va_list args;
	va_start(args, line);
	vprintf(line, args);
	fflush(stdout);
	va_end(args);
}

void SE::Utilz::CMDConsole::Print(const char * line, va_list args)
{
	vprintf(line, args);
	fflush(stdout);
}

void SE::Utilz::CMDConsole::Getline(std::string * string)
{
	std::getline(std::cin, *string);
}

size_t SE::Utilz::CMDConsole::Getline(const char * buffer, size_t size)
{
	std::string in;
	std::getline(std::cin, in);
	_ASSERT(in.size() + 1 <= size);
	memcpy((void*)buffer, in.c_str(), in.size());
	memcpy((void*)(buffer + in.size()), "\0", 1);
	return in.size();
}
