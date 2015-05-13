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
/*  File:       CutsceneSystem.cpp                               */
/*                                                               */
/*  Purpose:    This file contains the system for loading and    */
/*              playing 2D cut-scenes made up of animations,     */
/*              sounds, and movement effects.                    */
/*                                                               */
/*  Created:    01/16/2011                                       */
/*  Last Edit:  04/09/2011                                       */
/*****************************************************************/

#include "CutsceneSystem.h"

#include "DebugTools.h"
#include "XML_File.h"
#include "Sprite.h"
#include "FontController.h"
#include "StringToValue.h"

CutsceneSystem::Actor*		LoadCutsceneActor( const XML_Tag& actor_tag );
CutsceneSystem::Resource*	LoadCutsceneResource( const XML_Tag& asset_tag );
CutsceneSystem::Event*		LoadCutsceneEvent( const XML_Tag& event_tag );
CutsceneSystem::Resource*	LoadCutsceneSprite( const char* name, const char* file );
CutsceneSystem::Resource*	LoadCutsceneSound( const char* name, const char* file );
CutsceneSystem::Resource*	LoadCutsceneFont( const char* name, const char* file );

class CutsceneSystem::Resource
{
public:
	static enum Type { RES_IMAGE, RES_SOUND, RES_FONT, RESOURCE_TYPE_COUNT };

	Resource() : 
		ResourceType( RESOURCE_TYPE_COUNT ),
		Name( "" ),
		Asset( "" )
	{}

	Type ResourceType;
	std::string Name;
	std::string Asset;

	virtual void Render( int x, int y, float alpha ) {}
};

class ResourceSprite : public CutsceneSystem::Resource
{
public:
	Sprite Image;

	void Render( int x, int y, float alpha ) { Image.Render( x, y, alpha ); }
};

class ResourceSound : public CutsceneSystem::Resource
{
};

class ResourceFont : public CutsceneSystem::Resource
{
public:
	Font* CutsceneFont;
	std::string Text;
	
	void Render( int x, int y, float alpha ) { CutsceneFont->Render_Text( Text.c_str(), x, y ); }
};

class CutsceneSystem::Actor
{
public:
	Actor() : ActorName( "" ), X( -1.0f ), Y( -1.0f ), Alpha( 1.0f ), Asset( NULL ) {}
	std::string ActorName;
	float X;
	float Y;
	float Alpha;
	Resource* Asset;

	void Render( int x, int y, float alpha ) { Asset->Render( x, y, alpha ); }
};

class CutsceneSystem::Event
{
public:
	static enum Type { ET_SPRITECHANGE, ET_WARPTO, ET_MOVETO, ET_SOUNDPLAY, ET_WAIT, ET_ALPHA, ET_SETFONT, ET_SETTEXT, EVENT_TYPE_COUNT };
	
	Event() : Type( EVENT_TYPE_COUNT ), Trigger( "" ), PassMethod( "" ), EndingEvent( "" ), Target( "" ), X( -1.0f ), Y( -1.0f ), Time( -1.0f ), Asset( "" ) {}

	Type Type;
	std::string Trigger;
	std::string PassMethod;
	std::string EndingEvent;
	std::string Target;
	float X;
	float Y;
	float Time;
	std::string Asset;
};

CutsceneSystem::CutsceneSystem() : 
	CutsceneTime( 0.0f )
{}


CutsceneSystem::~CutsceneSystem()
{
	Shutdown();
}

