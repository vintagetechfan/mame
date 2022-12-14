// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Alex W. Jackson
/*****************************************************************************

B-Wings  (c) 1984 Data East Corporation
Zaviga   (c) 1984 Data East Corporation

driver by Acho A. Tang
revised by Alex W. Jackson

Known issues:

- The main program is responsible for sprite clipping but occasional
  glitches can be seen at the top and bottom screen edges. (post rotate)

- B-Wings bosses sometimes flicker. (sync issue)

- The text layer has an unknown attribute. (needs verification)

- Zaviga's DIPs are incomplete. (manual missing)

- "RGB dip-switch" looks kludgy at best;

*****************************************************************************/
// Directives

#include "emu.h"
#include "bwing.h"

#include "cpu/m6502/deco16.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"


//****************************************************************************
// Interrupt Handlers

INTERRUPT_GEN_MEMBER(bwing_state::bwp3_interrupt)
{
	if (!m_bwp3_nmimask)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

//****************************************************************************
// Memory and I/O Handlers

void bwing_state::bwp3_u8F_w(uint8_t data)
{
	m_bwp3_u8F_d = data;  // prepares custom chip for various operations
}

void bwing_state::bwp3_nmimask_w(uint8_t data)
{
	m_bwp3_nmimask = data & 0x80;
}

void bwing_state::bwp3_nmiack_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


void bwing_state::bwp1_ctrl_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		// MSSTB
		case 0: m_subcpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE); break;

		// IRQACK
		case 1: m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); break;

		// FIRQACK
		case 2: m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE); break;

		// NMIACK
		case 3: m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); break;

		// SWAP(bank-swaps sprite RAM between 1800 & 1900; ignored bc. they're treated as a single chunk.)
		case 4: break;

		// SNDREQ
		case 5:
			if (data == 0x80) // protection trick to screw CPU1 & 3
				m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE); // SNMI
			else
			{
				m_soundlatch->write(data);
				m_audiocpu->set_input_line(DECO16_IRQ_LINE, HOLD_LINE); // SNDREQ
			}
		break;

		// BANKSEL(supposed to bank-switch CPU0 4000-7fff(may also 8000-bfff) 00=bank 0, 80=bank 1, unused)
		case 6: break;

		// hardwired to SWAP
		case 7: break;
	}
}


void bwing_state::bwp2_ctrl_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0: m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE); break; // SMSTB

		case 1: m_subcpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE); break;

		case 2: m_subcpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); break;

		case 3: m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); break;
	}
}

//****************************************************************************
// CPU Memory Maps

// Main CPU
void bwing_state::bwp1_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("sharedram");
	map(0x0800, 0x0fff).ram();
	map(0x1000, 0x13ff).ram().w(FUNC(bwing_state::videoram_w)).share("videoram");
	map(0x1400, 0x17ff).ram();
	map(0x1800, 0x19ff).ram().share("spriteram");
	map(0x1a00, 0x1aff).ram().w(FUNC(bwing_state::paletteram_w)).share("paletteram");
	map(0x1b00, 0x1b00).portr("DSW0");
	map(0x1b01, 0x1b01).portr("DSW1");
	map(0x1b02, 0x1b02).portr("IN0");
	map(0x1b03, 0x1b03).portr("IN1");
	map(0x1b04, 0x1b04).portr("IN2");
	map(0x1b00, 0x1b07).w(FUNC(bwing_state::scrollreg_w));
	map(0x1c00, 0x1c07).ram().w(FUNC(bwing_state::bwp1_ctrl_w));
	map(0x2000, 0x3fff).m(m_vrambank, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0xffff).rom(); // "B-Wings US" writes to 9631-9632(debug?)
}

// Banked video RAM
void bwing_state::bank_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().w(FUNC(bwing_state::fgscrollram_w)).share("fgscrollram");
	map(0x1000, 0x1fff).ram().w(FUNC(bwing_state::bgscrollram_w)).share("bgscrollram");
	map(0x2000, 0x7fff).ram().w(FUNC(bwing_state::gfxram_w)).share("gfxram");
}

// Sub CPU
void bwing_state::bwp2_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("sharedram");
	map(0x0800, 0x0fff).ram();
	map(0x1800, 0x1803).w(FUNC(bwing_state::bwp2_ctrl_w));
	map(0xa000, 0xffff).rom();
}


