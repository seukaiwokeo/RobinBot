#pragma once

#include <Windows.h>

class CDrawing
{
public:
	void FillArea( int x, int y, int w, int h, int r, int g, int b, int a );
	void DrawBox( int x, int y, int w, int h, int linewidth, int r, int g, int b, int a );
	int iStringLen( const char *fmt, ... );
	int iStringHeight( void );
	void DrawString( int x, int y, int r, int g, int b, const char *fmt, ... );
	void DrawStringCenter( int x, int y, int r, int g, int b, const char *fmt, ... );
};

extern CDrawing g_Drawing;