bool CutsceneSystem::Load( const char* cutscene_file )
{
	//  Attempt to load the specified XML file
	FAIL_IF ( cutscene_file == NULL ) { return false; }
	const XML_File* file = XML_File::Load_XML_File( cutscene_file );
	FAIL_IF (file == NULL ) { return false; }

	//  Locate the encompassing tag, labeled "Cutscene"
	const XML_Tag* cutscene_tag = file->Get_Encompassing_Tag()->Find_Child( "Cutscene", 1 );
	FAIL_IF ( "Cutscene file has no encompassing 'Cutscene' tag." && cutscene_tag == NULL )
	{
		delete file;
		return false;
	}

	//  Locate the tag for the list of actor objects, labeled "Actors"
	const XML_Tag* actors_tag = cutscene_tag->Find_Child( "Actors", 1 );
	FAIL_IF ( "Cutscene has no Actors list." && actors_tag == NULL )
	{
		delete file;
		return false;
	}

	//  Locate the tag for the list of assets, labeled "Assets"
	const XML_Tag* resource_tag = cutscene_tag->Find_Child( "Resources", 1 );
	FAIL_IF ( "Cutscene has no Assets list." && resource_tag == NULL )
	{
		delete file;
		return false;
	}

	//  Locate the tag for the list of events, labeled "Events"
	const XML_Tag* events_tag = cutscene_tag->Find_Child( "Events", 1 );
	FAIL_IF ( "Cutscene has no Events list." && events_tag == NULL )
	{
		delete file;
		return false;
	}

	//  Load in the list of Actors
	for ( XML_Tag::ChildListIterType iter = actors_tag->Children.begin(); iter != actors_tag->Children.end(); ++iter )
	{
		ActorList.push_back( LoadCutsceneActor( (**iter) ) );
	}

	//  Load in the list of Assets
	for ( XML_Tag::ChildListIterType iter = resource_tag->Children.begin(); iter != resource_tag->Children.end(); ++iter )
	{
		Resource* new_resource = LoadCutsceneResource( (**iter) );
		ResourceList.push_back( new_resource );
	}

	//  Load in the list of Events
	for ( XML_Tag::ChildListIterType iter = events_tag->Children.begin(); iter != events_tag->Children.end(); ++iter )
	{
		Event* new_event = LoadCutsceneEvent( (**iter) );
		EventList.push_back( new_event );

		//  If the new event has a Trigger of "Start", then throw it into the ActiveEvents immediately
		if ( new_event->Trigger.compare( "Start" ) == 0 )
		{
			ActiveEventList.push_back( new_event );
		}
	}

	return true;
}


