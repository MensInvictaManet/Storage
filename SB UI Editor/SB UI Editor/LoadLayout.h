#ifndef _LOAD_LAYOUT_
#define _LOAD_LAYOUT_

class XML_File;

bool LoadLayoutFile( const char* filename );
void SpecifyLayoutRenderingOrder( const char** objectNames, const int objectCount );
void RenderLayout( void );
void UnloadLayoutFile( void );
void LayoutInput(void);
void LayoutSelect(const int x, const int y);

#endif