// Sound CPU
void bwing_state::bwp3_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x0200).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x1000, 0x1000).w(FUNC(bwing_state::bwp3_nmiack_w));
	map(0x2000, 0x2000).w("ay1", FUNC(ay8912_device::data_w));
	map(0x4000, 0x4000).w("ay1", FUNC(ay8912_device::address_w));
	map(0x6000, 0x6000).w("ay2", FUNC(ay8912_device::data_w));
	map(0x8000, 0x8000).w("ay2", FUNC(ay8912_device::address_w));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xd000, 0xd000).w(FUNC(bwing_state::bwp3_nmimask_w));
	map(0xe000, 0xffff).rom().region("audiocpu", 0);
}


void bwing_state::bwp3_io_map(address_map &map)
{
	map(0x00, 0x00).portr("VBLANK").w(FUNC(bwing_state::bwp3_u8F_w));
}

//****************************************************************************
// I/O Port Maps

INPUT_CHANGED_MEMBER(bwing_state::coin_inserted)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER(bwing_state::tilt_pressed)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( bwing )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Diagnostics" )       PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Invincibility" )     PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Infinite ) )     PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:2,3") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPSETTING(    0x02, "20000 80000" )
	PORT_DIPSETTING(    0x04, "20000 60000" )
	PORT_DIPSETTING(    0x06, "20000 40000" )
	PORT_DIPNAME( 0x08, 0x08, "Enemy Crafts" )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Enemy Missiles" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )            PORT_DIPLOCATION("SW2:6") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Country" )           PORT_DIPLOCATION("SW2:7") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x00, "Japan/US" )
	PORT_DIPSETTING(    0x40, "Japan Only" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:8") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bwing_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, bwing_state,coin_inserted, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_TILT ) PORT_CHANGED_MEMBER(DEVICE_SELF, bwing_state,tilt_pressed,0)

	PORT_START("VBLANK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("EXTRA") // a matter of taste
	PORT_DIPNAME( 0x07, 0x00, "RGB" )
	PORT_DIPSETTING(    0x00, "Default" )
	PORT_DIPSETTING(    0x01, "More Red" )
	PORT_DIPSETTING(    0x02, "More Green" )
	PORT_DIPSETTING(    0x03, "More Blue" )
	PORT_DIPSETTING(    0x04, "Max" )
INPUT_PORTS_END

//****************************************************************************
// Graphics Layouts

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static const gfx_layout ram_tilelayout =
{
	16, 16,
	RGN_FRAC(1,6), // two sets interleaved in the same RAM
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( gfx_bwing )
	GFXDECODE_ENTRY( "gfx1",      0, charlayout,     0x00, 1 ) // chars
	GFXDECODE_ENTRY( "gfx2",      0, spritelayout,   0x20, 2 ) // sprites
	GFXDECODE_RAM( "gfxram",      0, ram_tilelayout, 0x10, 2 ) // foreground tiles
	GFXDECODE_RAM( "gfxram", 0x1000, ram_tilelayout, 0x30, 2 ) // background tiles
GFXDECODE_END

//****************************************************************************
// Hardware Definitions

void bwing_state::machine_start()
{
	save_item(NAME(m_palatch));
	save_item(NAME(m_mapmask));
	save_item(NAME(m_bwp3_nmimask));
	save_item(NAME(m_bwp3_u8F_d));

	save_item(NAME(m_sreg));

	machine().save().register_postload(save_prepost_delegate(FUNC(bwing_state::bwing_postload), this));
}

void bwing_state::machine_reset()
{
	m_palatch = 0;
	m_mapmask = 0;

	m_bwp3_nmimask = 0;
	m_bwp3_u8F_d = 0;
}

void bwing_state::bwing_postload()
{
	m_gfxdecode->gfx(2)->mark_all_dirty();
	m_gfxdecode->gfx(3)->mark_all_dirty();
}


void bwing_state::bwing(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bwing_state::bwp1_map);

	MC6809E(config, m_subcpu, 2000000);
	m_subcpu->set_addrmap(AS_PROGRAM, &bwing_state::bwp2_map);

	DECO16(config, m_audiocpu, 2000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &bwing_state::bwp3_map);
	m_audiocpu->set_addrmap(AS_IO, &bwing_state::bwp3_io_map);
	m_audiocpu->set_periodic_int(FUNC(bwing_state::bwp3_interrupt), attotime::from_hz(1000));

	config.set_maximum_quantum(attotime::from_hz(18000));     // high enough?

	ADDRESS_MAP_BANK(config, "vrambank").set_map(&bwing_state::bank_map).set_options(ENDIANNESS_BIG, 8, 15, 0x2000);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(600));   // must be long enough for polling
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(bwing_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_bwing);
	PALETTE(config, m_palette).set_entries(64);


	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8912(config, "ay1", XTAL(24'000'000) / 2 / 8).add_route(ALL_OUTPUTS, "speaker", 0.5);

	AY8912(config, "ay2", XTAL(24'000'000) / 2 / 8).add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC08(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.1);
}

//****************************************************************************
// ROM Maps

ROM_START( bwings )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02-.10a",0x04000, 0x04000, CRC(6074a86b) SHA1(0ce1bd74450144fd3c6556787d6c5c5d4531d830) )  // different
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bw_bv-00-.4a", 0x0c000, 0x04000, CRC(1f83804c) SHA1(afd5eb0822db4fd982062945ca27e66ed9680645) )  // different

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06-.10d",0x0a000, 0x02000, CRC(eca00fcb) SHA1(c7affbb900e3940257f8cebc91266328a4a5dca3) )  // different
	ROM_LOAD( "bw_bv-05-.9d", 0x0c000, 0x02000, CRC(1e393300) SHA1(8d847256eb5dbccf5f524ec3aa836073d70b4edc) )  // different
	ROM_LOAD( "bw_bv-04-.7d", 0x0e000, 0x02000, CRC(6548c5bb) SHA1(d12cc8d0d5692c3de766f5c42c818dd8f685760a) )  // different

	ROM_REGION( 0x2000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x00000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )
ROM_END


ROM_START( bwingso )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02.10a", 0x04000, 0x04000, CRC(5ce74ab5) SHA1(b414f0bbe1c4c5b4c810bb4b9fba16aaf86520ff) )
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bw_bv-00.4a",  0x0c000, 0x04000, CRC(926bef63) SHA1(d4bd2e91fa0abc5e9472d4b684c076bdc3c29f5b) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06.10d", 0x0a000, 0x02000, CRC(91a21a4c) SHA1(042eed60119a861f6b3ccfbe68d880f182a8a8e1) )
	ROM_LOAD( "bw_bv-05.9d",  0x0c000, 0x02000, CRC(f283f39a) SHA1(9f7f4c39d49f4dfff73fe74cd457480e8a43a3c5) )
	ROM_LOAD( "bw_bv-04.7d",  0x0e000, 0x02000, CRC(29ae75b6) SHA1(48c94e996857f2ac995bcd25f0e67b9f7c17d807) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x00000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )
