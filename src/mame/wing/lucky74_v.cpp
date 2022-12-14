// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/****************************************************************************************

    LUCKY 74 - WING CO.,LTD.
    ------------------------

    *** VIDEO HARDWARE ***


    Written by Roberto Fresca.


    Games running on this hardware:

    * Lucky 74 (bootleg, set 1), 1988, Wing Co.,Ltd.
    * Lucky 74 (bootleg, set 2), 1988, Wing Co.,Ltd.


*****************************************************************************************


    Color Circuitry
    ---------------

    Here is a little diagram showing how the color PROMs are connected to a 74174
    and therefore to a resistor network that derives to the RGB connector.


                                  220
    (E6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (E7)24s10-12 _|                        |
                                  470      |
    (E6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (E7)24s10-11 _|                        |
                                   1K      |
    (E6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (E7)24s10-10 _|                        |
                                   2K      |
    (E6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Red
    (E7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (D6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (D7)24s10-12 _|                        |
                                  470      |
    (D6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (D7)24s10-11 _|                        |
                                   1K      |
    (D6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (D7)24s10-10 _|                        |
                                   2K      |
    (D6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Green
    (D7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _

                                  220
    (C6)24s10-12 -+- 74174-02 ---/\/\/\----+
    (C7)24s10-12 _|                        |
                                  470      |
    (C6)24s10-11 -+- 74174-10 ---/\/\/\----+
    (C7)24s10-11 _|                        |
                                   1K      |
    (C6)24s10-10 -+- 74174-12 ---/\/\/\----+
    (C7)24s10-10 _|                        |
                                   2K      |
    (C6)24s10-09 -+- 74174-15 ---/\/\/\----+---> Blue
    (C7)24s10-09 _|                        |
                                           /
                                        1K \
                                           /
                                           |
                                           _


    Regarding the above diagram, there are 2 different states controlled by both 06B53P.
    Each state arrange a different palette that will be assigned to each graphics bank.

    As we can see here, same pin of different PROMs are connected together in parallel.

    To reproduce the states, we need to create a double-sized palette and fill the first
    half with the values created through state 1, then fill the second half with proper
    values from state 2.


****************************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "lucky74.h"


void lucky74_state::fg_videoram_w(offs_t offset, uint8_t data)
{
	m_fg_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void lucky74_state::fg_colorram_w(offs_t offset, uint8_t data)
{
	m_fg_colorram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void lucky74_state::bg_videoram_w(offs_t offset, uint8_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lucky74_state::bg_colorram_w(offs_t offset, uint8_t data)
{
	m_bg_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void lucky74_state::palette(palette_device &palette) const
{
	// There are 2 states (see the technical notes).
	// We're constructing a double-sized palette with one half for each state.
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances_rgb[4] = { 2000, 1000, 470, 220 };

	double weights_r[4], weights_g[4], weights_b[4];
	compute_resistor_weights(0, 255,    -1.0,
			4,  resistances_rgb,    weights_r,  1000,   0,
			4,  resistances_rgb,    weights_g,  1000,   0,
			4,  resistances_rgb,    weights_b,  1000,   0);

	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component (this 1, PROM E6)
		bit0 = BIT(color_prom[0x000 + i], 0);
		bit1 = BIT(color_prom[0x000 + i], 1);
		bit2 = BIT(color_prom[0x000 + i], 2);
		bit3 = BIT(color_prom[0x000 + i], 3);
		int const r1 = combine_weights(weights_r, bit0, bit1, bit2, bit3);

		// red component (this 2, PROM E7)
		bit0 = BIT(color_prom[0x100 + i], 0);
		bit1 = BIT(color_prom[0x100 + i], 1);
		bit2 = BIT(color_prom[0x100 + i], 2);
		bit3 = BIT(color_prom[0x100 + i], 3);
		int const r2 = combine_weights(weights_r, bit0, bit1, bit2, bit3);

		// green component (this 1, PROM D6)
		bit0 = BIT(color_prom[0x200 + i], 0);
		bit1 = BIT(color_prom[0x200 + i], 1);
		bit2 = BIT(color_prom[0x200 + i], 2);
		bit3 = BIT(color_prom[0x200 + i], 3);
		int const g1 = combine_weights(weights_g, bit0, bit1, bit2, bit3);

		// green component (this 2, PROM D7)
		bit0 = BIT(color_prom[0x300 + i], 0);
		bit1 = BIT(color_prom[0x300 + i], 1);
		bit2 = BIT(color_prom[0x300 + i], 2);
		bit3 = BIT(color_prom[0x300 + i], 3);
		int const g2 = combine_weights(weights_g, bit0, bit1, bit2, bit3);

		// blue component (this 1, PROM C6)
		bit0 = BIT(color_prom[0x400 + i], 0);
		bit1 = BIT(color_prom[0x400 + i], 1);
		bit2 = BIT(color_prom[0x400 + i], 2);
		bit3 = BIT(color_prom[0x400 + i], 3);
		int const b1 = combine_weights(weights_b, bit0, bit1, bit2, bit3);

		// blue component (this 2, PROM C7)
		bit0 = BIT(color_prom[0x500 + i], 0);
		bit1 = BIT(color_prom[0x500 + i], 1);
		bit2 = BIT(color_prom[0x500 + i], 2);
		bit3 = BIT(color_prom[0x500 + i], 3);
		int const b2 = combine_weights(weights_b, bit0, bit1, bit2, bit3);


		// PROMs circuitry, 1st state
		palette.set_pen_color(i, rgb_t(r1, g1, b1));

		// PROMs circuitry, 2nd state
		palette.set_pen_color(i + 256, rgb_t(r2, g2, b2));
	}
}


TILE_GET_INFO_MEMBER(lucky74_state::get_fg_tile_info)
{
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 0;
	int attr = m_fg_colorram[tile_index];
	int code = m_fg_videoram[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	tileinfo.set(bank, code, color, 0);
}

TILE_GET_INFO_MEMBER(lucky74_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---- xxxx   tiles color.
    xxxx ----   tiles page offset.
*/
	int bank = 1;
	int attr = m_bg_colorram[tile_index];
	int code = m_bg_videoram[tile_index] + ((attr & 0xf0) << 4);
	int color = (attr & 0x0f);

	tileinfo.set(bank, code, color, 0);
}


void lucky74_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lucky74_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lucky74_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t lucky74_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
