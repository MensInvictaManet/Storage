#ifndef _MAIN_PROGRAM_
#define _MAIN_PROGRAM_

class MainProgram
{
public:
	MainProgram();
	const bool Initialize();
	const bool Input( const float time_slice );
	const bool Update( const float time_slice );
	const bool Render3D( void );
	const bool Render2D( void );
	const bool Shutdown();
};

#endif