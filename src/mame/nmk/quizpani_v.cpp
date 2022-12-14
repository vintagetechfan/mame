// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/************************************************************************

    Quiz Panicuru Fantasy video hardware

************************************************************************/

#include "emu.h"
#include "quizpani.h"


TILEMAP_MAPPER_MEMBER(quizpani_state::bg_scan)
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

TILE_GET_INFO_MEMBER(quizpani_state::bg_tile_info)
{
	int code = m_bg_videoram[tile_index];

	tileinfo.set(1,
			(code & 0xfff) + (0x1000 * m_bgbank),
			code >> 12,
			0);
}

TILE_GET_INFO_MEMBER(quizpani_state::txt_tile_info)
{
	int code = m_txt_videoram[tile_index];

	tileinfo.set(0,
			(code & 0xfff) + (0x1000 * m_txtbank),
			code >> 12,
			0);
}

void quizpani_state::bg_videoram_w(offs_t offset, uint16_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void quizpani_state::txt_videoram_w(offs_t offset, uint16_t data)
{
	m_txt_videoram[offset] = data;
	m_txt_tilemap->mark_tile_dirty(offset);
}

void quizpani_state::tilesbank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if(m_txtbank != (data & 0x30)>>4)
		{
			m_txtbank = (data & 0x30)>>4;
			m_txt_tilemap->mark_all_dirty();
		}

		if(m_bgbank != (data & 3))
		{
			m_bgbank = data & 3;
			m_bg_tilemap->mark_all_dirty();
		}
	}
}

void quizpani_state::video_start()
{
	m_bg_tilemap  = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizpani_state::bg_tile_info)),  tilemap_mapper_delegate(*this, FUNC(quizpani_state::bg_scan)), 16,16, 256,32);
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizpani_state::txt_tile_info)), tilemap_mapper_delegate(*this, FUNC(quizpani_state::bg_scan)), 16,16, 256,32);
	m_txt_tilemap->set_transparent_pen(15);

	save_item(NAME(m_bgbank));
	save_item(NAME(m_txtbank));
	m_bg_tilemap->set_scrolldx(64,64);
	m_txt_tilemap->set_scrolldx(64,64);
}

uint32_t quizpani_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, m_scrollreg[0]);
	m_bg_tilemap->set_scrolly(0, m_scrollreg[1]);
	m_txt_tilemap->set_scrollx(0, m_scrollreg[2]);
	m_txt_tilemap->set_scrolly(0, m_scrollreg[3]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}
