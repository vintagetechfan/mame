// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/***************************************************************************

  ladyfrog.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/
#include "emu.h"
#include "ladyfrog.h"


void ladyfrog_state::ladyfrog_spriteram_w(offs_t offset, uint8_t data)
{
	m_spriteram[offset] = data;
}

uint8_t ladyfrog_state::ladyfrog_spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

TILE_GET_INFO_MEMBER(ladyfrog_state::get_tile_info)
{
	int pal = m_videoram[tile_index * 2 + 1] & 0x0f;
	int tile = m_videoram[tile_index * 2] + ((m_videoram[tile_index * 2 + 1] & 0xc0) << 2)+ ((m_videoram[tile_index * 2 + 1] & 0x30) << 6);
	tileinfo.set(0,
			tile + 0x1000 * m_tilebank,
			pal,TILE_FLIPY
			);
}

void ladyfrog_state::ladyfrog_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

uint8_t ladyfrog_state::ladyfrog_videoram_r(offs_t offset)
{
	return m_videoram[offset];
}

void ladyfrog_state::ladyfrog_palette_w(offs_t offset, uint8_t data)
{
	if (offset & 0x100)
		m_palette->write8_ext((offset & 0xff) + (m_palette_bank << 8), data);
	else
		m_palette->write8((offset & 0xff) + (m_palette_bank << 8), data);
}

uint8_t ladyfrog_state::ladyfrog_palette_r(offs_t offset)
{
	if (offset & 0x100)
		return m_paletteram_ext[(offset & 0xff) + (m_palette_bank << 8)];
	else
		return m_paletteram[(offset & 0xff) + (m_palette_bank << 8)];
}

void ladyfrog_state::ladyfrog_gfxctrl_w(uint8_t data)
{
	m_palette_bank = (data & 0x20) >> 5;
}

void ladyfrog_state::ladyfrog_gfxctrl2_w(uint8_t data)
{
	m_tilebank = ((data & 0x18) >> 3) ^ 3;
	m_bg_tilemap->mark_all_dirty();
}

uint8_t ladyfrog_state::ladyfrog_scrlram_r(offs_t offset)
{
	return m_scrlram[offset];
}

void ladyfrog_state::ladyfrog_scrlram_w(offs_t offset, uint8_t data)
{
	m_scrlram[offset] = data;
	m_bg_tilemap->set_scrolly(offset, data);
}

void ladyfrog_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i;
	for (i = 0; i < 0x20; i++)
	{
		int pr = m_spriteram[0x9f - i];
		int offs = (pr & 0x1f) * 4;
		{
			int code, sx, sy, flipx, flipy, pal;
			code = m_spriteram[offs + 2] + ((m_spriteram[offs + 1] & 0x10) << 4) + m_spritetilebase;
			pal = m_spriteram[offs + 1] & 0x0f;
			sx = m_spriteram[offs + 3];
			sy = 238 - m_spriteram[offs + 0];
			flipx = ((m_spriteram[offs + 1] & 0x40)>>6);
			flipy = ((m_spriteram[offs + 1] & 0x80)>>7);
			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					code,
					pal,
					flipx,flipy,
					sx,sy,15);

			if (m_spriteram[offs + 3] > 240)
			{
				sx = (m_spriteram[offs + 3] - 256);
				m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
						code,
						pal,
						flipx,flipy,
							sx,sy,15);
			}
		}
	}
}

VIDEO_START_MEMBER(ladyfrog_state,ladyfrog_common)
{
	m_spriteram = std::make_unique<uint8_t[]>(160);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ladyfrog_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_paletteram.resize(0x200);
	m_paletteram_ext.resize(0x200);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);
	m_palette->extmem().set(m_paletteram_ext, ENDIANNESS_LITTLE, 1);

	m_bg_tilemap->set_scroll_cols(32);
	m_bg_tilemap->set_scrolldy(15, 15);

	save_pointer(NAME(m_spriteram), 160);
	save_item(NAME(m_paletteram));
	save_item(NAME(m_paletteram_ext));
}

void ladyfrog_state::video_start()
{
	// weird, there are sprite tiles at 0x000 and 0x400, but they don't contain all the sprites!
	m_spritetilebase = 0x800;
	VIDEO_START_CALL_MEMBER(ladyfrog_common);
}

VIDEO_START_MEMBER(ladyfrog_state,toucheme)
{
	m_spritetilebase = 0x000;
	VIDEO_START_CALL_MEMBER(ladyfrog_common);
}


uint32_t ladyfrog_state::screen_update_ladyfrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
