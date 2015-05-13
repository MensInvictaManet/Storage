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
/*  File:       XML_File.h                                       */
/*                                                               */
/*  Purpose:    This file contains functionality to load an XML  */
/*              file into memory and confirm or deny valid XML   */
/*              structure and syntax before loading in its data. */
/*                                                               */
/*  Created:    08/27/2009                                       */
/*  Last Edit:  08/27/2009                                       */
/*****************************************************************/

#ifndef _XML_FILE_
#define _XML_FILE_

#include <fstream>
#include <stack>
#include <vector>

#define MAX_LINE_LENGTH		2048

// Classes
class XML_Tag
{
private:
	char* Name;
	char* Value;
	bool Enclosed;

public:
	XML_Tag();
	~XML_Tag();
	bool Define_Tag_Name( char* tag_name );
	bool Define_Tag_Value( char* tag_value );
	void Define_Tag_Enclosed( const bool enclosed ) { Enclosed = enclosed; }
	bool Is_Same_Tag( char* tag_name );
	
	inline const char*	Get_Name( void )		const { return Name; }
	inline const char*	Get_Value( void )		const { return Value; }
	inline const bool		Get_Enclosed( void )	const { return Enclosed; }

	XML_Tag*				Parent;

	typedef std::vector<XML_Tag*> ChildListType;
	typedef ChildListType::const_iterator ChildListIterType;
	ChildListType	Children;

	typedef std::vector< std::pair<std::string, std::string> > ElementListType;
	ElementListType Elements;

	const XML_Tag* Find_Child( const char* string, unsigned int depth ) const;
	const std::string Get_Element( std::string, bool allowed_to_fail = false ) const;

	void Set_Element(std::string name, std::string value);
};

class XML_File
{
	//~  Member Functions
	private:
		XML_File();
		
		//  Reading
		bool Load_XML_Data( const char* xml_file );
		void Iterate_Whitespace(  char* string, unsigned int& index );
		bool Add_XML_Tag(  char* string, unsigned int& index );
		bool Define_Tag_Value( char* string, unsigned int& index );
		bool Remove_XML_Tag( char* string, unsigned int& index );
		bool Add_XML_Version_Info( char* string, unsigned int& index, char* version_string, char* xml_encoding );
		bool Begin_Comment( char* string, unsigned int& index , bool& within_comment );
		bool Iterate_Comment( char* string, unsigned int& index , bool& within_comment  );
		bool End_Of_XML_Line( char* string, unsigned int& index );

		//  Writing
		bool Write_XML_Tag( const XML_Tag* xml_tag, std::ofstream& stream, const int depth ) const;

	public:
		static const XML_File* Load_XML_File( const char* xml_file );
		bool Write_XML_File( const char* xml_file ) const;
		~XML_File();

		inline const XML_Tag* Get_Encompassing_Tag( void ) const			{ return &Encompassing_Tag; }

	//~  Member Variables
	private:
		std::stack<XML_Tag*>			XML_Tag_Stack;
		std::vector<XML_Tag*>		XML_Tag_List;
		XML_Tag							Encompassing_Tag;
		XML_Tag*							Current_Tag;

		std::string						VersionString;
		std::string						EncodingString;

	public:
};

#endif