void CutsceneSystem::Update( float time_slice )
{
	//  Iterate the current cutscene time with the time slice
	CutsceneTime += time_slice;

	std::vector< Event* > added_events;

	//  Look through the Active Events list for any events that are expiring
	for ( std::vector< Event* >::iterator iter = ActiveEventList.begin(); iter != ActiveEventList.end(); )
	{
		bool remove_event = false;

		//  Process the event according to it's type
		//  TODO:  Optimize the shit out of this
		switch ( (*iter)->Type )
		{
		case Event::ET_WARPTO:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Actor name and keep a hold of it
				Actor* target_actor = NULL;
				for ( std::vector< Actor* >::iterator actor_iter = ActorList.begin(); actor_iter != ActorList.end(); ++actor_iter )
				{
					if ( (*actor_iter)->ActorName.compare( (*iter)->Target ) == 0 )
					{
						target_actor = (*actor_iter);
						break;
					}
				}

				target_actor->X = (*iter)->X;
				target_actor->Y = (*iter)->Y;
				remove_event = true;
			}
			break;

		case Event::ET_SPRITECHANGE:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  If there is no Asset given, remove the event
				if ( (*iter)->Asset.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Actor name and keep a hold of it
				Actor* target_actor = NULL;
				for ( std::vector< Actor* >::iterator actor_iter = ActorList.begin(); actor_iter != ActorList.end(); ++actor_iter )
				{
					if ( (*actor_iter)->ActorName.compare( (*iter)->Target ) == 0 )
					{
						target_actor = (*actor_iter);
						break;
					}
				}

				//  If no Actor by the given name was found, remove the event
				if ( target_actor == NULL )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Asset name and keep a hold of it
				Resource* target_asset = NULL;
				for ( std::vector< Resource* >::iterator asset_iter = ResourceList.begin(); asset_iter != ResourceList.end(); ++asset_iter )
				{
					if ( (*asset_iter)->Name.compare( (*iter)->Asset ) == 0 )
					{
						target_asset = (*asset_iter);
						break;
					}
				}

				//  If no Asset by the given name was found, remove the event
				if ( target_asset == NULL )
				{
					remove_event = true;
					break;
				}

				//  Assign the Asset to the Actor and break out of the event process
				target_actor->Asset = target_asset;
				remove_event = true;
			}
			break;

		case Event::ET_MOVETO:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  If there is no target Time given (usually set to -1.0f), remove the event
				if ( (*iter)->Time < 0.0f )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Actor name and keep a hold of it
				Actor* target_actor = NULL;
				for ( std::vector< Actor* >::iterator actor_iter = ActorList.begin(); actor_iter != ActorList.end(); ++actor_iter )
				{
					if ( (*actor_iter)->ActorName.compare( (*iter)->Target ) == 0 )
					{
						target_actor = (*actor_iter);
						break;
					}
				}

				if ( time_slice > (*iter)->Time )
				{
					target_actor->X = (*iter)->X;
					target_actor->Y = (*iter)->Y;
					remove_event = true;
					break;
				}
				else
				{
					//  Move the appropriate amount based on the time slice
					float percentage = time_slice / (*iter)->Time;
					target_actor->X +=  ( (*iter)->X - target_actor->X ) * percentage;
					target_actor->Y += ( (*iter)->Y - target_actor->Y ) * percentage;

					//  Update the time on the event
					(*iter)->Time -= time_slice;
				}
			}
			break;

		case Event::ET_SOUNDPLAY:
			{
				//  TODO:  Play the sound file if the event is just starting

				//  Update the time on the event
				(*iter)->Time -= time_slice;
			}
			break;

		case Event::ET_WAIT:
			{
				//  Update the time on the event
				(*iter)->Time -= time_slice;
			}
			break;

		case Event::ET_ALPHA:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  If there is no target Asset given (set to a valid float), remove the event
				float value;
				if ( !StringToFloat( (*iter)->Asset.c_str(), value ) )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Actor name and keep a hold of it
				Actor* target_actor = NULL;
				for ( std::vector< Actor* >::iterator actor_iter = ActorList.begin(); actor_iter != ActorList.end(); ++actor_iter )
				{
					if ( (*actor_iter)->ActorName.compare( (*iter)->Target ) == 0 )
					{
						target_actor = (*actor_iter);
						break;
					}
				}

				if ( time_slice > (*iter)->Time )
				{
					target_actor->Alpha = value;
					remove_event = true;
					break;
				}
				
				//  Assign the new alpha value based on the time slice and the amount needed to cover
				float percentage = time_slice / (*iter)->Time;
				target_actor->Alpha += ( value - target_actor->Alpha ) * percentage;

				//  Update the time on the event
				(*iter)->Time -= time_slice;
			}
			break;

		case Event::ET_SETFONT:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  If there is no Asset given, remove the event
				if ( (*iter)->Asset.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Actor name and keep a hold of it
				Actor* target_actor = NULL;
				for ( std::vector< Actor* >::iterator actor_iter = ActorList.begin(); actor_iter != ActorList.end(); ++actor_iter )
				{
					if ( (*actor_iter)->ActorName.compare( (*iter)->Target ) == 0 )
					{
						target_actor = (*actor_iter);
						break;
					}
				}

				//  If no Actor by the given name was found, remove the event
				if ( target_actor == NULL )
				{
					remove_event = true;
					break;
				}

				//  Loop through the ActorList for the Asset name and keep a hold of it
				Resource* target_asset = NULL;
				for ( std::vector< Resource* >::iterator asset_iter = ResourceList.begin(); asset_iter != ResourceList.end(); ++asset_iter )
				{
					if ( (*asset_iter)->Name.compare( (*iter)->Asset ) == 0 )
					{
						target_asset = (*asset_iter);
						break;
					}
				}

				//  If no Asset by the given name was found, remove the event
				if ( target_asset == NULL )
				{
					remove_event = true;
					break;
				}

				//  Assign the Asset to the Actor and break out of the event process
				target_actor->Asset = target_asset;
				remove_event = true;
			}
			break;

		case Event::ET_SETTEXT:
			{
				//  If there is no target Actor given, remove the event
				if ( (*iter)->Target.empty() == true )
				{
					remove_event = true;
					break;
				}

				//  Find the font and alter it's position
				for ( std::vector< Resource* >::iterator font_iter = ResourceList.begin(); font_iter != ResourceList.end(); ++font_iter )
				{
					if ( (*font_iter)->Name.compare( (*iter)->Target ) == 0 )
					{
						if ( (*font_iter)->ResourceType == Resource::RES_FONT )
						{
							ResourceFont* font = static_cast<ResourceFont*>( (*font_iter) );
							font->Text = (*iter)->Asset;
						}
					}
				}

				//  Immediately remove the event
				remove_event = true;
			}
			break;

		default:
			break;
		}
		
		//  If the time left on an ActiveEvent is less than 0, remove the event
		remove_event |= ( (*iter)->Time < 0.001  );

		//  If we need to remove the event, remove it. Otherwise, just iterate and continue
		if ( remove_event == true )
		{
			//  If we have an ending event for the event that is being removed...
			if ( !(*iter)->EndingEvent.empty() )
			{
				//  Send out a call for the events EndingEvent, adding them to a list of events to make active after updating
				for ( std::vector< Event* >::iterator events_iter = EventList.begin(); events_iter != EventList.end(); ++events_iter )
				{
					if ( (*events_iter)->Trigger.compare( (*iter)->EndingEvent ) == 0 )
					{
						added_events.push_back( (*events_iter) );
					}
				}
			}

			//  Remove the event from the ActiveEventList
			iter = ActiveEventList.erase( iter );
		}
		else
		{
			++iter;
		}
	}

	//  Iterate through the list of EndingEvent calls and start events if needed
	for ( std::vector< Event* >::iterator iter = added_events.begin(); iter != added_events.end(); iter = added_events.erase( iter ) )
	{
		ActiveEventList.push_back( (*iter) );
	}
};


