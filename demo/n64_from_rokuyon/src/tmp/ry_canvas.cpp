/*
    Copyright 2022-2024 Hydr8gon

    This file is part of rokuyon.

    rokuyon is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    rokuyon is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with rokuyon. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ry_canvas.h"
#include "ry_app.h"
#include "../core.h"
#include "../vi.h"

#ifdef _WIN32
#include <GL/gl.h>
#include <GL/glext.h>
#endif

wxBEGIN_EVENT_TABLE(ryCanvas, wxGLCanvas)
EVT_PAINT(ryCanvas::draw)
EVT_SIZE(ryCanvas::resize)
wxEND_EVENT_TABLE()

ryCanvas::ryCanvas(ryFrame *frame): wxGLCanvas(frame, wxID_ANY, nullptr), frame(frame)
{
#if 1
    // Prepare the OpenGL context
    context = new wxGLContext(this);

    // Set focus so that key presses will be registered
    SetFocus();
#endif	
}

void ryCanvas::finish()
{
    // Tell the canvas to stop rendering
    finished = true;
}

void ryCanvas::draw(wxPaintEvent &event)
{
    // Stop rendering so the program can close
    if (finished)
        return;

    //return;
    SetCurrent(*context);	//[by jim] setting open gl context
    static bool setup = false;
	SetSize(wxSize(480,360));
	frame->SendSizeEvent();
    if (!setup)
    {
        // Prepare a texture for the framebuffer
#if 1
		GLuint texture;
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif		

        // Finish initial setup
#if 1        
		
        frame->SendSizeEvent();
#endif
        setup = true;
    }
	static uint32_t times = 0;
	times++;
	
#if 1
	if (times >1)
		return;
#endif	
	printf("times:%d\n", times);
    // Clear the screen
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

#if 1
    if (Core::running || frame->isPaused())
    {
        // At the swap interval, get the framebuffer as a texture
        if (++frameCount >= swapInterval)
        {
            if (_Framebuffer *fb = VI::getFramebuffer()) //[by jim] get video buffer from emulator n64
            {			  
            #if 1
			#if 0
			   if(times <= 3){	
				   	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb->width,
                    fb->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb->data);
			   		char path[100];
					snprintf(path, sizeof(path), "/tmp/my_video_%d.bin", times);
					// Try to open the specified ROM file
				    FILE *videoBufFile = fopen(path, "wb");	
					if (!videoBufFile) {
						printf("open videoBufFile is failt\n");
						exit(-1);
					}
					printf("ready to write\n");
				    fwrite(fb->data, sizeof(uint32_t), fb->width * fb->height  , videoBufFile);
				    fclose(videoBufFile);
			   	}
			#else
				if(times == 1){
					char path[100];					
					snprintf(path, sizeof(path), "/tmp/my_video_3.bin", times);
					// Try to open the specified ROM file
				    FILE *videoBufFile = fopen(path, "rb");	
					if (!videoBufFile) {
						printf("open videoBufFile is failt\n");
						exit(-1);
					}
					uint32_t buffer_width = 320, buffer_height = 237;
					uint32_t * data = new uint32_t[buffer_width * buffer_height];
					memset(data, 0x0, buffer_width * buffer_height * sizeof(uint32_t));
					printf("ready to read\n");
				    fread(data, sizeof(uint32_t), buffer_width * buffer_height  , videoBufFile);
				    fclose(videoBufFile);
				    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffer_width,
                    buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			   	}
			
			#endif	
			#endif
				printf("fb_width:%d,fb_height:%d,x:%d,y:%d,width:%d,height:%d\n",
					    fb->width, fb->height, x, y, width, height);
                frameCount = 0;
                delete fb;
            }
        }
#endif
		
#if 1
if (times == 1){
		//glColor3f(0.0f,1.0f,0.0f);
	  //uint32_t width = 200, height = 300;
    printf("width:%d,height:%d,x:%d,y:%d\n", width, height, x, y);
        // Submit the polygon vertices
        glBegin(GL_QUADS);	//[by jim] begin draw
        glTexCoord2i(1, 1);
        glVertex2i(x + width, y + height);
        glTexCoord2i(0, 1);
        glVertex2i(x, y + height);
        glTexCoord2i(0, 0);
        glVertex2i(x, y);
        glTexCoord2i(1, 0);
        glVertex2i(x + width, y);
        glEnd();	//[by jim] draw finish
}
		
#endif		
#if 1
    }
#endif

    // Track the refresh rate and update the swap interval every second
    // Speed is limited by drawing, so this tries to keep it at 60 Hz
#if 1    
    refreshRate++;
    std::chrono::duration<double> rateTime = std::chrono::steady_clock::now() - lastRateTime;
    if (rateTime.count() >= 1.0f)
    {
        swapInterval = (refreshRate + 5) / 60; // Margin of 5
        refreshRate = 0;
        lastRateTime = std::chrono::steady_clock::now();
    }
#endif
    // Finish the frame
    glFinish();
    SwapBuffers(); //[by jim] refresh screen, like SDL_GL_SwapBuffers();
}

void ryCanvas::resize(wxSizeEvent &event)
{
#if 1
    // Full screen breaks the minimum frame size, but changing to a different value fixes it
    // As a workaround, clear the minimum size on full screen and reset it shortly after
#if 1
    frame->SetMinClientSize(sizeReset ? wxSize(0, 0) : MIN_SIZE);
    sizeReset -= (bool)sizeReset;
#else
	frame->SetMinClientSize(MIN_SIZE);
#endif

    // Update the canvas dimensions
    SetCurrent(*context);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    wxSize size = GetSize();
	//size.x = 480, size.y = 360;
    glOrtho(0, size.x, size.y, 0, -1, 1); //[by jim]glOrtho(left, right, bottom, top, near, far)
    glViewport(0, 0, size.x, size.y);	//[by jim](x, y, width, height)
	printf("size.x:%d, size.y:%d\n", size.x, size.y);

    // Set the layout to be centered and as large as possible
    if (((float)size.x / size.y) > (320.0f / 240)) // Wide
    {
        width = 320 * size.y / 240;
        height = size.y;
        x = (size.x - width) / 2;
        y = 0;
    }
    else // Tall
    {
        width = size.x;
        height = 240 * size.x / 320;
        x = 0;
        y = (size.y - height) / 2;
    }
#endif	
}
