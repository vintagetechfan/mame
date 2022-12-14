// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/**********************************************************************

    p2000m.c

    Functions to emulate video hardware of the p2000m

**********************************************************************/

#include "emu.h"
#include "p2000t.h"




void p2000m_state::video_start()
{
	m_frame_count = 0;
}


uint32_t p2000m_state::screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *const videoram = m_videoram;

	for (int offs = 0; offs < 80 * 24; offs++)
	{
		int sy = (offs / 80) * 20;
		int sx = (offs % 80) * 12;

		int code;
		if ((m_frame_count > 25) && (videoram[offs + 2048] & 0x40))
			code = 32;
		else
		{
			code = videoram[offs];
			if ((videoram[offs + 2048] & 0x01) && (code & 0x20))
			{
				code += (code & 0x40) ? 64 : 96;
			} else {
				code &= 0x7f;
			}
			if (code < 32) code = 32;
		}

		m_gfxdecode->gfx(0)->zoom_opaque(bitmap,cliprect, code,
			videoram[offs + 2048] & 0x08 ? 0 : 1, 0, 0, sx, sy, 0x20000, 0x20000);

		if (videoram[offs] & 0x80)
		{
			for (int loop = 0; loop < 12; loop++)
			{
				bitmap.pix(sy + 18, sx + loop) = 0;   /* cursor */
				bitmap.pix(sy + 19, sx + loop) = 0;   /* cursor */
			}
		}
	}

	return 0;
}
