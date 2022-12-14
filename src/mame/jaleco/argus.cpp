// license:BSD-3-Clause
// copyright-holders:Yochizo
/***************************************************************************

Argus (Early NMK driver 1986-1987)
-------------------------------------
driver by Yochizo


Special thanks to :
=====================
 - Gerardo Oporto Jorrin for dipswitch informations.
 - Suzuki2go for screenshots of Argus and Valtric.
 - Jarek Parchanski for Psychic5 driver.


Supported games :
==================
 Argus      (C) 1986 NMK / Jaleco
 Valtric    (C) 1986 NMK / Jaleco
 Butasan    (C) 1987 NMK / Jaleco


System specs :
===============
 Argus
 ---------------------------------------------------------------
   CPU    : Z80 (4MHz) + Z80 (4MHz, Sound)
   Sound  : YM2203 x 1
   Layers : BG0, BG1, Sprite, Text [BG0 is controlled by VROMs]
   Colors : 832 colors
             Sprite : 128 colors
             BG0    : 256 colors
             BG1    : 256 colors
             Text   : 256 colors
   Others : Brightness controller  (Emulated)
            Half transparent color (Not emulated)
   Reference : https://youtu.be/aSgRfAgd7yU

 Valtric
 ---------------------------------------------------------------
   CPU    : Z80 (5MHz) + Z80 (5MHz, Sound)
   Sound  : YM2203 x 2
   Layers : BG1, Sprite, Text
   Colors : 768 colors
             Sprite : 256 colors
             BG1    : 256 colors
             Text   : 256 colors
   Others : Brightness controller  (Emulated)
            Half transparent color (Not emulated)
            Mosaic effect          (Emulated)

 Butasan
 ---------------------------------------------------------------
   CPU    : Z80 (5MHz) + Z80 (5MHz, Sound)
   Sound  : YM2203 x 2
   Layers : BG0, BG1, Sprite, Text [BG0 and BG1 is not shown simultaneously]
   Colors : 672 colors
             Sprite : 16x4 + 8x8 = 128 colors
             BG0    : 256 colors
             BG1    : 32 colors
             Text   : 256 colors
   Others : 2 VRAM pages           (Emulated)
            Various sprite sizes   (Emulated)


PCB Edge Connector Pinout :
============================
 Argus
 ----------------------------------
         PARTS         SOLDER
  --------------------------------
           GND   1 A   GND
           GND   2 B   GND
            5V   3 C   5V
            5V   4 D   5V
           12V   5 E   12V
     Video Red   6 F   Video Blue
   Video Green   7 H   Video Sync
     Speaker +   8 J   Speaker -
        Coin 2   9 K   Coin 1     (*) see below
           GND  10 L   GND
      1P Start  11 M   2P Start
         1P Up  12 N   2P Up
       1P Down  13 P   2P Down
      1P Right  14 R   2P Right
       1P Left  15 S   2P Left
     1P Shot 1  16 T   2P Shot 1
     1P Shot 2  17 U   2P Shot 2
           GND  18 V   GND
 (*) You may doubt that 9 and K are swapped. But they are not.

 Valtric and Butasan
 ----------------------------------
   JAMMA


Note :
=======
 - To enter test mode, press Coin 2 key at start in Argus,
                       press Coin 1 key at start in Valtric,
                   and press Service key at start in Butasan.
 - DIP locations verified for:
    butasan
    argus
    valtric

- Information about the internal ROM tests (see also MT03219):
    * argus: Checksum routine at $7fc9 (for banks at $7fc0). Checksum is a
      simple sum of the contents. Our dump gives a result of 0x95 while the
      game expects 0x9b, therefore it displays a checksum error. Checksums for
      the banked ROMs match.
    * valtric: Checksum routine at $987c (for banks at $f000). Checksum is a
      XOR over the contents. The expected checksums are stored in ROM vt_06.bin
      starting at $d000 (main ROM first, then banks). For our dump, the
      expected checksums are all 0x00, but the calculated checksums differ,
      therefore displays a checksum error for all ROMs. This has been validated
      on real hardware to also fail there.
    * butasan: Checksum routine is at $e0a8 (for banks at $ec74). Checksum is a
      simple sum over the contents. The test seems to be broken (or hacked) as
      it will only fail when the checksum is exactly 0x00.


Known issues :
===============
 - Half transparent color (50% alpha blending) is not emulated.
 - Sprite priority switch of Butasan is shown in test mode. What will be
   happened when set it ? JFF is not implemented this mystery switch too.
 - Data proms of Butasan does exist. But I don't know what is used for.
 - Though clock speed of Argus is actually 4 MHz, major sprite problems
   are broken out in the middle of slowdown. So, it is set 5 MHz now.
 - Sprite locations of Argus delay around 1 or 2 frames when horizontal
   scroll occurs.

****************************************************************************/


