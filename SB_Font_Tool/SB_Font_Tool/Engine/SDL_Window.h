/*****************************************************************/
/*	           ___                          _  _                  */
/*	    	    / _ \                        | |(_)                 */
/*          / /_\ \ _ __   ___   __ _   __| | _   __ _           */
/*    	   |  _  || '__| / __| / _` | / _` || | / _` |          */
/*	         | | | || |   | (__ | (_| || (_| || || (_| |          */
/*	         \_| |_/|_|    \___| \__,_| \__,_||_| \__,_|          */
/*                                                               */
/*                                      Engine Version 01.00.00  */
/*****************************************************************/
/*  File:       SDL_Window.h                                     */
/*                                                               */
/*  Purpose:    This file contains a basic class for opening     */
/*              and managing a window within SDL. The specifics  */
/*              of the window, such as width and height, can be  */
/*              easily altered, and shutdown is made simple.     */
/*                                                               */
/*  Created:    12/07/2008                                       */
/*  Last Edit:  04/18/2009                                       */
/*****************************************************************/

#ifndef _SDLSimpleWindow_
#define _SDLSimpleWindow_

#include <SDL.h>
#undef main
#include <SDL_opengl.h>
#undef main

class SDLSimpleWindow
{
	//~  Member Functions
	private:
		
	public:
		SDLSimpleWindow();
		~SDLSimpleWindow();
		
		bool	Define_Window( int width, int height, int bpp, bool full_screen, const char* title );
		void	SwapBuffers() { SDL_GL_SwapWindow(m_Window); }
		
		// Accessors
		inline int				Get_Height( void )		const	{	return m_nHeight;	}
		inline int				Get_Width( void )		const	{	return m_nWidth;	}

		
	//~ Member Variables
	private:
		int		m_nWidth;
		int		m_nHeight;
		int		m_nBPP;
		bool	m_bFullScreen;
		char	m_szTitle[64];

		SDL_Window*		m_Window;

	public:

};

#endif