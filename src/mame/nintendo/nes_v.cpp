// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Fabio Priuli
/***************************************************************************

  video/nes.c

  Routines to control the unique NES video hardware/PPU.

***************************************************************************/

#include "emu.h"
#include "nes.h"

void nes_state::video_reset()
{
	m_ppu->set_vidaccess_callback(*this, FUNC(nes_state::nes_ppu_vidaccess));
}

void nes_state::video_start()
{
	m_last_frame_flip =  0;
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t nes_state::screen_update_nes(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// render the ppu
	m_ppu->render(bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void nes_state::screen_vblank_nes(int state)
{
	// on rising edge
	if (!state)
	{
		// if this is a disk system game, check for the flip-disk key
		if ((m_cartslot && m_cartslot->exists() && (m_cartslot->get_pcb_id() == STD_DISKSYS))   // first scenario = disksys in m_cartslot (= famicom)
				|| m_disk)  // second scenario = disk via fixed internal disk option (fds & famitwin)
		{
			if (m_io_disksel)
			{
				// latch this input so it doesn't go at warp speed
				if ((m_io_disksel->read() & 0x01) && (!m_last_frame_flip))
				{
					if (m_disk)
						m_disk->disk_flip_side();
					else
						m_cartslot->disk_flip_side();
					m_last_frame_flip = 1;
				}

				if (!(m_io_disksel->read() & 0x01))
					m_last_frame_flip = 0;
			}
		}
	}
}
