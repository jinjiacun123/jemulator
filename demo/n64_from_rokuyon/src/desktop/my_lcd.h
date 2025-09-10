#ifndef MY_LCD_H
#define MY_LCD_H

#include "../common.h"

#include <SDL2/SDL.h>
#include <GL/gl.h> // Or include your GLAD header
#include <chrono>


#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 360

class myLcd
{
    public:
       myLcd();
	   void setView();
	   void draw();
	   void process();
	   ~myLcd();

    private:
       SDL_Window* 		m_window;
	   SDL_GLContext 	m_context;
	   GLuint 			m_texture;
       int 				m_frameCount 	= 0;
       int 				m_swapInterval 	= 0;	   
	   int 				m_refreshRate 	= 0;
	   std::chrono::steady_clock::time_point m_lastRateTime;
};

#endif // MY_LCD_H