#include "emu.h"
#include "argus.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "speaker.h"


void argus_common_state::machine_start()
{
	membank("mainbank")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x4000);
}

/***************************************************************************

  Interrupt(s)

***************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(argus_common_state::scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* Z80 - RST 10h */

	if(scanline == 16) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* Z80 - RST 08h */
}

TIMER_DEVICE_CALLBACK_MEMBER(butasan_state::scanline)
{
	int scanline = param;

	if(scanline == 248) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xd7); /* Z80 - RST 10h */

	if(scanline == 8) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); /* Z80 - RST 08h */
}


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

void argus_common_state::bankselect_w(u8 data)
{
	membank("mainbank")->set_entry(data & 7);   /* Select 8 banks of 16k */
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

void argus_state::argus_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc200, 0xc200).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc201, 0xc201).w(FUNC(argus_state::flipscreen_w));
	map(0xc202, 0xc202).w(FUNC(argus_state::bankselect_w));
	map(0xc300, 0xc301).ram().share("bg0_scrollx");
	map(0xc302, 0xc303).ram().share("bg0_scrolly");
	map(0xc308, 0xc309).ram().share("bg1_scrollx");
	map(0xc30a, 0xc30b).ram().share("bg1_scrolly");
	map(0xc30c, 0xc30c).w(FUNC(argus_state::bg_status_w));
	map(0xc400, 0xcfff).ram().w(FUNC(argus_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xd7ff).ram().w(FUNC(argus_state::txram_w)).share("txram");
	map(0xd800, 0xdfff).ram().w(FUNC(argus_state::bg1ram_w)).share("bg1ram");
	map(0xe000, 0xf1ff).ram();
	map(0xf200, 0xf7ff).ram().share("spriteram");
	map(0xf800, 0xffff).ram();
}

void valtric_state::valtric_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc200, 0xc200).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc201, 0xc201).w(FUNC(valtric_state::flipscreen_w));
	map(0xc202, 0xc202).w(FUNC(valtric_state::bankselect_w));
	map(0xc300, 0xc300).w(FUNC(valtric_state::unknown_w));
	map(0xc308, 0xc309).ram().share("bg1_scrollx");
	map(0xc30a, 0xc30b).ram().share("bg1_scrolly");
	map(0xc30c, 0xc30c).w(FUNC(valtric_state::bg_status_w));
	map(0xc30d, 0xc30d).w(FUNC(valtric_state::mosaic_w));
	map(0xc400, 0xcfff).ram().w(FUNC(valtric_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xd7ff).ram().w(FUNC(valtric_state::txram_w)).share("txram");
	map(0xd800, 0xdfff).ram().w(FUNC(valtric_state::bg1ram_w)).share("bg1ram");
	map(0xe000, 0xf1ff).ram();
	map(0xf200, 0xf7ff).ram().share("spriteram");
	map(0xf800, 0xffff).ram();
}

