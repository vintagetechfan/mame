// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************
    vtech2.c

    video hardware
    Juergen Buchmueller <pullmoll@t-online.de> MESS driver, Jan 2000
    Davide Moretti <dave@rimini.com> ROM dump and hardware description

****************************************************************************/

#include "emu.h"

#include "vtech2.h"

#define BORDER_H    64
#define BORDER_V    32

/* public */

/*
 *  when in text mode (bit 3 of I/O area = 0)
 *  bit 3 2 1 0 mode
 *      0 x x 0 40 columns x 24 rows display RAM at bank 7 1F800-1FFFF
 *      1 x x 0 40 columns x 24 rows display RAM at bank 3 0F800-0FFFF
 *      0 x x 1 80 columns x 24 rows display RAM at bank 7 1F800-1FFFF
 *      1 x x 1 80 columns x 24 rows display RAM at bank 3 0F800-0FFFF
 *
 *  when in graphics mode (bit 3 of I/O area = 1)
 *  bit 3 2 1 0 mode
 *      0 0 0 x GR5 bank 7 1C000-1FFFF
 *      1 0 0 x GR5 bank 3 0C000-0FFFF
 *      0 0 1 0 GR4 bank 7 1C000-1FFFF
 *      1 0 1 0 GR4 bank 3 0C000-0FFFF
 *      0 0 1 1 GR3 bank 7 1C000-1FFFF
 *      1 0 1 1 GR3 bank 3 1C000-1FFFF
 *      0 1 0 x GR0 bank 7 1E000-1FFFF
 *      1 1 0 x GR0 bank 3 0E000-0FFFF
 *      0 1 1 0 GR2 bank 7 1E000-1FFFF
 *      1 1 1 0 GR2 bank 3 0E000-0FFFF
 *      0 1 1 1 GR1 bank 7 1E000-1FFFF
 *      1 1 1 1 GR1 bank 3 0E000-0FFFF
 */


static const int offs_2[192] = {
	0x0000,0x0800,0x1000,0x1800,0x2000,0x2800,0x3000,0x3800,
	0x0100,0x0900,0x1100,0x1900,0x2100,0x2900,0x3100,0x3900,
	0x0200,0x0a00,0x1200,0x1a00,0x2200,0x2a00,0x3200,0x3a00,
	0x0300,0x0b00,0x1300,0x1b00,0x2300,0x2b00,0x3300,0x3b00,
	0x0400,0x0c00,0x1400,0x1c00,0x2400,0x2c00,0x3400,0x3c00,
	0x0500,0x0d00,0x1500,0x1d00,0x2500,0x2d00,0x3500,0x3d00,
	0x0600,0x0e00,0x1600,0x1e00,0x2600,0x2e00,0x3600,0x3e00,
	0x0700,0x0f00,0x1700,0x1f00,0x2700,0x2f00,0x3700,0x3f00,
	0x0050,0x0850,0x1050,0x1850,0x2050,0x2850,0x3050,0x3850,
	0x0150,0x0950,0x1150,0x1950,0x2150,0x2950,0x3150,0x3950,
	0x0250,0x0a50,0x1250,0x1a50,0x2250,0x2a50,0x3250,0x3a50,
	0x0350,0x0b50,0x1350,0x1b50,0x2350,0x2b50,0x3350,0x3b50,
	0x0450,0x0c50,0x1450,0x1c50,0x2450,0x2c50,0x3450,0x3c50,
	0x0550,0x0d50,0x1550,0x1d50,0x2550,0x2d50,0x3550,0x3d50,
	0x0650,0x0e50,0x1650,0x1e50,0x2650,0x2e50,0x3650,0x3e50,
	0x0750,0x0f50,0x1750,0x1f50,0x2750,0x2f50,0x3750,0x3f50,
	0x00a0,0x08a0,0x10a0,0x18a0,0x20a0,0x28a0,0x30a0,0x38a0,
	0x01a0,0x09a0,0x11a0,0x19a0,0x21a0,0x29a0,0x31a0,0x39a0,
	0x02a0,0x0aa0,0x12a0,0x1aa0,0x22a0,0x2aa0,0x32a0,0x3aa0,
	0x03a0,0x0ba0,0x13a0,0x1ba0,0x23a0,0x2ba0,0x33a0,0x3ba0,
	0x04a0,0x0ca0,0x14a0,0x1ca0,0x24a0,0x2ca0,0x34a0,0x3ca0,
	0x05a0,0x0da0,0x15a0,0x1da0,0x25a0,0x2da0,0x35a0,0x3da0,
	0x06a0,0x0ea0,0x16a0,0x1ea0,0x26a0,0x2ea0,0x36a0,0x3ea0,
	0x07a0,0x0fa0,0x17a0,0x1fa0,0x27a0,0x2fa0,0x37a0,0x3fa0
};

