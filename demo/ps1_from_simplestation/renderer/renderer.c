#include <renderer/renderer.h>
#include <ui/termcolour.h>

uint8_t setup_renderer(m_simplestation_state *m_simplestation, renderstack_t *renderstack)
{
    uint8_t m_result;

    switch (m_simplestation->renderer)
    {
        case OPENGL:
            m_result = init_opengl_renderer(m_simplestation);
            break;      
                
        default:
            __builtin_unreachable();
            break;
    }

    if (m_result != 0)
    {
        printf(RED "[GPU] setup_renderer: Failed to initialize a renderer!\n" NORMAL);
    }
    else
    {
        switch (m_simplestation->renderer)
        {
            case OPENGL:
                setup_opengl_renderstack(m_simplestation, renderstack);
                break;
                    
            default:
                __builtin_unreachable();
                break;
        }
    }

    return m_result;
}

void setup_opengl_renderstack(m_simplestation_state *m_simplestation, renderstack_t *renderstack)
{
	renderstack->gpu_fill_rect = &m_gpu_fill_rect;
    renderstack->gpu_image_draw = &m_gpu_image_draw;

    renderstack->draw = &draw;
    renderstack->display = &display;
}
