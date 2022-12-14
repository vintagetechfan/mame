// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tao Taido Video Hardware */

/*

its similar to other video system games

zooming might be wrong (only used on title logo?)

*/

#include "emu.h"
#include "taotaido.h"
#include "screen.h"

/* sprite tile codes 0x4000 - 0x7fff get remapped according to the content of these registers */
void taotaido_state::spritebank_w(offs_t offset, uint8_t data)
{
	m_spritebank[offset] = data;
}

/* sprites are like the other video system / psikyo games, we can merge this with aerofgt and plenty of other
   things eventually */


/* the tilemap */

void taotaido_state::tileregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 0: // would normally be x scroll?
		case 1: // would normally be y scroll?
		case 2:
		case 3:
			logerror ("unhanded tilemap register write offset %02x data %04x \n",offset,data);
			break;

		/* tile banks */
		case 4:
		case 5:
		case 6:
		case 7:
			if(ACCESSING_BITS_8_15)
				m_bgbank[(offset - 4) << 1] = data >> 8;
			if(ACCESSING_BITS_0_7)
				m_bgbank[((offset - 4) << 1) + 1] = data & 0xff;
			m_bg_tilemap->mark_all_dirty();
			break;
	}
}

void taotaido_state::bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(taotaido_state::bg_tile_info)
{
	int code = m_bgram[tile_index] & 0x01ff;
	int bank =(m_bgram[tile_index] & 0x0e00) >> 9;
	int col  =(m_bgram[tile_index] & 0xf000) >> 12;

	code |= m_bgbank[bank] << 9;

	tileinfo.set(1, code, col, 0);
}

TILEMAP_MAPPER_MEMBER(taotaido_state::tilemap_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return (col & 0x3f) | ((row & 0x3f) << 6) | ((col & 0x40) << 6);
}


uint32_t taotaido_state::tile_callback( uint32_t code )
{
	code = m_spriteram2_older[code & 0x7fff];

	if (code > 0x3fff)
	{
		int block = (code & 0x3800) >> 11;
		code &= 0x07ff;
		code |= m_spritebank[block] << 11;
	}

	return code;
}


void taotaido_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(taotaido_state::bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(taotaido_state::tilemap_scan_rows)), 16,16, 128,64);

	m_spriteram_old = std::make_unique<uint16_t[]>(0x2000/2);
	m_spriteram_older = std::make_unique<uint16_t[]>(0x2000/2);

	m_spriteram2_old = std::make_unique<uint16_t[]>(0x10000/2);
	m_spriteram2_older = std::make_unique<uint16_t[]>(0x10000/2);

	save_item(NAME(m_bgbank));
	save_pointer(NAME(m_spriteram_old), 0x2000/2);
	save_pointer(NAME(m_spriteram_older), 0x2000/2);
	save_pointer(NAME(m_spriteram2_old), 0x10000/2);
	save_pointer(NAME(m_spriteram2_older), 0x10000/2);
}


uint32_t taotaido_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  m_bg_tilemap->set_scrollx(0,(m_scrollram[0x380/2]>>4)); // the values put here end up being wrong every other frame
//  m_bg_tilemap->set_scrolly(0,(m_scrollram[0x382/2]>>4)); // the values put here end up being wrong every other frame

	/* not amazingly efficient however it should be functional for row select and linescroll */
	rectangle clip = cliprect;

	for (int line = cliprect.top(); line <= cliprect.bottom(); line++)
	{
		clip.min_y = clip.max_y = line;

		m_bg_tilemap->set_scrollx(0, ((m_scrollram[(0x00 + 4 * line) / 2]) >> 4) + 30);
		m_bg_tilemap->set_scrolly(0, ((m_scrollram[(0x02 + 4 * line) / 2]) >> 4) - line);

		m_bg_tilemap->draw(screen, bitmap, clip, 0,0);
	}

	m_spr->draw_sprites(m_spriteram_older.get(), m_spriteram.bytes(), screen, bitmap, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(taotaido_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		/* sprites need to be delayed by 2 frames? */

		memcpy(m_spriteram2_older.get(),m_spriteram2_old.get(),0x10000);
		memcpy(m_spriteram2_old.get(),m_spriteram2,0x10000);

		memcpy(m_spriteram_older.get(),m_spriteram_old.get(),0x2000);
		memcpy(m_spriteram_old.get(),m_spriteram,0x2000);
	}
}