static const int offs_1[192] = {
	0x2000,0x2080,0x2800,0x2880,0x3000,0x3080,0x3800,0x3880,
	0x2100,0x2180,0x2900,0x2980,0x3100,0x3180,0x3900,0x3980,
	0x2200,0x2280,0x2a00,0x2a80,0x3200,0x3280,0x3a00,0x3a80,
	0x2300,0x2380,0x2b00,0x2b80,0x3300,0x3380,0x3b00,0x3b80,
	0x2400,0x2480,0x2c00,0x2c80,0x3400,0x3480,0x3c00,0x3c80,
	0x2500,0x2580,0x2d00,0x2d80,0x3500,0x3580,0x3d00,0x3d80,
	0x2600,0x2680,0x2e00,0x2e80,0x3600,0x3680,0x3e00,0x3e80,
	0x2700,0x2780,0x2f00,0x2f80,0x3700,0x3780,0x3f00,0x3f80,
	0x2028,0x20a8,0x2828,0x28a8,0x3028,0x30a8,0x3828,0x38a8,
	0x2128,0x21a8,0x2928,0x29a8,0x3128,0x31a8,0x3928,0x39a8,
	0x2228,0x22a8,0x2a28,0x2aa8,0x3228,0x32a8,0x3a28,0x3aa8,
	0x2328,0x23a8,0x2b28,0x2ba8,0x3328,0x33a8,0x3b28,0x3ba8,
	0x2428,0x24a8,0x2c28,0x2ca8,0x3428,0x34a8,0x3c28,0x3ca8,
	0x2528,0x25a8,0x2d28,0x2da8,0x3528,0x35a8,0x3d28,0x3da8,
	0x2628,0x26a8,0x2e28,0x2ea8,0x3628,0x36a8,0x3e28,0x3ea8,
	0x2728,0x27a8,0x2f28,0x2fa8,0x3728,0x37a8,0x3f28,0x3fa8,
	0x2050,0x20d0,0x2850,0x28d0,0x3050,0x30d0,0x3850,0x38d0,
	0x2150,0x21d0,0x2950,0x29d0,0x3150,0x31d0,0x3950,0x39d0,
	0x2250,0x22d0,0x2a50,0x2ad0,0x3250,0x32d0,0x3a50,0x3ad0,
	0x2350,0x23d0,0x2b50,0x2bd0,0x3350,0x33d0,0x3b50,0x3bd0,
	0x2450,0x24d0,0x2c50,0x2cd0,0x3450,0x34d0,0x3c50,0x3cd0,
	0x2550,0x25d0,0x2d50,0x2dd0,0x3550,0x35d0,0x3d50,0x3dd0,
	0x2650,0x26d0,0x2e50,0x2ed0,0x3650,0x36d0,0x3e50,0x3ed0,
	0x2750,0x27d0,0x2f50,0x2fd0,0x3750,0x37d0,0x3f50,0x3fd0
};

static const int offs_0[96] = {
	0x2000,0x2800,0x3000,0x3800,0x2100,0x2900,0x3100,0x3900,
	0x2200,0x2a00,0x3200,0x3a00,0x2300,0x2b00,0x3300,0x3b00,
	0x2400,0x2c00,0x3400,0x3c00,0x2500,0x2d00,0x3500,0x3d00,
	0x2600,0x2e00,0x3600,0x3e00,0x2700,0x2f00,0x3700,0x3f00,
	0x2050,0x2850,0x3050,0x3850,0x2150,0x2950,0x3150,0x3950,
	0x2250,0x2a50,0x3250,0x3a50,0x2350,0x2b50,0x3350,0x3b50,
	0x2450,0x2c50,0x3450,0x3c50,0x2550,0x2d50,0x3550,0x3d50,
	0x2650,0x2e50,0x3650,0x3e50,0x2750,0x2f50,0x3750,0x3f50,
	0x20a0,0x28a0,0x30a0,0x38a0,0x21a0,0x29a0,0x31a0,0x39a0,
	0x22a0,0x2aa0,0x32a0,0x3aa0,0x23a0,0x2ba0,0x33a0,0x3ba0,
	0x24a0,0x2ca0,0x34a0,0x3ca0,0x25a0,0x2da0,0x35a0,0x3da0,
	0x26a0,0x2ea0,0x36a0,0x3ea0,0x27a0,0x2fa0,0x37a0,0x3fa0
};

