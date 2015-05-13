#include "LoadLayout.h"

#include <hash_map>
#include <vector>
#include <string>

#include "Engine/XML_File.h"
#include "Engine/DebugTools.h"
#include "Engine/TextureController.h"
#include "Engine/InputSystem.h"

const XML_File* LayoutXML = NULL;

static TextureController* texture_controller = TextureController::Get_Instance();

class LayoutElement
{
public:
	LayoutElement(const int x, const int y, const int additiveX, const int additiveY, const float scaleX, const float scaleY, const float additiveScaleX, const float additiveScaleY) : 
		X(x),
		Y(y),
		AdditiveX(additiveX),
		AdditiveY(additiveY),
		ScaleX(scaleX),
		ScaleY(scaleY),
		AdditiveScaleX(additiveScaleX),
		AdditiveScaleY(additiveScaleY),
		ActiveX(X),
		ActiveY(Y),
		ActiveScaleX(ScaleX),
		ActiveScaleY(ScaleY)
		{
			Show();
		}


	void Show() { Hidden = false; }
	void Hide() { Hidden = true; }
	const bool IsHidden() const { return Hidden; }
	virtual bool Collision(const int x, const int y) = 0;

	virtual void Render() const = 0;

	void Reposition(const int x, const int y)
	{
		ActiveX = x;
		ActiveY = y;
		
		if (ConnectedTag != NULL)
		{
			if (ConnectedTag->Get_Element("x", true).empty() == false)
			{
				char new_x[16];
				_itoa_s(ActiveX, new_x, 10);
				ConnectedTag->Set_Element("x", new_x);
			}
		}
		
	}

	void Rescale(const float scaleX, const float scaleY) { ActiveScaleX = scaleX; ActiveScaleY = scaleY; }
	
	const int AdditiveX;
	const int AdditiveY;
	const int X;
	const int Y;
	const float AdditiveScaleX;
	const float AdditiveScaleY;
	const float ScaleX;
	const float ScaleY;

	int ActiveX;
	int ActiveY;
	float ActiveScaleX;
	float ActiveScaleY;

	XML_Tag* ConnectedTag;
private:
	bool Hidden;
};

class LayoutTextElement : public LayoutElement
{
public:
	LayoutTextElement(const int x, const int y, const int additiveX, const int additiveY, const float scaleX, const float scaleY, const float additiveScaleX, const float additiveScaleY, const std::string font_name, const std::string text) : 
		LayoutElement(x, y, additiveX, additiveY, scaleX, scaleY, additiveScaleX, additiveScaleY),
		FontName(font_name),
		Text(text) 
		{
		}
	void Render() const
	{
		//  TODO: Write render code
	}
	void SetText( const std::string text ) { Text = text; }
	virtual bool Collision(const int x, const int y) { return false; }

private:
	std::string FontName;
	std::string Text;
};

class LayoutObjectElement : public LayoutElement
{
public:
	LayoutObjectElement(const int x, const int y, const int additiveX, const int additiveY, const float scaleX, const float scaleY, const float additiveScaleX, const float additiveScaleY, const unsigned int w, const unsigned int h, const int texture ) : 
		LayoutElement(x, y, additiveX, additiveY, scaleX, scaleY, additiveScaleX, additiveScaleY), 
		W(w), 
		H(h), 
		Texture(texture)
		{
		}
	void Render() const
	{
		texture_controller->Draw_Texture_Part(Texture, AdditiveX + ActiveX, AdditiveY + ActiveY, 0, 0, W, H, 1.0f, AdditiveScaleX * ActiveScaleX, AdditiveScaleY * ActiveScaleY);
	}
	virtual bool Collision(const int x, const int y) { return ((Texture >= 0) &&(x > X + AdditiveX) && (x < X + AdditiveX + int(W * ScaleX * AdditiveScaleX)) && (y > Y + AdditiveY) && (y < Y + AdditiveY + int(H * ScaleY * AdditiveScaleY))); }

	const unsigned int W;
	const unsigned int H;
	const int Texture;
};

bool LoadResourceFolder( const XML_Tag* folder, std::string root );
bool LoadSceneData( const XML_Tag* scene, std::string root, bool hide_override, int x_additive, int y_additive, float sx_additive, float sy_additive );

typedef stdext::hash_map< std::string, int > LayoutTextureListType;
static LayoutTextureListType TextureList;
static LayoutTextureListType TextureRootNameList;
static LayoutTextureListType ParticleRootNameList;

typedef stdext::hash_map< std::string, LayoutElement* > LayoutElementListType;
static LayoutElementListType LayoutElementList;

typedef std::vector<std::string> RenderingListType;
static RenderingListType RenderingList;