void CutsceneSystem::Render( void )
{
	//  Render all actors that have renderable assets
	for ( std::vector< Actor* >::iterator iter = ActorList.begin(); iter != ActorList.end(); ++iter )
	{
		if ( (*iter)->Asset == NULL ) { break; }
		(*iter)->Render( int((*iter)->X), int((*iter)->Y), (*iter)->Alpha );
	}
};


void CutsceneSystem::Shutdown( void )
{
	//  Free up the memory from any Assets
	for ( std::vector< Resource* >::iterator iter = ResourceList.begin(); iter != ResourceList.end(); )
	{
		switch ( (*iter)->ResourceType )
		{
		case Resource::RES_IMAGE:
			{
				ResourceSprite* resource = static_cast<ResourceSprite*>( (*iter) );
				delete resource;
			}
			break;

		case Resource::RES_SOUND:
			{
				ResourceSound* resource = static_cast<ResourceSound*>( (*iter) );
				delete resource;
			}
			break;

		case Resource::RES_FONT:
			{
				ResourceFont* resource = static_cast<ResourceFont*>( (*iter) );
				FontController::Get_Instance()->Unload_Font( resource->Asset.c_str() );
				delete resource;
			}
			break;

		default:
			break;
		}

		iter = ResourceList.erase( iter );
	}
}


CutsceneSystem::Actor* LoadCutsceneActor( const XML_Tag& actor_tag )
{
	//  Ensure that the given tag has a value
	FAIL_IF ( "Actor tag has no value." && actor_tag.Get_Value() == NULL ) { return false; }

	//  Create a new actor and put the given tag's value in as it's name
	CutsceneSystem::Actor* new_actor = new CutsceneSystem::Actor;
	new_actor->ActorName = actor_tag.Get_Value();
	new_actor->X = 0;
	new_actor->Y = 0;

	//  Submit the new actor to the actors list
	return new_actor;
}


