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
/*  File:       TextureController.cpp                            */
/*                                                               */
/*  Purpose:    This file contains a class that keeps track      */
/*              of and allows access to texture allocations.     */
/*              It acts as a single point of access to textures  */
/*              and their functions                              */
/*                                                               */
/*  Created:    01/02/2009                                       */
/*  Last Edit:  04/18/2010                                       */
/*****************************************************************/


#include "TextureController.h"

#include "DebugTools.h"
#include <SDL_image.h>

TextureController::TextureController()
{
	LoadFolder = "";
}

TextureController::~TextureController(void)
{
	FAIL_IF ( TextureList.empty() == false )
	{
		while ( TextureList.empty() == false )
		{
			Release_Texture( (*TextureList.begin()).first );
		}
	}
}

TextureController* TextureController::Get_Instance()
{
	static TextureController INSTANCE;
	return &INSTANCE;
}

int TextureController::Load_Texture(std::string texture_file)
{
	assert( texture_file.size() );

	std::string file_string = LoadFolder;
	file_string.append(texture_file);

	// If you can find a texture that has the same filename, return the ID
	for ( TextureListType::iterator iter = TextureList.begin(); iter != TextureList.end(); ++iter )
	{
		if ((*iter).second.Get_Filename().compare( file_string ) == 0)
		{
			(*iter).second.ReferenceCount += 1;
			return (*iter).first;
		}
	}

	// Attempt to load the texture file. If it can not be loaded, return.
	SDL_Surface* NewSurface = IMG_Load(file_string.c_str());
	FAIL_IF (NewSurface == NULL)
	{
		SDL_FreeSurface(NewSurface);
		return INVALID_IMAGE;
	}
	
	FAIL_IF ( (NewSurface->w & (NewSurface->w - 1)) != 0 && "Texture width is not a power of 2." )
	{
		SDL_FreeSurface(NewSurface);
		return INVALID_IMAGE;
	}

	FAIL_IF ( (NewSurface->h & (NewSurface->h - 1)) != 0 && "Texture width is not a power of 2." )
	{
		SDL_FreeSurface(NewSurface);
		return INVALID_IMAGE;
	}
	

	unsigned int TextureWidth = NewSurface->w;
	unsigned int TextureHeight = NewSurface->h;
	int image = convert_surface(NewSurface);

	SDL_FreeSurface(NewSurface);

	// If no texture has been found, create a new one
	TextureList.insert( std::pair<int, Texture>(image, Texture(1, TextureWidth, TextureHeight, file_string)) );
	
	FAIL_IF(image < 0) { return -1; }
	return image;
}

bool TextureController::Release_Texture( int image )
{
	FAIL_IF ( image == INVALID_IMAGE ) { return false; }
	FAIL_IF( TextureList.empty() ) { return false; }

	TextureListType::iterator iter = TextureList.find( image );
	FAIL_IF ( iter == TextureList.end() ) { return false; }
	FAIL_IF ( (*iter).second.ReferenceCount == 0 )	{ return false; }
		
	(*iter).second.ReferenceCount -= 1;
	if ( (*iter).second.ReferenceCount == 0 )
	{
		//  Free up the memory from the image
		const GLuint texture = image;
		glDeleteTextures(1, &texture);

		//  Move the last Texture in the list to this spot and pop the back
		TextureList.erase( iter );
	}

	return true;
}

void TextureController::Draw_Texture( int image, unsigned int x, unsigned int y, unsigned int w, unsigned int h, float alpha, float scaleX, float scaleY )
{
	FAIL_IF ( image < 0) { return; }

	TextureListType::iterator iter = TextureList.find( image );
	FAIL_IF ( iter == TextureList.end() ) { return; }

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image );
	glColor4f( 1.0f, 1.0f, 1.0f, alpha );

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(x, y);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(x + int(w * scaleX), y);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(x + int(w * scaleX), y + int(h * scaleY));
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(x, y + int(h * scaleY));
	glEnd();
}

void TextureController::Draw_Texture_Part( int image, int x, int y, unsigned int part_x, unsigned int part_y, unsigned int part_w, unsigned int part_h, float alpha, float scaleX, float scaleY )
{
	FAIL_IF ( image < 0) { return; }

	TextureListType::iterator iter = TextureList.find( image );
	FAIL_IF ( iter == TextureList.end() ) { return; }

	float Left		= (float)part_x / (float)Get_Texture_Width( image );
	float Right		= (float)(part_x + part_w) / (float)Get_Texture_Width( image );
	float Top		= (float)part_y / (float)Get_Texture_Height( image );
	float Bottom	= (float)(part_y + part_h) / (float)Get_Texture_Height( image );

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, image);
	glColor4f( 1.0f, 1.0f, 1.0f, alpha );

	glBegin(GL_QUADS);
		glTexCoord2f(Left, Top);
		glVertex2i(x, y);
		glTexCoord2f(Right, Top);
		glVertex2i(x + int(part_w * scaleX), y);
		glTexCoord2f(Right, Bottom);
		glVertex2i(x + int(part_w * scaleX), y + int(part_h * scaleY));
		glTexCoord2f(Left, Bottom);
		glVertex2i(x, y + int(part_h * scaleY));
	glEnd();
}