ROM_END


ROM_START( bwingsa )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02.10a", 0x04000, 0x04000, CRC(5ce74ab5) SHA1(b414f0bbe1c4c5b4c810bb4b9fba16aaf86520ff) )
	ROM_LOAD( "bv02.bin",     0x06000, 0x02000, CRC(2f84654e) SHA1(11b5343219b46d03f686ea348181c509121b9e3c) ) // only the lower 8k is different
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bv00.bin",     0x0c000, 0x04000, CRC(0bbc1222) SHA1(cfdf621a423a5ce4ba44a980e683d2abf044d6b9) ) // different

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06.10d", 0x0a000, 0x02000, CRC(91a21a4c) SHA1(042eed60119a861f6b3ccfbe68d880f182a8a8e1) )
	ROM_LOAD( "bw_bv-05.9d",  0x0c000, 0x02000, CRC(f283f39a) SHA1(9f7f4c39d49f4dfff73fe74cd457480e8a43a3c5) )
	ROM_LOAD( "bw_bv-04.7d",  0x0e000, 0x02000, CRC(29ae75b6) SHA1(48c94e996857f2ac995bcd25f0e67b9f7c17d807) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x00000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )
ROM_END

ROM_START( zaviga )
	// Top Board(DE-0169-0)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "as04.10a", 0x04000, 0x04000, CRC(b79f5da2) SHA1(b39748666d3f7fb1ac46d282cce09fe9531df6b1) )
	ROM_LOAD( "as02.7a", 0x08000, 0x04000, CRC(6addd16a) SHA1(940637c49bf9f38c77176ed2ae212048e9e7fd8f) )
	ROM_LOAD( "as00.4a", 0x0c000, 0x04000, CRC(c6ae4af0) SHA1(6f6f14385b20f9c9c312f816036c608fe8514b00) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "as08.10d", 0x0a000, 0x02000, CRC(b6187b3a) SHA1(d2d7c5b185f59986f45d8ec3ddf9b95364e57d96) )
	ROM_LOAD( "as07.9d", 0x0c000, 0x02000, CRC(dc1170e3) SHA1(c8e4d1564fd272d726d0e4ffd4f33f67f1b37cd7) )
	ROM_LOAD( "as06.7d", 0x0e000, 0x02000, CRC(ba888f84) SHA1(f94de8553cd4704d9b3349ded881a7cc62fa9b57) )

	ROM_REGION( 0x2000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "as05.13a", 0x00000, 0x02000, CRC(afe9b0ac) SHA1(3c653cd4fff7f4e00971249900b5a810b6e74dfe) )

	// Bottom Board(DE-0170-0)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "as14.5c", 0x00000, 0x01000, CRC(62132c1d) SHA1(6b101e220a440488da17de8446f4e2c8ec7c7de9) )

	// Middle Board(DE-0171-0)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "as11.1l", 0x00000, 0x04000, CRC(aa84af24) SHA1(af4ff085dc44b3d1493ec1c8b4a8d18dccecc872) )
	ROM_LOAD( "as12.1k", 0x04000, 0x04000, CRC(84af9041) SHA1(8fbd5995ca8e708cd7fb9cdfcdb174e12084f526) )
	ROM_LOAD( "as13.1h", 0x08000, 0x04000, CRC(15d0922b) SHA1(b8d715a9e610531472d516c19f6035adbce93c84) )