CutsceneSystem::Resource* LoadCutsceneResource( const XML_Tag& asset_tag )
{
	//  Find the asset type
	std::string asset_type = asset_tag.Get_Element( "type" );

	//  Ensure that the given tag has a Name and that it has a value
	const XML_Tag* name_tag = asset_tag.Find_Child( "Name", 1 );
	FAIL_IF ( "Resource tag has no Name." && name_tag == NULL ) { return false; }
	FAIL_IF ( "Resource Name tag has no value." && name_tag->Get_Value() == NULL ) { return false; }

	//  Ensure that the given tag has an Asset and that it has a value
	const XML_Tag* resource_tag = asset_tag.Find_Child( "Asset", 1 );
	FAIL_IF ( "Resource tag has no Asset." && resource_tag == NULL ) { return false; }
	FAIL_IF ( "Resource Asset tag has no value." && resource_tag->Get_Value() == NULL ) { return false; }

	//  Go through the list of types and call the appropriate loading function (TODO: Optimize)
	if		  ( asset_type.compare( "Sprite" ) == 0 )	{ return LoadCutsceneSprite( name_tag->Get_Value(), resource_tag->Get_Value() ); }
	else if ( asset_type.compare( "Sound" ) == 0	)	{ return LoadCutsceneSound( name_tag->Get_Value(), resource_tag->Get_Value() ); }
	else if ( asset_type.compare( "Font" ) == 0 )	{ return LoadCutsceneFont( name_tag->Get_Value(), resource_tag->Get_Value() ); }

	return false;
}


CutsceneSystem::Event* LoadCutsceneEvent( const XML_Tag& event_tag )
{
	//  Ensure that the given tag has a type
	const std::string event_type = event_tag.Get_Element( "type" );
	FAIL_IF ( event_type.length() == 0 ) { return false; }
	
	//  Go through the list of event types and act accordingly (TODO: Optimize )
	CutsceneSystem::Event* new_event = new CutsceneSystem::Event;
	if		  ( event_type.compare( "SpriteChange" ) == 0 )	{ new_event->Type = CutsceneSystem::Event::ET_SPRITECHANGE; }
	else if ( event_type.compare( "WarpTo" ) == 0 )			{ new_event->Type = CutsceneSystem::Event::ET_WARPTO; }
	else if ( event_type.compare( "MoveTo" ) == 0 )			{ new_event->Type = CutsceneSystem::Event::ET_MOVETO; }
	else if ( event_type.compare( "SoundPlay" ) == 0 )		{ new_event->Type = CutsceneSystem::Event::ET_SOUNDPLAY; }
	else if ( event_type.compare( "Wait" ) == 0 )			{ new_event->Type = CutsceneSystem::Event::ET_WAIT; }
	else if ( event_type.compare( "Alpha" ) == 0 )			{ new_event->Type = CutsceneSystem::Event::ET_ALPHA; }
	else if ( event_type.compare( "SetFont" ) == 0 )		{ new_event->Type = CutsceneSystem::Event::ET_SETFONT; }
	else if ( event_type.compare( "SetText" ) == 0 )		{ new_event->Type = CutsceneSystem::Event::ET_SETTEXT; }
	else { return false; }

	//  Ensure that the event has a Trigger and that it has a value, then assign it
	const XML_Tag* trigger_tag = event_tag.Find_Child( "Trigger", 1 );
	FAIL_IF ( "Event tag has no Trigger." && trigger_tag == NULL ) { return false; }
	FAIL_IF ( "Event Trigger tag has no value." && trigger_tag->Get_Value() == NULL ) { return false; }
	new_event->Trigger = trigger_tag->Get_Value();

	//  Check if the event has a PassMethod and see if it has a value, then assign it if it does
	const XML_Tag* pass_method_tag = event_tag.Find_Child( "PassMethod", 1 );
	if ( pass_method_tag != NULL && pass_method_tag->Get_Value() != 0 )
	{
		new_event->PassMethod = pass_method_tag->Get_Value();
	}

	//  Check if the event has an EndingEvent and see if it has a value, then assign it if it does
	const XML_Tag* ending_event_tag = event_tag.Find_Child( "EndingEvent", 1 );
	if ( ending_event_tag != NULL && ending_event_tag->Get_Value() != 0 )
	{
		new_event->EndingEvent = ending_event_tag->Get_Value();
	}

	//  Check if the event has a Target and see if it has a value, then assign it if it does
	const XML_Tag* target_tag = event_tag.Find_Child( "Target", 1 );
	if ( target_tag != NULL && target_tag->Get_Value() != 0 )
	{
		new_event->Target = target_tag->Get_Value();
	}

	//  Check if the event has a X and see if it has a value, then assign it if it does
	const XML_Tag* x_tag = event_tag.Find_Child( "X", 1 );
	if ( x_tag != NULL )
	{
		StringToFloat( x_tag->Get_Value(), new_event->X );
	}

	//  Check if the event has a Y and see if it has a value, then assign it if it does
	const XML_Tag* y_tag = event_tag.Find_Child( "Y", 1 );
	if ( y_tag != NULL )
	{
		StringToFloat( y_tag->Get_Value(), new_event->Y );
	}

	//  Check if the event has an Time and see if it has a value, then assign it if it does
	const XML_Tag* time_tag = event_tag.Find_Child( "Time", 1 );
	if ( time_tag != NULL )
	{
		StringToFloat( time_tag->Get_Value(), new_event->Time );
	}

	//  Check if the event has an Asset and see if it has a value, then assign it if it does
	const XML_Tag* asset_tag = event_tag.Find_Child( "Asset", 1 );
	if ( asset_tag != NULL )
	{

		new_event->Asset = asset_tag->Get_Value() ? asset_tag->Get_Value() : "";
	}

	return new_event;
}