void butasan_state::butasan_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("mainbank");
	map(0xc000, 0xc000).portr("SYSTEM");
	map(0xc001, 0xc001).portr("P1");
	map(0xc002, 0xc002).portr("P2");
	map(0xc003, 0xc003).portr("DSW1");
	map(0xc004, 0xc004).portr("DSW2");
	map(0xc100, 0xc100).w(FUNC(butasan_state::unknown_w));
	map(0xc200, 0xc200).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xc201, 0xc201).w(FUNC(butasan_state::flipscreen_w));
	map(0xc202, 0xc202).w(FUNC(butasan_state::bankselect_w));
	map(0xc203, 0xc203).w(FUNC(butasan_state::pageselect_w));
	map(0xc300, 0xc301).ram().share("bg0_scrollx");
	map(0xc302, 0xc303).ram().share("bg0_scrolly");
	map(0xc304, 0xc304).w(FUNC(butasan_state::bg0_status_w));
	map(0xc308, 0xc309).ram().share("bg1_scrollx");
	map(0xc30a, 0xc30b).ram().share("bg1_scrolly");
	map(0xc30c, 0xc30c).w(FUNC(butasan_state::bg1_status_w));
	map(0xc400, 0xc7ff).ram().w(FUNC(butasan_state::bg1ram_w)).share("butasan_bg1ram");
	map(0xc800, 0xcfff).ram().w(FUNC(butasan_state::paletteram_w)).share("paletteram");
	map(0xd000, 0xdfff).rw(FUNC(butasan_state::pagedram_r), FUNC(butasan_state::pagedram_w));
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xf67f).ram().share("spriteram");
	map(0xf680, 0xffff).ram();
}

void argus_common_state::sound_map_a(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void argus_common_state::sound_map_b(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xe000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void argus_common_state::sound_portmap_1(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x81).noprw(); // second ym2203 is not implemented on argus but still writes here
}

void argus_common_state::sound_portmap_2(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x80, 0x81).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( argus )
	PORT_START("SYSTEM")    /* System control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")     /* Player 1 control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    /* Player 2 control */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* DIPSW default setting: all OFF */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium_Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:8" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:7" )            /* Listed as "Unused" */
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:6,5,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0C, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1C, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( valtric )
	PORT_INCLUDE( argus )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* assigned JAMMA "Service", but not used */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	/* DIPSW default setting: all OFF */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,6")
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium_Difficult ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Invulnerability (Cheat)" )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( butasan )
	PORT_INCLUDE( valtric )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW ) /* work as both "Service Credit SW" and "Test Mode SW" */

	/* DIPSW default setting: all OFF */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Invulnerability (Cheat)" )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium_Difficult ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:2" )            /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:1" )            /* Listed as "Unused" */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static GFXDECODE_START( gfx_argus )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_col_2x2_group_packed_msb,  0*16,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_col_2x2_group_packed_msb,  8*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_col_2x2_group_packed_msb, 24*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x4_packed_msb,               40*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_valtric )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_col_2x2_group_packed_msb,  0*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_col_2x2_group_packed_msb, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_packed_msb,               32*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_butasan )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_col_2x2_group_packed_msb,  0*16, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_col_2x2_group_packed_msb, 16*16, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_col_2x2_group_packed_msb, 12*16, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x4_packed_msb,               32*16, 16 )
GFXDECODE_END


void argus_state::argus(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5000000);           /* 4 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &argus_state::argus_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(argus_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 5000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &argus_state::sound_map_a);
	m_audiocpu->set_addrmap(AS_IO, &argus_state::sound_portmap_1);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(54);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* This value is referred to psychic5 driver */
	m_screen->set_size(32*16, 32*16);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(argus_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_argus);
	PALETTE(config, m_palette).set_entries(896);

	JALECO_BLEND(config, m_blend, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 6000000 / 4));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.50);
}

void valtric_state::valtric(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5000000);    /* 5 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &valtric_state::valtric_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(valtric_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 5000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &valtric_state::sound_map_a);
	m_audiocpu->set_addrmap(AS_IO, &valtric_state::sound_portmap_2);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(54);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* This value is referred to psychic5 driver */
	m_screen->set_size(32*16, 32*16);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(valtric_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_valtric);
	PALETTE(config, m_palette).set_entries(768);

	JALECO_BLEND(config, m_blend, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 6000000 / 4));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.15);
	ym1.add_route(1, "mono", 0.15);
	ym1.add_route(2, "mono", 0.15);
	ym1.add_route(3, "mono", 0.50);

	ym2203_device &ym2(YM2203(config, "ym2", 6000000 / 4));
	ym2.add_route(0, "mono", 0.15);
	ym2.add_route(1, "mono", 0.15);
	ym2.add_route(2, "mono", 0.15);
	ym2.add_route(3, "mono", 0.50);
}