uint32_t vtech2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int offs, x, y;
	int full_refresh = 1;
	int lang_offs = 0;
	if (m_language == 0x10)
		lang_offs = 0x300;
	else
	if (m_language == 0x20)
		lang_offs = 0x200;

	if( full_refresh )
		bitmap.fill(((m_laser_bg_mode >> 4) & 15)<<1, cliprect);

	if (m_laser_latch & 0x08)
	{
		/* graphics modes */
		switch (m_laser_bg_mode & 7)
		{
		case  0:
		case  1:
			/*
			 * GR 5
			 * 640x192 1bpp
			 */
			for( y = 0; y < 192; y++ )
			{
				offs = offs_2[y];
				for( x = 0; x < 80; x++, offs++ )
				{
					int sx, sy, code, color = m_laser_two_color;
					sy = BORDER_V/2 + y;
					sx = BORDER_H/2 + x * 8;
					code = m_vram[offs];
					m_gfxdecode->gfx(2)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
			break;

		case 2:
			/*
			 * GR 4
			 * 320x192 1bpp with two colors per 8 pixels
			 */
			for( y = 0; y < 192; y++ )
			{
				offs = offs_2[y];
				for( x = 0; x < 40; x++, offs += 2 )
				{
					int sx, sy, code, color;
					sy = BORDER_V/2 + y;
					sx = BORDER_H/2 + x * 16;
					code = m_vram[offs];
					color = m_vram[offs+1];
					m_gfxdecode->gfx(3)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
			break;

		case  3:
			/*
			 * GR 3
			 * 160x192 4bpp
			 */
			for( y = 0; y < 192; y++ )
			{
				offs = offs_2[y];
				for( x = 0; x < 80; x++, offs++ )
				{
					int sx, sy, code;
					sy = BORDER_V/2 + y;
					sx = BORDER_H/2 + x * 8;
					code = m_vram[offs];
					m_gfxdecode->gfx(5)->opaque(bitmap,cliprect,code,0,0,0,sx,sy);
				}
			}
			break;

		case  4:
		case  5:
			/*
			 * GR 0
			 * 160x96 4bpp
			 */
			for( y = 0; y < 96; y++ )
			{
				offs = offs_0[y];
				for( x = 0; x < 80; x++, offs++ )
				{
					int sx, sy, code;
					sy = BORDER_V/2 + y * 2;
					sx = BORDER_H/2 + x * 8;
					code = m_vram[offs];
					m_gfxdecode->gfx(6)->opaque(bitmap,cliprect,code,0,0,0,sx,sy);
				}
			}
			break;

		case  6:
			/*
			 * GR 2
			 * 320x192 1bpp
			 */
			for( y = 0; y < 192; y++ )
			{
				offs = offs_1[y];
				for( x = 0; x < 40; x++, offs++ )
				{
					int sx, sy, code, color = m_laser_two_color;
					sy = BORDER_V/2 + y;
					sx = BORDER_H/2 + x * 16;
					code = m_vram[offs];
					m_gfxdecode->gfx(3)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
			break;

		case  7:
			/*
			 * GR 1
			 * 160x192 1bpp with two colors per 8 pixels
			 */
			for( y = 0; y < 192; y++ )
			{
				offs = offs_1[y];
				for( x = 0; x < 20; x++, offs += 2 )
				{
					int sx, sy, code, color;
					sy = BORDER_V/2 + y;
					sx = BORDER_H/2 + x * 32;
					code = m_vram[offs];
					color = m_vram[offs+1];
					m_gfxdecode->gfx(4)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
			break;
		}
	}
	else
	{
		/* text modes */
		if (m_laser_bg_mode & 1)
		{
			/* 80 columns text mode */
			for( y = 0; y < 24; y++ )
			{
				offs = ((y & 7) << 8) + ((y >> 3) * 80);
				for( x = 0; x < 80; x++, offs++ )
				{
					int sx, sy, code, color = m_laser_two_color;
					sy = BORDER_V/2 + y * 8;
					sx = BORDER_H/2 + x * 8;
					code = m_vram[0x3800+offs] + lang_offs;
					m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
		}
		else
		{
			/* 40 columns text mode */
			for( y = 0; y < 24; y++ )
			{
				offs = ((y & 7) << 8) + ((y >> 3) * 80);
				for( x = 0; x < 40; x++, offs += 2 )
				{
					int sx, sy, code, color = 0;
					sy = BORDER_V/2 + y * 8;
					sx = BORDER_H/2 + x * 16;
					code = m_vram[0x3800+offs] + lang_offs;
					color = m_vram[0x3801+offs];
					m_gfxdecode->gfx(1)->opaque(bitmap,cliprect,code,color,0,0,sx,sy);
				}
			}
		}
	}

	if( m_laser_frame_time > 0 )
	{
		popmessage("%s", m_laser_frame_message);
	}
	return 0;
}

void vtech2_state::laser_bg_mode_w(uint8_t data)
{
	if (m_laser_bg_mode != data)
	{
		m_laser_bg_mode = data;
		logerror("laser border:$%X mode:$%X\n", data >> 4, data & 15);
	}
}

void vtech2_state::laser_two_color_w(uint8_t data)
{
	if (m_laser_two_color != data)
	{
		m_laser_two_color = data;
		logerror("laser foreground:$%X background:$%X\n", data >> 4, data & 15);
	}
}
