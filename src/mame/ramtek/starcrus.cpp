// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Ryan Holtz
/***************************************************************************

    Ramtek Star Cruiser Driver

    (no known issues)

    Frank Palazzolo
    palazzol@home.com

    Netlist Audio by Ryan Holtz

***************************************************************************/

#include "emu.h"
#include "starcrus.h"

#include "cpu/i8085/i8085.h"
#include "screen.h"
#include "speaker.h"

void starcrus_state::machine_start()
{
	m_led.resolve();
}

void starcrus_state::starcrus_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x10ff).ram();
}

void starcrus_state::starcrus_io_map(address_map &map)
{
	map(0x00, 0x00).portr("P1").w(FUNC(starcrus_state::s1_x_w));
	map(0x01, 0x01).portr("P2").w(FUNC(starcrus_state::s1_y_w));
	map(0x02, 0x02).rw(FUNC(starcrus_state::coll_det_r), FUNC(starcrus_state::s2_x_w));
	map(0x03, 0x03).portr("DSW").w(FUNC(starcrus_state::s2_y_w));
	map(0x04, 0x04).w(FUNC(starcrus_state::p1_x_w));
	map(0x05, 0x05).w(FUNC(starcrus_state::p1_y_w));
	map(0x06, 0x06).w(FUNC(starcrus_state::p2_x_w));
	map(0x07, 0x07).w(FUNC(starcrus_state::p2_y_w));
	map(0x08, 0x08).w(FUNC(starcrus_state::ship_parm_1_w));
	map(0x09, 0x09).w(FUNC(starcrus_state::ship_parm_2_w));
	map(0x0a, 0x0a).w(FUNC(starcrus_state::proj_parm_1_w));
	map(0x0b, 0x0b).w(FUNC(starcrus_state::proj_parm_2_w));
}


static INPUT_PORTS_START( starcrus )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY /* ccw */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* engine */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY /* cw */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* torpedo */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* phaser */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2) /* ccw */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* engine */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2) /* cw */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* torpedo */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* phaser */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x03, "60 secs" )
	PORT_DIPSETTING(    0x02, "90 secs" )
	PORT_DIPSETTING(    0x01, "120 secs" )
	PORT_DIPSETTING(    0x00, "150 secs" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x20, 0x20, "Mode" )
	PORT_DIPSETTING(    0x20, DEF_STR( Standard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("POT_1")
	PORT_ADJUSTER( 50, "Pot: Noise Level" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "noise_volume")

	PORT_START("POT_2")
	PORT_ADJUSTER( 50, "Pot: Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "volume")
INPUT_PORTS_END



static const gfx_layout spritelayout1 =
{
	16,16,    /* 16x16 sprites */
	4,          /* 4 sprites */
	1,      /* 1 bits per pixel */
	{ 0 },  /* 1 chip */
	{ 0*8+4,  0*8+4,  1*8+4,  1*8+4, 2*8+4, 2*8+4, 3*8+4, 3*8+4,
		4*8+4,  4*8+4,  5*8+4,  5*8+4, 6*8+4, 6*8+4, 7*8+4, 7*8+4 },
	{ 0, 0, 1*64, 1*64, 2*64, 2*64, 3*64, 3*64,
		4*64, 4*64, 5*64, 5*64, 6*64, 6*64, 7*64, 7*64 },
	1  /* every sprite takes 1 consecutive bit */
};
static const gfx_layout spritelayout2 =
{
	16,16,   /* 16x16 sprites */
	4,       /* 4 sprites */
	1,       /* 1 bits per pixel */
	{ 0 },   /* 1 chip */
	{ 0*8+4,  1*8+4,  2*8+4,  3*8+4, 4*8+4, 5*8+4, 6*8+4, 7*8+4,
		8*8+4,  9*8+4,  10*8+4,  11*8+4, 12*8+4, 13*8+4, 14*8+4, 15*8+4 },
	{ 0, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	1 /* every sprite takes 1 consecutive bytes */
};

static GFXDECODE_START( gfx_starcrus )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0040, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0080, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x00c0, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0040, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0080, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x00c0, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0100, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0200, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0300, spritelayout2, 0, 1 )
GFXDECODE_END


void starcrus_state::starcrus(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 9750000/9);  /* 8224 chip is a divide by 9 */
	m_maincpu->set_addrmap(AS_PROGRAM, &starcrus_state::starcrus_map);
	m_maincpu->set_addrmap(AS_IO, &starcrus_state::starcrus_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(starcrus_state::irq0_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(starcrus_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_starcrus);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(starcrus))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:explode1", "EXPLODE_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:explode2", "EXPLODE_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:launch1", "LAUNCH_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:launch2", "LAUNCH_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:engine1", "ENGINE_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:engine2", "ENGINE_2.IN", 0);
	NETLIST_ANALOG_INPUT(config, "sound_nl:noise_volume", "R23.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:volume", "R75.DIAL");

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "R77.2").set_mult_offset(100000.0 / 32768.0, 0.0);
}

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( starcrus )
	ROM_REGION( 0x10000, "maincpu", 0 )  /* code */
	ROM_LOAD( "starcrus.j1",   0x0000, 0x0200, CRC(0ee60a50) SHA1(7419e7cb4c589da53d4a10ad129373502682464e) )
	ROM_LOAD( "starcrus.k1",   0x0200, 0x0200, CRC(a7bc3bc4) SHA1(0e38076e921856608b1dd712687bef1c2522b4b8) )
	ROM_LOAD( "starcrus.l1",   0x0400, 0x0200, CRC(10d233ec) SHA1(8933cf9fc51716a9e8f75a4444e7d7070cf5834d) )
	ROM_LOAD( "starcrus.m1",   0x0600, 0x0200, CRC(2facbfee) SHA1(d78fb38de49da938fce2b55c8decc244efee6f94) )
	ROM_LOAD( "starcrus.n1",   0x0800, 0x0200, CRC(42083247) SHA1(b32d67c914833f18e9955cd1c3cb1d948be0a7d5) )
	ROM_LOAD( "starcrus.p1",   0x0a00, 0x0200, CRC(61dfe581) SHA1(e1802fedf94541e9ccd9786b60e90890485f422f) )
	ROM_LOAD( "starcrus.r1",   0x0c00, 0x0200, CRC(010cdcfe) SHA1(ae76f1739b468e2987ce949470b36f1a873e061d) )
	ROM_LOAD( "starcrus.s1",   0x0e00, 0x0200, CRC(da4e276b) SHA1(3298f7cb259803f118a47292cbb413df253ef74d) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "starcrus.e6",   0x0000, 0x0200, CRC(54887a25) SHA1(562bf85cd063c2cc0a2f803095aaa6138dfb5bff) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD( "starcrus.l2",   0x0000, 0x0200, CRC(54887a25) SHA1(562bf85cd063c2cc0a2f803095aaa6138dfb5bff) )

	ROM_REGION( 0x0400, "gfx3", 0 )
	ROM_LOAD( "starcrus.j4",   0x0000, 0x0200, CRC(25f15ae1) SHA1(7528edaa01ad5a167191c7e72394cb6009db1b27) )
	ROM_LOAD( "starcrus.g5",   0x0200, 0x0200, CRC(73b27f6e) SHA1(4a6cf9244556a2c2647d594c7a19fe1a374a57e6) )
ROM_END


GAME( 1977, starcrus, 0, starcrus, starcrus, starcrus_state, empty_init, ROT0, "Ramtek", "Star Cruiser", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
