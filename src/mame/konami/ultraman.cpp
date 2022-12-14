// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Ultraman (c) 1991  Banpresto / Bandai

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    2009-03:
    Added dsw locations and verified factory setting based on Guru's notes

***************************************************************************/

#include "emu.h"
#include "ultraman.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "speaker.h"


void ultraman_state::sound_nmi_enable_w(uint8_t data)
{
	m_soundnmi->in_w<1>(BIT(data, 0));
}


void ultraman_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x08ffff).ram();
	map(0x180000, 0x183fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");/* Palette */
	map(0x1c0000, 0x1c0001).portr("SYSTEM");
	map(0x1c0002, 0x1c0003).portr("P1");
	map(0x1c0004, 0x1c0005).portr("P2");
	map(0x1c0006, 0x1c0007).portr("DSW1");
	map(0x1c0008, 0x1c0009).portr("DSW2");
	map(0x1c0018, 0x1c0019).w(FUNC(ultraman_state::ultraman_gfxctrl_w));   /* counters + gfx ctrl */
	map(0x1c0021, 0x1c0021).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x1c0029, 0x1c0029).w(m_soundnmi, FUNC(input_merger_device::in_set<0>));
	map(0x1c0030, 0x1c0031).w("watchdog", FUNC(watchdog_timer_device::reset16_w));
	map(0x204000, 0x204fff).rw(m_k051316[0], FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0x00ff); /* K051316 #0 RAM */
	map(0x205000, 0x205fff).rw(m_k051316[1], FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0x00ff); /* K051316 #1 RAM */
	map(0x206000, 0x206fff).rw(m_k051316[2], FUNC(k051316_device::read), FUNC(k051316_device::write)).umask16(0x00ff); /* K051316 #2 RAM */
	map(0x207f80, 0x207f9f).w(m_k051316[0], FUNC(k051316_device::ctrl_w)).umask16(0x00ff);   /* K051316 #0 registers */
	map(0x207fa0, 0x207fbf).w(m_k051316[1], FUNC(k051316_device::ctrl_w)).umask16(0x00ff);   /* K051316 #1 registers */
	map(0x207fc0, 0x207fdf).w(m_k051316[2], FUNC(k051316_device::ctrl_w)).umask16(0x00ff);   /* K051316 #2 registers */
	map(0x304000, 0x30400f).rw(m_k051960, FUNC(k051960_device::k051937_r), FUNC(k051960_device::k051937_w)).umask16(0x00ff);       /* Sprite control */
	map(0x304800, 0x304fff).rw(m_k051960, FUNC(k051960_device::k051960_r), FUNC(k051960_device::k051960_w)).umask16(0x00ff);       /* Sprite RAM */
}

void ultraman_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).ram();
	map(0xc000, 0xc000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xd000, 0xd000).w(FUNC(ultraman_state::sound_nmi_enable_w));
	map(0xe000, 0xe000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));       /* M6295 */
	map(0xf000, 0xf001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));   /* YM2151 */
}

void ultraman_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(m_soundnmi, FUNC(input_merger_device::in_clear<0>));
}


static INPUT_PORTS_START( ultraman )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin A" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )     /* 5C_3C according to manual, but it's not true */
	/* No Coin X = coin slot X open (coins produce sound), but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" ) /* Manual states it's "Unused" */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:7" ) /* Manual states it's "Unused" */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )          PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


void ultraman_state::machine_start()
{
	save_item(NAME(m_bank));
}

void ultraman_state::machine_reset()
{
	m_bank[0] = -1;
	m_bank[1] = -1;
	m_bank[2] = -1;

	m_soundnmi->in_w<0>(0);
}

