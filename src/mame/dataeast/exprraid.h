// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    Express Raider

*************************************************************************/

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class exprraid_state : public driver_device
{
public:
	exprraid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_main_ram(*this, "main_ram"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void exprraid(machine_config &config);
	void exprboot(machine_config &config);

	void init_exprraid();
	void init_wexpressb();
	void init_wexpressb2();
	void init_wexpressb3();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_deco16);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_main_ram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* protection */
	uint8_t           m_prot_value = 0U;

	/* video-related */
	tilemap_t       *m_bg_tilemap = nullptr;
	tilemap_t       *m_fg_tilemap = nullptr;
	int             m_bg_index[4]{};

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void exprraid_int_clear_w(uint8_t data);
	uint8_t exprraid_prot_status_r();
	uint8_t exprraid_prot_data_r();
	void exprraid_prot_data_w(uint8_t data);
	uint8_t vblank_r();
	void exprraid_videoram_w(offs_t offset, uint8_t data);
	void exprraid_colorram_w(offs_t offset, uint8_t data);
	void exprraid_flipscreen_w(uint8_t data);
	void exprraid_bgselect_w(offs_t offset, uint8_t data);
	void exprraid_scrollx_w(offs_t offset, uint8_t data);
	void exprraid_scrolly_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update_exprraid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void exprraid_gfx_expand();
	void master_io_map(address_map &map);
	void master_map(address_map &map);
	void slave_map(address_map &map);
};
