#include "my_lcd.h"
#include "../core.h"
#include "../vi.h"

myLcd::myLcd(){
	// 1. SDL Initialization and Window/OpenGL Context Creation
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // Example for OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); // For glBegin/glEnd

	m_window = SDL_CreateWindow("rokuyou_lab", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    m_context = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_context);

	//setuP
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void myLcd::setView(){
	// open gl setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	glOrtho(0.0, 480.0, 360.0, 0, -1.0, 1.0);//left, right, down, up, near, far
}

void myLcd::draw(){
	 glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black background
     glClear(GL_COLOR_BUFFER_BIT);

	// fill texImage
	 if (Core::running){
		  if (++m_frameCount >= m_swapInterval)
         {
        	 if (_Framebuffer *fb = VI::getFramebuffer()) //[by jim] get video buffer from emulator n64
        	 {				
        	 #if 1
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb->width,
                    fb->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb->data);
				printf("draw tex image 2d width:%d, height:%d\n", fb->width, fb->height);
			#else
				char path[100];					
				snprintf(path, sizeof(path), "/tmp/my_video_3.bin", 0);
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
			#endif
			 }
		  }
	 }
	 
	 uint32_t x = 0, y = 0, width = 200, height = 200;
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

	 // Track the refresh rate and update the swap interval every second
     // Speed is limited by drawing, so this tries to keep it at 60 Hz
#if 1    
    m_refreshRate++;
    std::chrono::duration<double> rateTime = std::chrono::steady_clock::now() - m_lastRateTime;
    if (rateTime.count() >= 1.0f)
    {
        m_swapInterval = (m_refreshRate + 5) / 60; // Margin of 5
        m_refreshRate = 0;
        m_lastRateTime = std::chrono::steady_clock::now();
    }
#endif

	 glFlush();
     // 3. Swap Buffers
     SDL_GL_SwapWindow(m_window);
}


void myLcd::process(){
	SDL_Event event;
	 while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(-1);
        }
     }
}


myLcd::~myLcd(){
	SDL_GL_DeleteContext(m_context);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}