ROM_END


ROM_START( zavigaj )
	// Top Board(DE-0169-0)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "as04.10a", 0x04000, 0x04000, CRC(b79f5da2) SHA1(b39748666d3f7fb1ac46d282cce09fe9531df6b1) )
	ROM_LOAD( "as02.7a",  0x08000, 0x04000, CRC(6addd16a) SHA1(940637c49bf9f38c77176ed2ae212048e9e7fd8f) )
	ROM_LOAD( "as00.4a",  0x0c000, 0x04000, CRC(c6ae4af0) SHA1(6f6f14385b20f9c9c312f816036c608fe8514b00) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "as08.10d", 0x0a000, 0x02000, CRC(b6187b3a) SHA1(d2d7c5b185f59986f45d8ec3ddf9b95364e57d96) )
	ROM_LOAD( "as07.9d",  0x0c000, 0x02000, CRC(dc1170e3) SHA1(c8e4d1564fd272d726d0e4ffd4f33f67f1b37cd7) )
	ROM_LOAD( "as06-.7d", 0x0e000, 0x02000, CRC(b02d270c) SHA1(beea3d44d367543b5b5075c5892580e690691e75) )  // different

	ROM_REGION( 0x2000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "as05.13a", 0x00000, 0x02000, CRC(afe9b0ac) SHA1(3c653cd4fff7f4e00971249900b5a810b6e74dfe) )

	// Bottom Board(DE-0170-0)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "as14.5c", 0x00000, 0x01000, CRC(62132c1d) SHA1(6b101e220a440488da17de8446f4e2c8ec7c7de9) )

	// Middle Board(DE-0171-0)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "as11.1l", 0x00000, 0x04000, CRC(aa84af24) SHA1(af4ff085dc44b3d1493ec1c8b4a8d18dccecc872) )
	ROM_LOAD( "as12.1k", 0x04000, 0x04000, CRC(84af9041) SHA1(8fbd5995ca8e708cd7fb9cdfcdb174e12084f526) )
	ROM_LOAD( "as13.1h", 0x08000, 0x04000, CRC(15d0922b) SHA1(b8d715a9e610531472d516c19f6035adbce93c84) )
ROM_END

//****************************************************************************
// Initializations

void bwing_state::init_bwing()
{
	uint8_t *rom = memregion("audiocpu")->base();
	int j = memregion("audiocpu")->bytes();

	// swap nibbles
	for (int i = 0; i < j; i++)
		rom[i] = ((rom[i] & 0xf0) >> 4) | ((rom[i] & 0xf) << 4);

	// relocate vectors
	rom[j - (0x10 - 0x4)] = rom[j - (0x10 - 0xb)] = rom[j - (0x10 - 0x6)];
	rom[j - (0x10 - 0x5)] = rom[j - (0x10 - 0xa)] = rom[j - (0x10 - 0x7)];
}

//****************************************************************************
// Game Entries

GAME( 1984, bwings,       0, bwing, bwing, bwing_state, init_bwing, ROT90, "Data East Corporation", "B-Wings (Japan new Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bwingso, bwings, bwing, bwing, bwing_state, init_bwing, ROT90, "Data East Corporation", "B-Wings (Japan old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bwingsa, bwings, bwing, bwing, bwing_state, init_bwing, ROT90, "Data East Corporation", "B-Wings (Alt Ver.?)", MACHINE_SUPPORTS_SAVE )

GAME( 1984, zaviga,       0, bwing, bwing, bwing_state, init_bwing, ROT90, "Data East Corporation", "Zaviga", MACHINE_SUPPORTS_SAVE )
GAME( 1984, zavigaj, zaviga, bwing, bwing, bwing_state, init_bwing, ROT90, "Data East Corporation", "Zaviga (Japan)", MACHINE_SUPPORTS_SAVE )
