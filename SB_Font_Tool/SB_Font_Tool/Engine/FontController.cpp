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
/*  File:       FontController.cpp                               */
/*                                                               */
/*  Purpose:    This file contains the primary system in place   */
/*              for loading and managing fonts in-game. The      */
/*              fonts can then be utilized and rendered easily.  */
/*                                                               */
/*  Created:    01/02/2009                                       */
/*  Last Edit:  04/18/2010                                       */
/*****************************************************************/

#include "FontController.h"

#include "TextureController.h"
#include "XML_File.h"
#include "DebugTools.h"


Font::Font() : 
	Texture( INVALID_IMAGE ),
	CharacterCount( 0 )
{
}


Font::~Font()
{
}


void Font::Shutdown( void )
{
	if ( Texture != INVALID_IMAGE )
	{
		TextureController::Get_Instance()->Release_Texture( Texture );
	}

	for (std::hash_map<int, Character*>::iterator iter = CharacterMap.begin(); iter != CharacterMap.end(); ++iter)
	{
		delete (*iter).second;
	}
	CharacterMap.clear();
}


void Font::Render_Text( const char* text, int x, int y, float x_scale, float y_scale, bool x_centered, bool y_centered )
{
	if ( Texture == INVALID_IMAGE ) return;

	//  Determine the X offset
	unsigned int x_offset = x;
	if ( x_centered )
	{
		unsigned int width = (unsigned int)(Get_Text_Width( text ) * x_scale);
		x_offset -= (width >> 1);
	}

	//  Determine the Y offset inside the loop per character
	unsigned int y_offset = y;

	//  Render each character in a line while updating the X offset
	TextureController* textures = TextureController::Get_Instance();
	Character* character = NULL;
	for ( unsigned int i = 0; i < strlen( text ); ++i )
	{
		if (CharacterMap.find(text[i]) == CharacterMap.end()) { assert(false); continue; }
		character = CharacterMap[text[i]];

		if (i != 0 && character->SpecialKerningList.find(text[i - 1]) != character->SpecialKerningList.end())
		{
			x_offset += (unsigned int)(character->SpecialKerningList[text[i - 1]]->SpacingDelta * x_scale);
		}

		if ( y_centered ) { y_offset = y - (unsigned int)((character->H >> 1) * y_scale); }
		textures->Draw_Texture_Part( Texture, x_offset + (unsigned int)(character->X_Offset * x_scale), y_offset + (unsigned int)(character->Y_Offset * y_scale), character->X, character->Y, character->W, character->H, 1.0f, x_scale, y_scale );
		x_offset += (unsigned int)((character->W + character->X_Offset + character->Advance) * x_scale);
	}
}


unsigned int Font::Get_Text_Width( const char* text )
{
	unsigned int width = 0;
	for (unsigned int i = 0; i < strlen( text ); ++i)
	{
		if (CharacterMap.find(text[i]) == CharacterMap.end()) { assert(false); continue; }
		width += CharacterMap[text[i]]->W + CharacterMap[text[i]]->X_Offset;
	}
	return width;
}


unsigned int Font::Character_Count_Before_Passing_Width( const char* text, unsigned int width, bool cut_at_spaces )
{
	unsigned int character_count = 0;
	std::string new_string("");
	for ( character_count = 0; character_count < strlen( text ); ++character_count )
	{
		new_string += text[character_count];
		if ( Get_Text_Width( new_string.c_str() ) > width ) { break; }
	}

	if ( character_count == strlen( text ) ) { return character_count; }

	if ( cut_at_spaces )
	{
		unsigned int new_return_value = new_string.find_last_of( " " );
		if ( ( new_return_value != std::string::npos ) && ( new_return_value + 1 < new_string.size() ) )
		{
			character_count = new_return_value + 1;
		}
	}
	return character_count;
}


const XML_Tag* Font::Get_Font_XML( const XML_File *xml )
{
	const XML_Tag* entire_xml = xml->Get_Encompassing_Tag();
	FAIL_IF ( entire_xml->Children.size() != 1 ) { return NULL; }

	const XML_Tag* font = entire_xml->Find_Child( "BitmapFont", 1 );
	FAIL_IF ( font == NULL ) { return NULL; }

	FAIL_IF ( font->Children.size() == 0 ) { return NULL; }

	return font;
}


FontController* FontController::Get_Instance( void )
{
	static FontController INSTANCE;
	return &INSTANCE;
}


void FontController::Set_Font_Folder( const char* folder_name )
{
	FontFolder = std::string( folder_name );
}


bool FontController::Load_Font( const char* font_name )
{
	std::string font_name_string( font_name );

	FontListType::iterator iter = FontList.find( font_name_string );
	FAIL_IF ( "Font already exists in the list" && iter != FontList.end() ) { return false; }

	std::string full_file_path( FontFolder );
	full_file_path.append( font_name );
	full_file_path.append( "/" );
	full_file_path.append( font_name );
	full_file_path.append( ".xml" );

	const XML_File* font_xml = XML_File::Load_XML_File( full_file_path.c_str() );
	FAIL_IF ( !font_xml ) { return false; }

	const XML_Tag* font = Font::Get_Font_XML( font_xml );
	FAIL_IF (font == NULL)
	{
		delete font_xml;
		return false;
	}

	std::string font_texture( "Assets/Fonts/" );
	font_texture.append( font_name );
	font_texture.append( "/" );
	font_texture.append( font_name );
	font_texture.append( ".png" );

	int texture = TextureController::Get_Instance()->Load_Texture( font_texture );
	FAIL_IF ( texture == INVALID_IMAGE ) { return false; }

	Font new_font;
	new_font.Texture = texture;

	unsigned int totalHeight = 0;
	new_font.CharacterCount = 0;
	for (unsigned int i = 0; i < font->Children.size(); ++i)
	{
		++new_font.CharacterCount;
		Font::Character* new_character = new Font::Character;
		unsigned int index = atoi(font->Children[i]->Find_Child("ID", 1)->Get_Value());
		new_character->X = atoi(font->Children[i]->Find_Child("X", 1)->Get_Value());
		new_character->Y = atoi(font->Children[i]->Find_Child("Y", 1)->Get_Value());
		new_character->W = atoi(font->Children[i]->Find_Child("W", 1)->Get_Value());
		new_character->H = atoi(font->Children[i]->Find_Child("H", 1)->Get_Value());
		new_font.AddCharacter(index, new_character);

		totalHeight += new_character->W; 
	}
	new_font.Height = totalHeight / font->Children.size();

	FontList[ font_name_string ] = new_font;

	return true;
}


Font* FontController::Get_Font( const char* font_name )
{
	std::string font_name_string( font_name );
	FontListType::iterator iter = FontList.find( font_name_string );
	FAIL_IF ( iter == FontList.end() ) { return NULL; }

	return &((*iter).second);
}


bool FontController::Unload_Font( const char* font_name )
{
	std::string font_name_string( font_name );

	FontListType::iterator iter = FontList.find( font_name_string );
	FAIL_IF ( "Font does not exist in the list" && iter == FontList.end() ) { return false; }

	iter->second.Shutdown();
	FontList.erase( iter );
	return true;
}


void FontController::Shutdown( void )
{
	for ( FontListType::iterator iter = FontList.begin(); iter != FontList.end(); ++iter )
	{
		(*iter).second.Shutdown();
	}
	FontList.clear();
}


FontController::FontController()
{
}


FontController::~FontController()
{
	if ( !FontList.empty() )
	{
		Shutdown();
	}
}