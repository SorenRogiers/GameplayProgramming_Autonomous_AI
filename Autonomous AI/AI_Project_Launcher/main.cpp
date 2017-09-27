#pragma once
#include "Windows.h"
#include <string>

#include <Box2D/Box2D.h>
#include "Box2D\Common\b2Draw.h"
#include "Box2D\Common\b2Math.h"

#include <GL/gl3w.h>
#include <ImGui/imgui.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include "IBehaviourPlugin.h"


typedef int(*RunFrameworkDll)(HMODULE);

#undef main
int main(int argc, char* argv[])
{
	//Load Plugin Module
	HMODULE pPluginHandle = nullptr;
	pPluginHandle = LoadLibrary(L"AI_Project_Plugin.dll");

	int res =  reinterpret_cast<RunFrameworkDll>(GetProcAddress(pPluginHandle, "RunFrameworkDLL"))(pPluginHandle);

	FreeLibrary(pPluginHandle);

	return res;
}