void butasan_state::butasan(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5000000);    /* 5 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &butasan_state::butasan_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(butasan_state::scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, 5000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &butasan_state::sound_map_b);
	m_audiocpu->set_addrmap(AS_IO, &butasan_state::sound_portmap_2);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(54);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));  /* This value is taken from psychic5 driver */
	m_screen->set_size(32*16, 32*16);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_screen_update(FUNC(butasan_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_butasan);
	PALETTE(config, m_palette).set_entries(768);

	JALECO_BLEND(config, m_blend, 0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym2203_device &ym1(YM2203(config, "ym1", 6000000 / 4));
	ym1.irq_handler().set_inputline(m_audiocpu, 0);
	ym1.add_route(0, "mono", 0.30);
	ym1.add_route(1, "mono", 0.30);
	ym1.add_route(2, "mono", 0.30);
	ym1.add_route(3, "mono", 1.0);

	ym2203_device &ym2(YM2203(config, "ym2", 6000000 / 4));
	ym2.add_route(0, "mono", 0.30);
	ym2.add_route(1, "mono", 0.30);
	ym2.add_route(2, "mono", 0.30);
	ym2.add_route(3, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( argus )
	ROM_REGION( 0x28000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "ag_02.bin",    0x00000, 0x08000, CRC(278a3f3d) SHA1(c5ac5a004ebf0194c33f71dab4020fa636cefbc2) )
	ROM_LOAD( "ag_03.bin",    0x10000, 0x08000, CRC(3a7f3bfa) SHA1(b11e134c084fc3c982dfe31836c1cf3fc0d481fd) )
	ROM_LOAD( "ag_04.bin",    0x18000, 0x08000, CRC(76adc9f6) SHA1(e223a8b2371c51f121958ee3687c777f597334c9) )
	ROM_LOAD( "ag_05.bin",    0x20000, 0x08000, CRC(f76692d6) SHA1(1dc353a042cdda909eb9f1b1ca749a3b3eaa01e4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "ag_01.bin",    0x00000, 0x04000, CRC(769e3f57) SHA1(209160a96486ab0b90967c015143ec28fba2e2a4) )

	ROM_REGION( 0x20000, "gfx1", 0 )    /* Sprite */
	ROM_LOAD( "ag_09.bin",    0x00000, 0x08000, CRC(6dbc1c58) SHA1(ef7b6901b702dd347b3a3f162162138175efe578) )
	ROM_LOAD( "ag_08.bin",    0x08000, 0x08000, CRC(ce6e987e) SHA1(9de257d8061ec917f4d443ff509fd457f995d73b) )
	ROM_LOAD( "ag_07.bin",    0x10000, 0x08000, CRC(bbb9638d) SHA1(61dec71d4d976bef3af26d0dc9c0355fd1098ffb) )
	ROM_LOAD( "ag_06.bin",    0x18000, 0x08000, CRC(655b48f8) SHA1(4fce1dffe091b97e7055955743434e49e97b4b79) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* BG0 */
	ROM_LOAD( "ag_13.bin",    0x00000, 0x08000, CRC(20274268) SHA1(9b7767d14bd169dabe6add0623d353bf4b59779b) )
	ROM_LOAD( "ag_14.bin",    0x08000, 0x08000, CRC(ceb8860b) SHA1(90e094686d9d18e49e4848d18d1e31ac95f13937) )
	ROM_LOAD( "ag_11.bin",    0x10000, 0x08000, CRC(99ce8556) SHA1(39caababd6e20ecb0375b85fb6490ee0b04f0949) )
	ROM_LOAD( "ag_12.bin",    0x18000, 0x08000, CRC(e0e5377c) SHA1(b5981d832127d0b28b6a7bb0437716593e0ed71a) )

	ROM_REGION( 0x08000, "gfx3", 0 )    /* BG1 */
	ROM_LOAD( "ag_17.bin",    0x00000, 0x08000, CRC(0f12d09b) SHA1(718db4ff016526dddacdf6f0088f247ee97c6543) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* Text */
	ROM_LOAD( "ag_10.bin",    0x00000, 0x04000, CRC(2de696c4) SHA1(1ad0f1cde127a1618c2ea74a53e522963a79e5ce) )

	ROM_REGION( 0x08000, "vrom1", 0 )                   /* Map */
	ROM_LOAD( "ag_15.bin",    0x00000, 0x08000, CRC(99834c1b) SHA1(330f271771b158493b28bb178c8cda98efd1d90c) )

	ROM_REGION( 0x08000, "vrom2", 0 )                   /* Pattern */
	ROM_LOAD( "ag_16.bin",    0x00000, 0x08000, CRC(39a51714) SHA1(ad89a630f1352eb4d8beeeebf909d5e2b5d7cc12) )
ROM_END

ROM_START( valtric )
	ROM_REGION( 0x30000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "vt_04.bin",    0x00000, 0x08000, CRC(709c705f) SHA1(b82e2209a0371dcbc2708c485b02985cea04353f) ) // 512
	ROM_LOAD( "vt_06.bin",    0x10000, 0x10000, CRC(c9cbb4e4) SHA1(3c84cda778263a9bb2031e29f6f29f29878d2070) ) // 512
	ROM_LOAD( "vt_05.bin",    0x20000, 0x10000, CRC(7ab2684b) SHA1(9bca7e2fd3b5f4043de37cd439d5235957e5012f) ) // 512

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "vt_01.bin",    0x00000, 0x08000, CRC(4616484f) SHA1(24d060218cc1542ebfc2100ecd6489a0e17b36ee) ) // 512

	ROM_REGION( 0x20000, "gfx1", 0 )    /* Sprite */
	ROM_LOAD( "vt_02.bin",    0x00000, 0x10000, CRC(66401977) SHA1(91c527d0bcea54d723068715a12cb3c976d04294) ) // 512
	ROM_LOAD( "vt_03.bin",    0x10000, 0x10000, CRC(9203bbce) SHA1(f40cee48f62a87a0b5d18e271faa5b8dd36ae5f1) ) // 512

	ROM_REGION( 0x40000, "gfx2", 0 )    /* BG */
	ROM_LOAD( "vt_08.bin",    0x00000, 0x10000, CRC(661dd338) SHA1(cc643a14607c10e4a1710766f77422cd89a6bf94) ) // 27512
	ROM_LOAD( "vt_09.bin",    0x10000, 0x10000, CRC(085a35b1) SHA1(ff589e67b6b5a6e661f29294a32a3840f45a9304) ) // 27512
	ROM_LOAD( "vt_10.bin",    0x20000, 0x10000, CRC(09c47323) SHA1(fcfbd5054e63fae00b6a3959228964ac8f3cbf37) ) // 27512
	ROM_LOAD( "vt_11.bin",    0x30000, 0x10000, CRC(4cf800b5) SHA1(7241e284b15475d8a6d533e4caadd0acbf058231) ) // 27512

	ROM_REGION( 0x08000, "gfx3", 0 )    /* Text */
	ROM_LOAD( "vt_07.bin",    0x00000, 0x04000, CRC(4b9771ae) SHA1(6079f2d3abf836f04b83983485fdaba2fb357811) ) // 27128

	ROM_REGION (0x100, "proms", 0) // currently not used by the emulation, without it you just get diagonal glitches on a real board.
	ROM_LOAD( "82s129.1s", 0x000, 0x100, CRC(bc42f9b5) SHA1(962c812d5d1f0e539238a4ec49866ef3558db629) ) // located on the top board
ROM_END

ROM_START( butasan ) /* English "subtitle" of Butasan for Japanese region.  Original Jaleco PCB */
	ROM_REGION( 0x30000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "4.t2",    0x00000, 0x08000, CRC(1ba1d8e4) SHA1(ab141a1fbaab9f3ef6788b131833effb32c42930) ) /* M5L27256K-2 (32kb) - 4.4.bin */
	ROM_LOAD( "3.s2",    0x10000, 0x10000, CRC(a6b3ccc2) SHA1(fcc9db1cd68fd9477d86e63e6906d194d5ee477a) )
	ROM_LOAD( "2.p2",    0x20000, 0x10000, CRC(96517fa9) SHA1(03ee1f118f109c85b046098c457a90b40e163f3c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "1.b2",    0x00000, 0x10000, CRC(c9d23e2d) SHA1(cee289d5bf7626fc35808a09f9f1f4628fa16974) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* Sprite */
	ROM_LOAD( "16.k16",  0x00000, 0x10000, CRC(e0ce51b6) SHA1(458c7c422a7b6ce42f397a8868610f6386fd815c) )
	ROM_LOAD( "15.k15",  0x10000, 0x10000, CRC(2105f6e1) SHA1(54c13073f0dc8b5d3fb5578aa5958a5dd01396a6) )
	ROM_LOAD( "14.k14",  0x20000, 0x10000, CRC(8ec891c1) SHA1(e16f18a0eed300752af8f07fd3cef5cd825a2a05) )
	ROM_LOAD( "13.k13",  0x30000, 0x10000, CRC(5023e74d) SHA1(edf43e6c89f0e537cebf1c21a671dba4cd7d91ea) )
	ROM_LOAD( "12.k12",  0x40000, 0x10000, CRC(44f59905) SHA1(bf364f7f907fee551e9228db7c27c106bcfecf6c) )
	ROM_LOAD( "11.k11",  0x50000, 0x10000, CRC(b8929f1d) SHA1(18a72f30284bed0c6723105f87eb10d64d3f461d) )
	ROM_LOAD( "10.k10",  0x60000, 0x10000, CRC(fd4d3baf) SHA1(fa8e3970a8aac83efcb669fe5d4683adade9aa4f) )
	ROM_LOAD( "9.k9",    0x70000, 0x10000, CRC(7da4c0fd) SHA1(fb2b148ccfee530313da886eddf7711ee83b4aeb) )

	ROM_REGION( 0x20000, "gfx2", 0 )    /* BG0 */
	ROM_LOAD( "5.l7",    0x00000, 0x10000, CRC(b8e026b0) SHA1(eb6ff9042b21b7190000c571ccba7d81f11ce9f1) )
	ROM_LOAD( "6.n7",    0x10000, 0x10000, CRC(8bbacb81) SHA1(015be76e44ed2389eff912d8f61a757667d7670b) )

	ROM_REGION( 0x10000, "gfx3", 0 )    /* BG1 */
	ROM_LOAD( "7.a8",    0x00000, 0x10000, CRC(3a48d531) SHA1(0ff6256bb7ea909d95b2bfb994ebc5432ea6d055) )

	ROM_REGION( 0x08000, "gfx4", 0 )    /* Text */
	ROM_LOAD( "8.a7",    0x00000, 0x08000, CRC(85153252) SHA1(20af223f9dc2e29e506e257c36e96d10dc150467) )

	ROM_REGION( 0x00200, "proms", 0 )                   /* Data proms ??? */
	ROM_LOAD( "buta-01.prm",  0x00000, 0x00100, CRC(45baedd0) SHA1(afdafb67d55007e6fb99518657e27ce61d2cb7e6) )
	ROM_LOAD( "buta-02.prm",  0x00100, 0x00100, CRC(0dcb18fc) SHA1(0b097b873c9484981f87a5e3d1af767f901ae05f) )
ROM_END

ROM_START( butasanj )
	ROM_REGION( 0x30000, "maincpu", 0 )                     /* Main CPU */
	ROM_LOAD( "buta-04.bin",  0x00000, 0x08000, CRC(47ff4ca9) SHA1(d89a41f6987c91d20b010f0cbda332cf54b21f8c) )
	ROM_LOAD( "buta-03.bin",  0x10000, 0x10000, CRC(69fd88c7) SHA1(fd827d7926a2de5ffe2982b3a59ea43de00ee46b) )
	ROM_LOAD( "buta-02.bin",  0x20000, 0x10000, CRC(519dc412) SHA1(48bbb01b217bd19c48ef7ab12c60805aaa02527c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                    /* Sound CPU */
	ROM_LOAD( "1.b2",    0x00000, 0x10000, CRC(c9d23e2d) SHA1(cee289d5bf7626fc35808a09f9f1f4628fa16974) ) // buta-01.bin

	ROM_REGION( 0x80000, "gfx1", 0 )    /* Sprite */
	ROM_LOAD( "16.k16",  0x00000, 0x10000, CRC(e0ce51b6) SHA1(458c7c422a7b6ce42f397a8868610f6386fd815c) ) // buta-16.bin
	ROM_LOAD( "buta-15.bin",  0x10000, 0x10000, CRC(3ed19daa) SHA1(b8090c3baa2b31681bed15c682a97c024e229df7) )
	ROM_LOAD( "14.k14",  0x20000, 0x10000, CRC(8ec891c1) SHA1(e16f18a0eed300752af8f07fd3cef5cd825a2a05) ) // buta-14.bin
	ROM_LOAD( "13.k13",  0x30000, 0x10000, CRC(5023e74d) SHA1(edf43e6c89f0e537cebf1c21a671dba4cd7d91ea) ) // buta-13.bin
	ROM_LOAD( "12.k12",  0x40000, 0x10000, CRC(44f59905) SHA1(bf364f7f907fee551e9228db7c27c106bcfecf6c) ) // buta-12.bin
	ROM_LOAD( "11.k11",  0x50000, 0x10000, CRC(b8929f1d) SHA1(18a72f30284bed0c6723105f87eb10d64d3f461d) ) // buta-11.bin
	ROM_LOAD( "10.k10",  0x60000, 0x10000, CRC(fd4d3baf) SHA1(fa8e3970a8aac83efcb669fe5d4683adade9aa4f) ) // buta-10.bin
	ROM_LOAD( "9.k9",    0x70000, 0x10000, CRC(7da4c0fd) SHA1(fb2b148ccfee530313da886eddf7711ee83b4aeb) ) // buta-09.bin

	ROM_REGION( 0x20000, "gfx2", 0 )    /* BG0 */
	ROM_LOAD( "5.l7",    0x00000, 0x10000, CRC(b8e026b0) SHA1(eb6ff9042b21b7190000c571ccba7d81f11ce9f1) ) // buta-05.bin
	ROM_LOAD( "6.n7",    0x10000, 0x10000, CRC(8bbacb81) SHA1(015be76e44ed2389eff912d8f61a757667d7670b) ) // buta-06.bin

	ROM_REGION( 0x10000, "gfx3", 0 )    /* BG1 */
	ROM_LOAD( "7.a8",    0x00000, 0x10000, CRC(3a48d531) SHA1(0ff6256bb7ea909d95b2bfb994ebc5432ea6d055) ) // buta-07.bin

	ROM_REGION( 0x08000, "gfx4", 0 )    /* Text */
	ROM_LOAD( "buta-08.bin",    0x00000, 0x08000, CRC(5d45ce9c) SHA1(113c3e7ce20634ee4bb740705485572583298694) )

	ROM_REGION( 0x00200, "proms", 0 )                   /* Data proms ??? */
	ROM_LOAD( "buta-01.prm",  0x00000, 0x00100, CRC(45baedd0) SHA1(afdafb67d55007e6fb99518657e27ce61d2cb7e6) )
	ROM_LOAD( "buta-02.prm",  0x00100, 0x00100, CRC(0dcb18fc) SHA1(0b097b873c9484981f87a5e3d1af767f901ae05f) )
ROM_END


/*  ( YEAR   NAME     PARENT   MACHINE   INPUT    STATE          INIT        MONITOR COMPANY                 FULLNAME ) */
GAME( 1986, argus,    0,       argus,    argus,   argus_state,   empty_init, ROT270, "NMK (Jaleco license)", "Argus",                                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1986, valtric,  0,       valtric,  valtric, valtric_state, empty_init, ROT270, "NMK (Jaleco license)", "Valtric",                                     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1987, butasan,  0,       butasan,  butasan, butasan_state, empty_init, ROT0,   "NMK (Jaleco license)", "Butasan - Pig's & Bomber's (Japan, English)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1987, butasanj, butasan, butasan,  butasan, butasan_state, empty_init, ROT0,   "NMK (Jaleco license)", "Butasan (Japan, Japanese)",                   MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