static LayoutElement* SelectedElement = NULL;

bool LoadLayoutFile( const char* filename )
{
	LayoutXML = XML_File::Load_XML_File( filename );
	FAIL_IF ( LayoutXML == NULL ) { return false; }

	const XML_Tag* top_tag = LayoutXML->Get_Encompassing_Tag();
	FAIL_IF ( top_tag == NULL ) { return false; }

	const XML_Tag* root_tag = top_tag->Find_Child( "ROOT", 1 );
	FAIL_IF ( root_tag == NULL ) { return false; }

	const XML_Tag* project_tag = root_tag->Find_Child( "PROJECT", 1 );
	FAIL_IF ( project_tag == NULL ) { return false; }

	LoadResourceFolder( project_tag, "Root" );

	const XML_Tag* scene_tag = root_tag->Find_Child( "SCENE", 1 );
	FAIL_IF ( scene_tag == NULL ) { return false; }

	LoadSceneData( scene_tag, "Root", false, 0, 0, 1.0f, 1.0f );

	return true;
}

void SpecifyLayoutRenderingOrder( const char** objectNames, const int objectCount )
{
	for (int i = 0; i < objectCount; ++i) { RenderingList.push_back(objectNames[i]); }
}


bool LoadResourceFolder( const XML_Tag* folder, std::string root )
{
	for ( XML_Tag::ChildListIterType child_iter = folder->Children.begin(); child_iter != folder->Children.end(); ++child_iter )
	{
		std::string new_element_name( (*child_iter)->Get_Name() );

		if (new_element_name.compare("FOLDER") == 0)
		{
			LoadResourceFolder( (*child_iter), root + "." + (*child_iter)->Get_Element( "name" ) );
			continue;
		}
		else if (new_element_name.compare("IMAGE") == 0)
		{
			const std::string file = "Assets/Assets" + (*child_iter)->Get_Element( "file" );
			const std::string name = root + "." + (*child_iter)->Get_Element( "name" );

			int texture = texture_controller->Load_Texture( file );
			TextureRootNameList.insert( std::pair<std::string, int>(name, texture) );
			TextureList.insert( std::pair<std::string, int>(file, texture) );
			continue;
		}
		else if (new_element_name.compare("ANIM") == 0)
		{
			continue;
		}
		else if (new_element_name.compare("SPRITE") == 0)
		{
			continue;
		}
		else if (new_element_name.compare("PARTICLE") == 0)
		{
			const std::string file = "Assets/Assets" + (*child_iter)->Get_Element( "file" );
			const std::string name = root + "." + (*child_iter)->Get_Element( "name" );
			ParticleRootNameList.insert( std::pair<std::string, int>(name, -1) );
			continue;
		}
	}

	return true;
}

bool LoadSceneData( const XML_Tag* scene, std::string root, bool hide_override, int x_additive, int y_additive, float sx_additive, float sy_additive )
{
	for ( XML_Tag::ChildListIterType child_iter = scene->Children.begin(); child_iter != scene->Children.end(); ++child_iter )
	{
		std::string new_element_name( (*child_iter)->Get_Name() );

		if (new_element_name.compare("CANVAS") == 0)
		{
			int x = atoi((*child_iter)->Get_Element("x").c_str());
			int y = atoi((*child_iter)->Get_Element("y").c_str());
			float sx = float(atof((*child_iter)->Get_Element("sx").c_str()));
			float sy = float(atof((*child_iter)->Get_Element("sy").c_str()));
			hide_override |= ((*child_iter)->Get_Element("hide").compare("1") == 0);
			LoadSceneData( (*child_iter), root + "." + (*child_iter)->Get_Element("name"), hide_override, x_additive + x, y_additive + y, sx_additive * sx, sy_additive * sy );
		}
		else if (new_element_name.compare("OBJECT") == 0)
		{
			std::string asset = (*child_iter)->Get_Element("aset");
			std::string name = (*child_iter)->Get_Element("name");

			LayoutObjectElement* new_object = NULL;
			//  Check the texture list for the asset first
			if (TextureRootNameList.find(asset) != TextureRootNameList.end())
			{
				bool hidden = hide_override | ((*child_iter)->Get_Element("hide").compare("1") == 0);
				int x = std::atoi((*child_iter)->Get_Element("x").c_str());
				int y = std::atoi((*child_iter)->Get_Element("y").c_str());
				int w = std::atoi((*child_iter)->Get_Element("w").c_str());
				int h = std::atoi((*child_iter)->Get_Element("h").c_str());
				float sx = float(atof((*child_iter)->Get_Element("sx").c_str()));
				float sy = float(atof((*child_iter)->Get_Element("sy").c_str()));
				bool center_align = ((*child_iter)->Get_Element("a", true).compare("cc") == 0);
				if (center_align)
				{
					x -= w / 2;
					y -= h / 2;
				}
				new_object = new LayoutObjectElement(x, y, x_additive, y_additive, sx, sy, sx_additive, sy_additive, w, h, TextureRootNameList[asset]);
				if (hidden) new_object->Hide();
			}
			else
			{
				new_object = new LayoutObjectElement(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1);
				new_object->Hide();
			}
			LayoutElementList[root + "." + name] = new_object;
			new_object->ConnectedTag = (*child_iter);
		}
	}

	return true;
}


