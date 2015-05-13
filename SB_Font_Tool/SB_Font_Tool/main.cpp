#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		720

#include "Engine\Engine.h"
#include "MainProgram.h"
#include <glut.h>

int main( int argc, char* argv[] )
{
	const GLubyte* gl_error = NULL;
	gl_error = gluErrorString(glGetError());

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	
	gl_error = gluErrorString(glGetError());

	InitializeEngine(WINDOW_WIDTH, WINDOW_HEIGHT, "SuperBingo UI Editor - v01.00.00");
	
	MainProgram mainProgram;
	if ( !mainProgram.Initialize() )
	{
		assert( "Failed to initialize the main program." && false );
		return 1;
	}

	// TEST
	////CutsceneSystem cutscene_system;
	////cutscene_system.Load( "Assets/TestCutscene.xml" );
	
	//  Process any events from the Operating System
	while ( ProcessEventsEngine() )
	{
		// Get the time slice
		static float time_slice;
		static int elapsed_time[2];
		elapsed_time[0] = SDL_GetTicks();
		time_slice = std::max<float>( std::min<float>( float(elapsed_time[0] - elapsed_time[1]) / 1000.0f, 1.0f ), 0.0f );
		elapsed_time[1] = elapsed_time[0];

		//  INPUT
		InputEngine();
		mainProgram.Input( time_slice );
		////cutscene_system.Input();

		// UPDATE
		UpdateEngine( time_slice );
		mainProgram.Update( time_slice );
		////cutscene_system.Update( time_slice );

		// RENDER 3D
		PreRender3DEngine();
		Render3DEngine();
		mainProgram.Render3D();

		// RENDER 2D
		PreRender2DEngine();
		Render2DEngine();
		mainProgram.Render2D();
		////cutscene_system.Render();

		PostLoopEngine();
	}

	//  Shut down systems
	ShutdownEngine();
	mainProgram.Shutdown();
	////cutscene_system.Shutdown();
	
	return 0;
}