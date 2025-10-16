#include <gpu/gpu.h>
#include <gpu/command_buffer.h>
#include <renderer/renderer.h>
#include <ui/termcolour.h>
#include <stdio.h>

uint8_t m_gpu_init(m_simplestation_state *m_simplestation, renderstack_t *renderstack)
{
    uint8_t m_result = 0;

    m_simplestation->m_gpu = (m_psx_gpu_t *) malloc(sizeof(m_psx_gpu_t));
    if (!m_simplestation->m_gpu){
		printf("[GPU] init: Couldn't initialize PSX's GPU, exiting...");
        m_result = 1;
		return m_result;
	}    
    memset(m_simplestation->m_gpu, 0, sizeof(m_psx_gpu_t));
    m_simplestation->m_gpu->m_field = top;
	
    if (m_gpu_command_buffer_init(m_simplestation))
    {
        printf(RED "[GPU] command_buffer_init: Failed to allocate PSX's GPU Command Buffer!\n" NORMAL);
        m_result = 1;
		return m_result;
    }
    else
    {
        m_result = setup_renderer(m_simplestation, renderstack);
        
        if (m_result)
        {
            printf(RED "[GPU] m_gpu_init: Failed to initialize a renderer!\n" NORMAL);
        }
    }

    return m_result;
}


horizontalRes m_gpu_set_horizontal_res(uint8_t fields)
{
	if (!(fields & 1))
	{
		switch ((fields >> 1) & 0x3)
		{
			case 0: 
				return XRes256;
			case 1: 
				return XRes320;
		}
	}
}

void m_gpu_exit(m_simplestation_state *m_simplestation)
{
    if (m_simplestation->m_gpu_state)
    {
        free(m_simplestation->m_gpu);
    }

    m_simplestation->m_gpu_state = OFF;
}
