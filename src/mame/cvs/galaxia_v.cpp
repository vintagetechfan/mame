// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

  Galaxia Video HW

  hardware is derived from cvs

***************************************************************************/

#include "emu.h"
#include "galaxia.h"

#define SPRITE_PEN_BASE     (0x10)
#define STAR_PEN            (0x18)
#define BULLET_PEN          (0x19)


// Colors are 3bpp, but how they are generated is a mystery
// there's no color prom on the pcb, nor palette ram

void galaxia_state::galaxia_palette(palette_device &palette) const
{
	// estimated with video/photo references
	constexpr int lut_clr[0x18] = {
		// background
		0, 1, 4, 5,
		0, 3, 6, 2,
		0, 1, 4, 5, // unused?
		0, 3, 1, 7,

		// sprites
		0, 4, 3, 6, 1, 5, 2, 7
	};

	for (int i = 0; i < 0x18; i++)
		palette.set_pen_color(i, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

void galaxia_state::astrowar_palette(palette_device &palette) const
{
	// no reference material available(?), except for Data East astrof
	constexpr int lut_clr[8] = { 7, 3, 5, 1, 4, 2, 6, 7 };

	for (int i = 0; i < 8; i++)
	{
		// background
		palette.set_pen_color(i*2, 0, 0, 0);
		palette.set_pen_color(i*2 + 1, pal1bit(lut_clr[i] >> 0), pal1bit(lut_clr[i] >> 1), pal1bit(lut_clr[i] >> 2));

		// sprites
		palette.set_pen_color(i | SPRITE_PEN_BASE, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}

	// stars/bullets
	palette.set_pen_color(STAR_PEN, pal1bit(1), pal1bit(1), pal1bit(1));
	palette.set_pen_color(BULLET_PEN, pal1bit(1), pal1bit(1), pal1bit(0));
}

TILE_GET_INFO_MEMBER(galaxia_state::get_galaxia_bg_tile_info)
{
	uint8_t code = m_video_ram[tile_index] & 0x7f; // d7 unused
	uint8_t color = m_color_ram[tile_index] & 3; // highest bits unused

	tileinfo.set(0, code, color, 0);
}

TILE_GET_INFO_MEMBER(galaxia_state::get_astrowar_bg_tile_info)
{
	uint8_t code = m_video_ram[tile_index];
	uint8_t color = m_color_ram[tile_index] & 7; // highest bits unused

	tileinfo.set(0, code, color, 0);
}

void galaxia_state::init_common()
{
	assert((STAR_PEN & 7) == 0);
	cvs_init_stars();
}

void galaxia_state::video_start()
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxia_state::get_galaxia_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);

}

VIDEO_START_MEMBER(galaxia_state,astrowar)
{
	init_common();

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(galaxia_state::get_astrowar_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_cols(8);
	m_bg_tilemap->set_scrolldx(8, 8);

	m_screen->register_screen_bitmap(m_temp_bitmap);
}


/********************************************************************************/

uint32_t galaxia_state::screen_update_galaxia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);
	bitmap_ind16 const &s2636_1_bitmap = m_s2636[1]->update(cliprect);
	bitmap_ind16 const &s2636_2_bitmap = m_s2636[2]->update(cliprect);

	bitmap.fill(0, cliprect);
	cvs_update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			bool const bullet = m_bullet_ram[y] && x == (m_bullet_ram[y] ^ 0xff);
			bool const background = (bitmap.pix(y, x) & 3) != 0;

			// draw bullets (guesswork)
			if (bullet)
			{
				// background vs. bullet collision detection
				if (background) m_collision_register |= 0x80;

				// bullet size/color/priority is guessed
				bitmap.pix(y, x) = BULLET_PEN;
				if (x) bitmap.pix(y, x-1) = BULLET_PEN;
			}

			// copy the S2636 images into the main bitmap and check collision
			int const pixel0 = s2636_0_bitmap.pix(y, x);
			int const pixel1 = s2636_1_bitmap.pix(y, x);
			int const pixel2 = s2636_2_bitmap.pix(y, x);

			int const pixel = pixel0 | pixel1 | pixel2;

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. S2636 collision detection
				if (S2636_IS_PIXEL_DRAWN(pixel0) && S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x01;
				if (S2636_IS_PIXEL_DRAWN(pixel1) && S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x02;
				if (S2636_IS_PIXEL_DRAWN(pixel2) && S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x04;

				// S2636 vs. bullet collision detection
				if (bullet) m_collision_register |= 0x08;

				// S2636 vs. background collision detection
				if (background)
				{
					/* bit4 causes problems on 2nd level
					if (S2636_IS_PIXEL_DRAWN(pixel0)) m_collision_register |= 0x10; */
					if (S2636_IS_PIXEL_DRAWN(pixel1)) m_collision_register |= 0x20;
					if (S2636_IS_PIXEL_DRAWN(pixel2)) m_collision_register |= 0x40;
				}

				bitmap.pix(y, x) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}


uint32_t galaxia_state::screen_update_astrowar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// astrowar has only one S2636
	bitmap_ind16 const &s2636_0_bitmap = m_s2636[0]->update(cliprect);

	bitmap.fill(0, cliprect);
	cvs_update_stars(bitmap, cliprect, STAR_PEN, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	copybitmap(m_temp_bitmap, bitmap, 0, 0, 0, 0, cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		// draw bullets (guesswork)
		if (m_bullet_ram[y])
		{
			uint8_t const pos = m_bullet_ram[y] ^ 0xff;

			// background vs. bullet collision detection
			if (m_temp_bitmap.pix(y, pos) & 1)
				m_collision_register |= 0x02;

			// bullet size/color/priority is guessed
			bitmap.pix(y, pos) = BULLET_PEN;
			if (pos) bitmap.pix(y, pos-1) = BULLET_PEN;
		}

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// NOTE: similar to zac2650.c, the sprite chip runs at a different frequency than the background generator
			// the exact timing ratio is unknown, so we'll have to do with guesswork
			float const s_ratio = 256.0f / 196.0f;

			float const sx = x * s_ratio;
			if (int(sx + 0.5f) > cliprect.right())
				break;

			// copy the S2636 bitmap into the main bitmap and check collision
			int const pixel = s2636_0_bitmap.pix(y, x);

			if (S2636_IS_PIXEL_DRAWN(pixel))
			{
				// S2636 vs. background collision detection
				if ((m_temp_bitmap.pix(y, int(sx)) | m_temp_bitmap.pix(y, int(sx + 0.5f))) & 1)
					m_collision_register |= 0x01;

				bitmap.pix(y, int(sx)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
				bitmap.pix(y, int(sx + 0.5f)) = S2636_PIXEL_COLOR(pixel) | SPRITE_PEN_BASE;
			}
		}
	}

	return 0;
}
