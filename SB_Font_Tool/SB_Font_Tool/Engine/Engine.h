#ifndef _ENGINE_
#define _ENGINE_

#pragma comment(lib, "SDL2_image.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glu32.lib")

#include <assert.h>
#include <string>
#include <algorithm>

#include "TaskScheduler.h"
#include "WinsockWrapper/WinsockWrapper.h"
#include "RandomNumbers.h"
#include "InputSystem.h"
#include "SDL_Window.h"
#include "TextureController.h"
#include "DebugConsole.h"
#include "FontController.h"
#include "DialogSystem/MenuGUI.h"
#include "DialogSystem/MenuElementsList.h"
#include "CutsceneSystem.h"

void Setup_OpenGL_Defaults( void );
void Resize_View( unsigned int width, unsigned int height );
bool SDL_ProcessEvents( InputSystem* input );

int InitializeEngine( const int window_width, const int window_height, std::string title );
bool ProcessEventsEngine();
void InputEngine();
void UpdateEngine( const float time_slice );
void PreRender3DEngine();
void Render3DEngine();
void PreRender2DEngine();
void Render2DEngine();
void PostLoopEngine();
void ShutdownEngine();

#endif

//// TO-DO LIST
// - Alter the TextureController texture list to use a hash map to determine if it already contains a texture. This should speed loading and accessing.