CutsceneSystem::Resource* LoadCutsceneSprite( const char* name, const char* file )
{
	//  Ensure that both the name and file are valid strings
	FAIL_IF ( "No name given for sprite object." && name == NULL ) { return false; }
	FAIL_IF ( "No name given for sprite file." && file == NULL ) { return false; }

	//  Create and fill in the new Sprite
	ResourceSprite* new_asset = new ResourceSprite;
	new_asset->ResourceType = CutsceneSystem::Resource::RES_IMAGE;
	new_asset->Name = name;
	new_asset->Asset = file;
	new_asset->Image.LoadSprite( file );

	return (CutsceneSystem::Resource*)(new_asset);
}


CutsceneSystem::Resource* LoadCutsceneSound( const char* name, const char* file )
{
	//  Ensure that both the name and file are valid strings
	FAIL_IF ( "No name given for sound object." && name == NULL ) { return false; }
	FAIL_IF ( "No name given for sound file." && file == NULL ) { return false; }

	//  Create and fill in the new Sound
	ResourceSound* new_asset = new ResourceSound;
	new_asset->ResourceType = CutsceneSystem::Resource::RES_SOUND;
	new_asset->Name = name;
	new_asset->Asset = file;
	//  TODO:  Load in the sound (requires a sound system, lol)

	return (CutsceneSystem::Resource*)(new_asset);
}


CutsceneSystem::Resource* LoadCutsceneFont( const char* name, const char* file )
{
	//  Ensure that both the name and file are valid strings
	FAIL_IF ( "No name given for font object." && name == NULL ) { return false; }
	FAIL_IF ( "No name given for font file." && file == NULL ) { return false; }

	//  Create and fill in the new Font
	ResourceFont* new_asset = new ResourceFont;
	new_asset->ResourceType = CutsceneSystem::Resource::RES_FONT;
	new_asset->Name = name;
	new_asset->Asset = file;
	FontController::Get_Instance()->Load_Font( new_asset->Asset.c_str() );
	new_asset->CutsceneFont = FontController::Get_Instance()->Get_Font( new_asset->Asset.c_str() );

	return (CutsceneSystem::Resource*)(new_asset);
}