void ultraman_state::ultraman(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 24000000/2);      /* 12 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ultraman_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(ultraman_state::irq4_line_hold));

	Z80(config, m_audiocpu, 24000000/6);    /* 4 MHz? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &ultraman_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &ultraman_state::sound_io_map);

	INPUT_MERGER_ALL_HIGH(config, "soundnmi").output_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	config.set_maximum_quantum(attotime::from_hz(600));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(14*8, (64-14)*8-1, 2*8, 30*8-1 );
	screen.set_screen_update(FUNC(ultraman_state::screen_update_ultraman));
	screen.set_palette("palette");

	auto &palette(PALETTE(config, "palette"));
	palette.set_format(palette_device::xRGB_555, 8192);
	palette.enable_shadows();

	K051960(config, m_k051960, 0);
	m_k051960->set_palette("palette");
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(ultraman_state::sprite_callback));

	K051316(config, m_k051316[0], 0);
	m_k051316[0]->set_palette("palette");
	m_k051316[0]->set_offsets(8, 0);
	m_k051316[0]->set_zoom_callback(FUNC(ultraman_state::zoom_callback_1));

	K051316(config, m_k051316[1], 0);
	m_k051316[1]->set_palette("palette");
	m_k051316[1]->set_offsets(8, 0);
	m_k051316[1]->set_zoom_callback(FUNC(ultraman_state::zoom_callback_2));

	K051316(config, m_k051316[2], 0);
	m_k051316[2]->set_palette("palette");
	m_k051316[2]->set_offsets(8, 0);
	m_k051316[2]->set_zoom_callback(FUNC(ultraman_state::zoom_callback_3));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "ymsnd", 24000000/6).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}



ROM_START( ultraman )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "910-b01.c11",  0x000000, 0x020000, CRC(3d9e4323) SHA1(54ee218c9be1ac029836624839d0845b39e6e30f) )
	ROM_LOAD16_BYTE(    "910-b02.d11",  0x000001, 0x020000, CRC(d24c82e9) SHA1(e792e2601e235939546fe98d52bfafe5a95b3491) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 code */
	ROM_LOAD( "910-a05.d05",    0x00000, 0x08000, CRC(ebaef189) SHA1(73e6163466d55ae782f55839ba9c98f06c30876b) )

	ROM_REGION( 0x100000, "k051960", 0 )   /* Sprites */
	ROM_LOAD32_WORD( "910-a19.l04",    0x000000, 0x080000, CRC(2dc9ffdc) SHA1(aa34247c82d48c8d13f5209be292127938a4a682) )
	ROM_LOAD32_WORD( "910-a20.l01",    0x000002, 0x080000, CRC(a4298dce) SHA1(62faf8f0c0490a9562b75ce27909fbee6e84b22a) )

	ROM_REGION( 0x080000, "k051316_1", 0 )
	ROM_LOAD( "910-a07.j15",    0x000000, 0x020000, CRC(8b43a64e) SHA1(e373d0fd88b59fb01782dfaeccb1e13673a35766) )
	ROM_LOAD( "910-a08.j16",    0x020000, 0x020000, CRC(c3829826) SHA1(0d383a7afac2a3b5da692375a2b2cd675848861a) )
	ROM_LOAD( "910-a09.j18",    0x040000, 0x020000, CRC(ee10b519) SHA1(a34bd7d89bb8a19af7252ed96ffce212788c586b) )
	ROM_LOAD( "910-a10.j19",    0x060000, 0x020000, CRC(cffbb0c3) SHA1(e9ebe350289f0436de10a6289b04eed3b6a9f98e) )

	ROM_REGION( 0x080000, "k051316_2", 0 )
	ROM_LOAD( "910-a11.l15",    0x000000, 0x020000, CRC(17a5581d) SHA1(aca5d465a0e181a266a165aeb0112a4696b0cd18) )
	ROM_LOAD( "910-a12.l16",    0x020000, 0x020000, CRC(39763fb5) SHA1(0e1795af4bae545a0a2be265398837fb2d623232) )
	ROM_LOAD( "910-a13.l18",    0x040000, 0x020000, CRC(66b25a4f) SHA1(954552b005582c90d570ae32c715108ec4b088f1) )
	ROM_LOAD( "910-a14.l19",    0x060000, 0x020000, CRC(09fbd412) SHA1(d11587db7b03f3a75ad8964523bb34f4453bbaca) )

	ROM_REGION( 0x080000, "k051316_3", 0 )
	ROM_LOAD( "910-a15.m15",    0x000000, 0x020000, CRC(6d5bfbb7) SHA1(e98c594446b506cb32cc5cc958d2f0de22ebed5e) )
	ROM_LOAD( "910-a16.m16",    0x020000, 0x020000, CRC(5f6f8c3d) SHA1(e365836d2263f36aa4602f0618bf7ce693d2e106) )
	ROM_LOAD( "910-a17.m18",    0x040000, 0x020000, CRC(1f3ec4ff) SHA1(875f53516f47decc4ce31154cf4694c8429ee4ea) )
	ROM_LOAD( "910-a18.m19",    0x060000, 0x020000, CRC(fdc42929) SHA1(079827c1b1a3c32f8547dd91bba8ae37034c16be) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "910-a21.f14",    0x000000, 0x000100, CRC(64460fbc) SHA1(b5295e1d3303d5d816ad44da7b011bbfa613f9e4) )  /* priority encoder (not used) */

	ROM_REGION( 0x040000, "oki", 0 )    /* M6295 data */
	ROM_LOAD( "910-a06.c06",    0x000000, 0x040000, CRC(28fa99c9) SHA1(54663d79ee105ac18d6ba01333a52e3732f2e5fe) )
ROM_END


GAME( 1991, ultraman, 0, ultraman, ultraman, ultraman_state, empty_init, ROT0, "Banpresto / Bandai", "Ultraman (Japan)", MACHINE_SUPPORTS_SAVE )
