#include "MainProgram.h"

#include "Engine/FontController.h"
#include "Engine/InputSystem.h"
#include "Engine/DebugTools.h"
#include "Engine/TextureController.h"
#include <fstream>
#include <string>

FontController* font_controller = FontController::Get_Instance();
InputSystem* input_system = InputSystem::Get_Instance();
Font* loaded_font1 = NULL;
Font* loaded_font2 = NULL;

#define MAX_UNI_CHARS		255//0xFFFFU

//!Structure containing font header information
struct FontHeader
{
	//!Number of characters contained in the font
	short nmChar;
	//!Width of a space for the font
	short spaceWidth;
	short m_Flags;
	unsigned short m_MaxChars;
	int nmKerning;
	char pad[4];
	//!Structure containing information about font characters
	Uint64 charptr;  // pointer to _chars
	//!Structure containing information about font kerning
	Uint64 kernptr;
	Uint64 textureptr[10];
	//!ascii map for the font
	short asciiMap[MAX_UNI_CHARS];
};

struct FontChar
{
	unsigned short unicode;
	short u1,u2,v1,v2,firstKern;
	signed char advance;
	signed char yoff;
	signed char xoff;
	unsigned char texindex;
};
	
struct FontKern
{
	unsigned short code1;
	unsigned short code2;
	short spacing;
};

std::string font_name = "ArialNarrow";
std::string font_save_location = "Assets/Fonts/" + font_name + "/" + font_name + ".fnt";

MainProgram::MainProgram()													{}

const bool MainProgram::Initialize()
{
	//  Load the created font
	ASSERT(font_controller->Load_Font( font_name.c_str() ));
	loaded_font1 = font_controller->Get_Font( font_name.c_str() );

	//  Attempt to load a FNT to test the structure
	std::ifstream fnt_load("Assets/ARIALNB.fnt", std::ios::in | std::ios::binary);
	FAIL_IF (fnt_load.bad() || !fnt_load.good()) { return false; }

    fnt_load.seekg (0, fnt_load.end);
    int length = (int)(fnt_load.tellg());
    fnt_load.seekg (0, fnt_load.beg);

	char* file_data = new char[length];
	fnt_load.read(file_data, length);
	FontHeader* loaded_header = (FontHeader*)(file_data);
	int test = int(sizeof(FontHeader));
	
	Font new_font;
	new_font.SetTexture(TextureController::Get_Instance()->Load_Texture("Assets/ARIALNB0.png"));
	for (int i = 0; i < MAX_UNI_CHARS; ++i)
	{
		int index = loaded_header->asciiMap[i];
		if (index == -1) continue;

		FontChar& new_char = ((FontChar*)(loaded_header->charptr + (char*)loaded_header))[index];
		Font::Character* new_character = new Font::Character;
		new_character->X = new_char.u1;
		new_character->Y = new_char.v1;
		new_character->W = new_char.u2 - new_char.u1;
		new_character->H = new_char.v2 - new_char.v1;
		new_character->X_Offset = new_char.xoff;
		new_character->Y_Offset = -new_char.yoff;
		new_character->Advance = new_char.advance - new_character->W;

		int kernIndex = new_char.firstKern;
		if (kernIndex != -1)
		{
			FontKern& kern_data = ((FontKern*)(loaded_header->kernptr + (char*)loaded_header))[kernIndex];
			while (kern_data.code2 == index)
			{
				Font::Character::SpecialKerning* new_kern = new Font::Character::SpecialKerning;
				new_kern->PreviousCharacter = kern_data.code1;
				new_kern->SpacingDelta = kern_data.spacing;
				new_character->SpecialKerningList[i] = new_kern;
				kern_data = ((FontKern*)(loaded_header->kernptr + (char*)loaded_header))[++kernIndex];
			}
		}

		new_font.AddCharacter(i, new_character);
	}
	font_controller->AddFont("Arial", new_font);
	loaded_font2 = font_controller->Get_Font("Arial");

	delete [] file_data;
	return true;
}

