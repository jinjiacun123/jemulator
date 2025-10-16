
#include <simplestation.h>
#include <renderer/renderer.h>

renderstack_t renderstack;
event_t *events;

int main(int argc, char **argv)
{
	m_simplestation_state m_simplestation;

    m_simplestation.m_gpu_state = OFF;
    m_simplestation.m_gpu_command_buffer_state = OFF;

    if (m_gpu_init(&m_simplestation, &renderstack) != 0){
			// If GPU couldn't be initialized, exit out
			m_simplestation_exit(&m_simplestation, 1);
			return 0;
	}

    glfwInit();

    renderstack.display(&m_simplestation);


    sleep(8);
}

uint8_t m_simplestation_exit(m_simplestation_state *m_simplestation, uint8_t m_is_fatal)
{
    return 0;
}