void RenderLayout( void )
{
	if (RenderingList.empty()) { return; }
	for (RenderingListType::const_iterator iter = RenderingList.begin(); iter != RenderingList.end(); ++iter)
	{
		if (LayoutElementList.find((*iter)) == LayoutElementList.end()) continue;
		LayoutElementList[(*iter)]->Render();
	}
}

void UnloadLayoutFile( void )
{
	for (LayoutTextureListType::const_iterator iter = TextureList.begin(); iter != TextureList.end(); ++iter)
	{
		texture_controller->Release_Texture( (*iter).second );
	}
	TextureRootNameList.clear();
	TextureList.clear();
}

void LayoutInput(void)
{
	InputSystem* input_system = InputSystem::Get_Instance();

	if (input_system->Get_Key(SDL_SCANCODE_SPACE) == 1)
	{
		LayoutXML->Write_XML_File("Assets/Assets/Layout/new_layout.xml");
	}

	if (input_system->Get_Mouse_Button(InputSystem::MOUSE_RIGHT) == 1)
	{
		LayoutSelect(input_system->Get_Mouse_X(), input_system->Get_Mouse_Y());
	}

	if (SelectedElement != NULL)
	{
		if (input_system->Get_Key(SDL_SCANCODE_LSHIFT) || input_system->Get_Key(SDL_SCANCODE_RSHIFT))
		{
			if (input_system->Get_Key(SDL_SCANCODE_BACKSPACE))
			{
				SelectedElement->Rescale(SelectedElement->ScaleX, SelectedElement->ScaleY);
			}
			if (input_system->Get_Key(SDL_SCANCODE_LEFT))
			{
				SelectedElement->Rescale(SelectedElement->ActiveScaleX - 0.001f, SelectedElement->ActiveScaleY);
			}
			if (input_system->Get_Key(SDL_SCANCODE_RIGHT))
			{
				SelectedElement->Rescale(SelectedElement->ActiveScaleX + 0.001f, SelectedElement->ActiveScaleY);
			}
			if (input_system->Get_Key(SDL_SCANCODE_UP))
			{
				SelectedElement->Rescale(SelectedElement->ActiveScaleX, SelectedElement->ActiveScaleY - 0.001f);
			}
			if (input_system->Get_Key(SDL_SCANCODE_DOWN))
			{
				SelectedElement->Rescale(SelectedElement->ActiveScaleX, SelectedElement->ActiveScaleY + 0.001f);
			}
		}
		else
		{
			if (input_system->Get_Key(SDL_SCANCODE_BACKSPACE))
			{
				SelectedElement->Reposition(SelectedElement->X, SelectedElement->Y);
			}
			if (input_system->Get_Key(SDL_SCANCODE_LEFT))
			{
				SelectedElement->Reposition(SelectedElement->ActiveX - 1, SelectedElement->ActiveY);
			}
			if (input_system->Get_Key(SDL_SCANCODE_RIGHT))
			{
				SelectedElement->Reposition(SelectedElement->ActiveX + 1, SelectedElement->ActiveY);
			}
			if (input_system->Get_Key(SDL_SCANCODE_UP))
			{
				SelectedElement->Reposition(SelectedElement->ActiveX, SelectedElement->ActiveY - 1);
			}
			if (input_system->Get_Key(SDL_SCANCODE_DOWN))
			{
				SelectedElement->Reposition(SelectedElement->ActiveX, SelectedElement->ActiveY + 1);
			}
		}
	}
}

void LayoutSelect(const int x, const int y)
{
	if (RenderingList.empty()) { return; }
	for (RenderingListType::const_iterator iter = RenderingList.end(); iter != RenderingList.begin();)
	{
		--iter;
		if (LayoutElementList.find((*iter)) == LayoutElementList.end()) continue;
		if (LayoutElementList[(*iter)]->Collision(x, y))
		{
			SelectedElement = LayoutElementList[(*iter)];
			break;
		}
	}
}