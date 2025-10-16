#include <gpu/gp0.h>
#include <ui/termcolour.h>

extern renderstack_t renderstack;

void m_gpu_gp0_handler(m_simplestation_state *m_simplestation)
{
    switch (m_simplestation->m_gpu->m_gp0_cmd_ins)
    {
            case 0x00:case 0x01:case 0x28:case 0x2C:
            case 0x2D:case 0x2F:case 0x30:case 0x38:case 0x40:case 0x64:			
            case 0x65:case 0x68:case 0xC0:case 0xE2:case 0xE6:                
			break;

            case 0x02:
                renderstack.gpu_fill_rect(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;            
           
            case 0xA0:
                renderstack.gpu_image_draw(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;

            case 0xE1:
                m_gpu_set_draw_mode(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;

            case 0xE3:
                m_gpu_set_draw_area_top_left(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;

            case 0xE4:
                m_gpu_set_draw_area_bottom_right(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;

            case 0xE5:
                m_gpu_set_draw_offset(m_simplestation->m_gpu->m_gp0_instruction, m_simplestation);
                break;

            default:
                printf(RED "[GPU] gp0: Unhandled GP0 Opcode: 0x%02X\n" NORMAL, m_simplestation->m_gpu->m_gp0_cmd_ins);
                m_simplestation_exit(m_simplestation, 1);
                break;
    }
}

extern GLuint m_psx_vram_texel;
uint32_t m_current_idx = 0;

void m_gpu_gp0(uint32_t m_value, m_simplestation_state *m_simplestation)
{
    m_simplestation->m_gpu->m_gp0_instruction = (m_value >> 24) & 0xFF;
    
    if (m_simplestation->m_gpu->m_gp0_words_remaining == 0)
    {
        m_gpu_command_buffer_clear(m_simplestation);
        
        m_simplestation->m_gpu->m_gp0_cmd_ins = m_simplestation->m_gpu->m_gp0_instruction;

        switch (m_simplestation->m_gpu->m_gp0_instruction)
        {
            case 0x00:
            case 0x01:
			case 0x28:
            case 0x2C:
            case 0x2D:
			case 0x30:
            case 0x38:
            case 0x40:
            case 0x64:
            case 0x65:
            case 0x68:
			case 0xC0:
			case 0xE2:
			case 0xE6:
                break;

            case 0x02:
                m_simplestation->m_gpu->m_gp0_words_remaining = 3;
                break;

            case 0x2F:COLLECTIONS;
                m_simplestation->m_gpu->m_gp0_words_remaining = 9;
                break;

            case 0xA0:
                m_simplestation->m_gpu->m_gp0_words_remaining = 3;
                break;

            case 0xE1:
                m_simplestation->m_gpu->m_gp0_words_remaining = 1;
                break;

            case 0xE3:
                m_simplestation->m_gpu->m_gp0_words_remaining = 1;
                break;

            case 0xE4:
                m_simplestation->m_gpu->m_gp0_words_remaining = 1;
                break;

            case 0xE5:
                m_simplestation->m_gpu->m_gp0_words_remaining = 1;
                break;

            default:COLLECTIONS;
                printf(RED "[GPU] gp0: Unhandled GP0 Opcode: 0x%02X (Full Opcode: 0x%08X)\n" NORMAL, m_simplestation->m_gpu->m_gp0_instruction, m_value);
                m_simplestation_exit(m_simplestation, 1);
                break;
        }
    }
    m_simplestation->m_gpu->m_gp0_words_remaining -= 1;

    switch (m_simplestation->m_gpu->m_gp0_write_mode)
    {
        case command:
            m_gpu_command_buffer_push_word(m_simplestation, m_value);

            if (m_simplestation->m_gpu->m_gp0_words_remaining == 0)
            {
                m_gpu_gp0_handler(m_simplestation);
            }
            break;

        case cpu_to_vram:
	{
            uint16_t width = m_simplestation->m_gpu_command_buffer->m_buffer[2] & 0xffff;
			uint16_t height = m_simplestation->m_gpu_command_buffer->m_buffer[2] >> 16;
            
            width = ((width - 1) & 0x3ff) + 1;
            height = ((height - 1) & 0x1ff) + 1;

			int32_t x = m_simplestation->m_gpu_command_buffer->m_buffer[1] & 0x3ff;
			int32_t y = (m_simplestation->m_gpu_command_buffer->m_buffer[1] >> 16) & 0x1ff;;

            //if (m_value = 0x00) m_value = 0x0F;
            
            m_simplestation->m_gpu->write_buffer[m_current_idx++] = m_value;

            if (m_simplestation->m_gpu->m_gp0_words_remaining == 0)
            {
                if (m_simplestation->renderer == OPENGL)
                {
                	static bool is_first = false;
					if(!is_first)
					{
						x = y = 0;
						int length = (1024 * 512);
						char *filePath = "/tmp/psx_data.bin";
#if 1
						FILE *file = fopen(filePath, "wb");
						
	                	#if 1 //[by jim]
	                    glBindTexture(GL_TEXTURE_2D, m_psx_vram_texel);
	                    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, m_simplestation->m_gpu->write_buffer);
						printf("x:0x%02x,y:0x%02x,width:0x%02x,height:0x%02x\n",
							x, y, width, height);
						glBindTexture(GL_TEXTURE_2D, 0);
						#endif
						is_first = true;						
						fwrite(m_simplestation->m_gpu->write_buffer, 1, length, file);
						fclose(file);
#else
					    FILE *file = fopen(filePath, "rb");
						uint32_t write_buffer[(1024 * 512)] = {0};
						fread(write_buffer, 1, length, file);
	                	#if 1 //[by jim]
	                    glBindTexture(GL_TEXTURE_2D, m_psx_vram_texel);
	                    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV, write_buffer);
						printf("x:0x%02x,y:0x%02x,width:0x%02x,height:0x%02x\n",
							x, y, width, height);
						glBindTexture(GL_TEXTURE_2D, 0);
						#endif
						is_first = true;						
						fclose(file);
#endif						
					}
					m_sync_vram(m_simplestation);
                }

                for (int i = 0; i < (1024 * 512); i++) m_simplestation->m_gpu->write_buffer[i] = 0;
                m_current_idx = 0;
                m_simplestation->m_gpu->m_gp0_write_mode = command;
            }

            break;
	}

        default:
            printf(YELLOW "[GP0] Unknown GP0 mode!\n" NORMAL);
            break;
    }
}

void m_gpu_set_draw_mode(uint32_t m_value, m_simplestation_state *m_simplestation)
{
    m_simplestation->m_gpu->m_page_base_x = ((uint8_t) (m_simplestation->m_gpu_command_buffer->m_buffer[0] & 0xF));
    m_simplestation->m_gpu->m_page_base_y = ((uint8_t) ((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 4) & 1));
    m_simplestation->m_gpu->m_semitransparency = ((uint8_t) ((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 5) & 3));

    switch ((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 7) & 3)
    {
        case 0:
            m_simplestation->m_gpu->m_texture_depth = t4bit;
            break;

        case 1:
        case 2:
			break;
        default:
            printf(RED "[GPU] set_draw_mode: Unknown texture depth value!\n" NORMAL);
            m_simplestation_exit(m_simplestation, 1);
            break;
    }

    m_simplestation->m_gpu->m_dithering = (((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 9) & 1) != 0);
    m_simplestation->m_gpu->m_draw_to_display = (((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 10) & 1) != 0);
    m_simplestation->m_gpu->m_texture_disable = (((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 11) & 1) != 0);
    m_simplestation->m_gpu->m_rectangle_texture_x_flip = (((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 12) & 1) != 0);
    m_simplestation->m_gpu->m_rectangle_texture_y_flip = (((m_simplestation->m_gpu_command_buffer->m_buffer[0] >> 13) & 1) != 0);
}


void m_gpu_set_draw_area_top_left(uint32_t m_value, m_simplestation_state *m_simplestation)//[by jim] need
{
    uint32_t m_val = m_simplestation->m_gpu_command_buffer->m_buffer[0];
	printf("function:%s, m_value:%d\n", __FUNCTION__, m_val);
    m_simplestation->m_gpu->m_drawing_area_left = m_val & 0x3FF;
    m_simplestation->m_gpu->m_drawing_area_top = ((m_val >> 10) & 0x1FF);
    m_renderer_update_display_area(m_simplestation);
}

void m_gpu_set_draw_area_bottom_right(uint32_t m_value, m_simplestation_state *m_simplestation)//[by jim] need
{
    uint32_t m_val = m_simplestation->m_gpu_command_buffer->m_buffer[0];
	printf("function:%s, m_value:%d\n", __FUNCTION__, m_val);
    m_simplestation->m_gpu->m_drawing_area_right = m_val & 0x3FF;
    m_simplestation->m_gpu->m_drawing_area_bottom = ((m_val >> 10) & 0x1FF);
    m_renderer_update_display_area(m_simplestation);
}

extern GLint uniform_offset;

void m_gpu_set_draw_offset(uint32_t m_value, m_simplestation_state *m_simplestation)
{
    if (m_simplestation->renderer == OPENGL) draw(m_simplestation, false, false, false);
    
    uint16_t m_x = ((uint16_t) (m_value & 0x7FF));
    uint16_t m_y = ((uint16_t) ((m_value >> 11) & 0x7FF));


    if (m_simplestation->renderer == OPENGL) glUniform2i(uniform_offset, (GLint) (((int16_t) (m_x << 5)) >> 5), (GLint) (((int16_t) (m_y << 5)) >> 5));
}