const bool MainProgram::Input( const float time_slice )
{
	if (input_system->Get_Key(SDL_SCANCODE_SPACE))
	{
		FontHeader output_header;
		output_header.nmChar = loaded_font1->Get_Character_Count();
		output_header.spaceWidth = loaded_font1->Get_Text_Width(" ");
		output_header.m_Flags = 0;
		output_header.m_MaxChars = 0;
		output_header.nmKerning = 0;
		output_header.pad[0] = 0;
		output_header.pad[1] = 0;
		output_header.pad[2] = 0;
		output_header.pad[3] = 0;
		output_header.charptr = (Uint64)(sizeof(FontHeader));
		output_header.kernptr = output_header.charptr + (sizeof(FontChar) * loaded_font1->Get_Character_Count());
		output_header.textureptr[0] = 0;
		output_header.textureptr[1] = 0;
		output_header.textureptr[2] = 0;
		output_header.textureptr[3] = 0;
		output_header.textureptr[4] = 0;
		output_header.textureptr[5] = 0;
		output_header.textureptr[6] = 0;
		output_header.textureptr[7] = 0;
		output_header.textureptr[8] = 0;
		output_header.textureptr[9] = 0;
		
		std::memset(output_header.asciiMap, -1, sizeof(short) * MAX_UNI_CHARS);
		int counter = 0;
		const std::hash_map<int, Font::Character*>& character_map = loaded_font1->Get_Character_Map();
		for (std::hash_map<int, Font::Character*>::const_iterator iter = character_map.begin(); iter != character_map.end(); ++iter)
		{
			output_header.asciiMap[(*iter).first] = counter++;
		}

		std::ofstream output_file(font_save_location.c_str(), std::ios::out | std::ios::binary);
		if (output_file.bad() || !output_file.good()) { assert(false); return false; }

		output_file.write((char*)(&output_header), sizeof(FontHeader));
		
		for (std::hash_map<int, Font::Character*>::const_iterator iter = character_map.begin(); iter != character_map.end(); ++iter)
		{
			FontChar font_char;
			font_char.unicode = (*iter).first;
			font_char.u1 = (*iter).second->X;
			font_char.u2 = (*iter).second->W + font_char.u1;
			font_char.v1 = (*iter).second->Y;
			font_char.v2 = (*iter).second->H + font_char.v1;
			font_char.xoff = 0;
			font_char.yoff = 0;
			font_char.texindex = 0;
			font_char.firstKern = -1;
			font_char.advance = (*iter).second->W;
			output_file.write((char*)(&font_char), sizeof(FontChar));
		}
		
		for (std::hash_map<int, Font::Character*>::const_iterator iter = character_map.begin(); iter != character_map.end(); ++iter)
		{
			for (std::hash_map<int, Font::Character::SpecialKerning*>::const_iterator kern_iter = (*iter).second->SpecialKerningList.begin(); kern_iter != (*iter).second->SpecialKerningList.end(); ++kern_iter)
			{
				output_file.write((char*)((*kern_iter).second), sizeof(FontKern));
			}
		}

		output_file.close();
	}

	return true;
}

const bool MainProgram::Update( const float time_slice )			{ return true; }
const bool MainProgram::Render3D( void )							{ return true; }

const bool MainProgram::Render2D( void )
{
	loaded_font1->Render_Text("aaaabbbbccccddddeeeeffffgggghhhhiiiijjjjkkkkllllmmmmnnnnooooppppqqqq", 100, 100);
	loaded_font1->Render_Text("rrrrssssttttuuuuvvvvwwwwxxxxyyyyzzzzAAAABBBBCCCDDDDEEEEFFFFGGGGHHHH", 100, 148);
	loaded_font1->Render_Text("IIIJJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRRSSSSTTTTUUUUVVVVWWWWXXXXYYYY", 100, 196);
	loaded_font1->Render_Text("ZZZZ1111222233334444555566667777888899990000....::::,,,,;;;;''''\"\"\"\"", 100, 244);
	loaded_font1->Render_Text("((((!!!!????))))++++----****////====", 100, 292);
	loaded_font1->Render_Text(("This is a \"test\" of the " + font_name + " font. :)").c_str(), 100, 340);
	
	loaded_font2->Render_Text("This is a test of the Arial font. :)", 40, 500);
	return true;
}

const bool MainProgram::Shutdown()
{
	return true;
}