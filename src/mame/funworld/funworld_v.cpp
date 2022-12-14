// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Peter Ferrie
/**********************************************************************************

  Fun World / Amatic / TAB / Impera.
  Series 7000 hardware.

  Video Hardware.
  Written by Roberto Fresca.

***********************************************************************************

  TAB/Impera/FunWorld color system circuitry
  ------------------------------------------

  74HC174 - Hex D-type flip-flops with reset; positive-edge trigger.
  N82S147 - 4K-bit TTL Bipolar PROM.
  74LS374 - 3-STATE Octal D-Type transparent latches and edge-triggered flip-flops.

                   N82S147         74LS374       RESNET        PULL-DOWN
   74HC174        .-------.       .-------.
  .-------.   (1)-|01   20|--VCC--|20   02|------[(1K)]---+              .-----.
  |       |   (1)-|02   06|-------|03   05|------[(470)]--+--+-----------| RED |
  |16: VCC|   (1)-|03   07|-------|04   06|------[(220)]--+  |           '-----'
  |       |   (1)-|04   08|-------|07     |                  '--[(100)]--GND
  |     02|-------|05   09|-------|08   09|------[(1K)]---+              .------.
  |     05|-------|16   11|-------|13   12|------[(470)]--+--+-----------| BLUE |
  |     07|-------|17   12|-------|14   15|------[(220)]--+  |           '------'
  |     10|-------|18   13|-------|17     |                  '--[(100)]--GND
  |     12|-------|19   14|-------|18   16|------[(470)]--+              .-------.
  |     13|---+---|15   10|---+---|10   19|------[(220)]--+--+-----------| GREEN |
  |15 08  |   |   |       |   |   |   01  |                  |           '-------'
  '-+--+--'   |   '-------'   |   '----+--'                  '--[(100)]--GND
    |  |      |               |        |
    |  '------+------GND------'        |
    '----------------------------------'

  (1): Connected either to:
       - A custom 40-pin GFX IC
       - 2x HYxxx devices (TAB blue PCB).
       - A little board with 4x 74LS138 or 74LS137 (Impera green PCB).

  NOTE: The 74LS374 could be replaced by a 74HCT373.

***********************************************************************************/


#include "emu.h"
#include "video/resnet.h"
#include "funworld.h"


void funworld_state::funworld_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances_rb[3] = { 1000, 470, 220 };
	static constexpr int resistances_g [2] = { 470, 220 };

	double weights_r[3], weights_b[3], weights_g[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rb, weights_r,  100,    0,
			3,  resistances_rb, weights_b,  100,    0,
			2,  resistances_g,  weights_g,  100,    0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);
		// blue component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const b = combine_weights(weights_b, bit0, bit1, bit2);
		// green component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const g = combine_weights(weights_g, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void funworld_state::funworld_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void funworld_state::funworld_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


/**** normal hardware limit ****
    - bits -
    7654 3210
    xxxx xx--   tiles color.
    xxx- x-xx   tiles color (title).
    xxxx -xxx   tiles color (background).
*/

TILE_GET_INFO_MEMBER(funworld_state::get_bg_tile_info)
{
	/*  - bits -
	    7654 3210
	    xxxx ----   tiles color.
	    ---- xxxx   unused.
	*/
	int offs = tile_index;
	int attr = m_videoram[offs] + (m_colorram[offs] << 8);
	int code = attr & 0xfff;
	int color = m_colorram[offs] >> 4;  // 4 bits for color.

	tileinfo.set(0, code, color, 0);
}


void funworld_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(funworld_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 29);
}

void magicrd2_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(magicrd2_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 112, 34);
}

void chinatow_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chinatow_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 31);
}

void multiwin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(multiwin_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 31);
}


uint32_t funworld_state::screen_update_funworld(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
