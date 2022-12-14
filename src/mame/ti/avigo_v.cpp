// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/***************************************************************************

  avigo.c

  Functions to emulate the video hardware of the TI Avigo 10 PDA

***************************************************************************/

#include "emu.h"
#include "avigo.h"

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

/* mem size = 0x017c0 */


/* current column to read/write */

#define AVIGO_VIDEO_DEBUG 0
#define LOG(x) do { if (AVIGO_VIDEO_DEBUG) logerror x; } while (0)


uint8_t avigo_state::vid_memory_r(offs_t offset)
{
	if (!offset)
		return m_screen_column;

	if ((offset<0x0100) || (offset>=0x01f0) || (m_screen_column >= (AVIGO_SCREEN_WIDTH>>3)))
	{
		LOG(("vid mem read: %04x\n", offset));
		return 0;
	}

	/* 0x0100-0x01f0 contains data for selected column */
	return m_video_memory[m_screen_column + ((offset&0xff)*(AVIGO_SCREEN_WIDTH>>3))];
}

void avigo_state::vid_memory_w(offs_t offset, uint8_t data)
{
	if (!offset)
	{
		/* select column to read/write */
		m_screen_column = data;

		LOG(("vid mem column write: %02x\n",data));

		if (data>=(AVIGO_SCREEN_WIDTH>>3))
		{
			LOG(("error: vid mem column write: %02x\n",data));
		}
		return;
	}

	if ((offset<0x0100) || (offset>=0x01f0) || (m_screen_column >= (AVIGO_SCREEN_WIDTH>>3)))
	{
		LOG(("vid mem write: %04x %02x\n", offset, data));
		return;
	}

	/* 0x0100-0x01f0 contains data for selected column */
	m_video_memory[m_screen_column + ((offset&0xff)*(AVIGO_SCREEN_WIDTH>>3))] = data;
}

void avigo_state::video_start()
{
	/* current selected column to read/write */
	m_screen_column = 0;

	/* allocate video memory */
	m_video_memory = make_unique_clear<uint8_t[]>((AVIGO_SCREEN_WIDTH>>3) * AVIGO_SCREEN_HEIGHT + 1);

	save_pointer(NAME(m_video_memory), (AVIGO_SCREEN_WIDTH>>3) * AVIGO_SCREEN_HEIGHT + 1);
}

uint32_t avigo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw avigo display */
	for (int y=0; y<AVIGO_SCREEN_HEIGHT; y++)
	{
		uint8_t *line_ptr = &m_video_memory[y*(AVIGO_SCREEN_WIDTH>>3)];

		int x = 0;
		for (int by=((AVIGO_SCREEN_WIDTH>>3)-1); by>=0; by--)
		{
			uint8_t byte = line_ptr[0];

			int px = x;
			for (int b=7; b>=0; b--)
			{
				bitmap.pix(y, px) = ((byte>>7) & 0x01);
				px++;
				byte <<= 1;
			}

			x = px;
			line_ptr++;
		}
	}
	return 0;
}
