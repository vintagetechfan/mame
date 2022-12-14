// license:BSD-3-Clause
// copyright-holders:Curt Coder,Jonathan Gevaryahu
/*

    Tandberg TDV2324

    Skeleton driver
    By Curt Coder with some work by Lord Nightmare

    Status:
    * Main cpu is currently hacked to read i/o port 0xE6 as 0x10;
      it then seems to copy code to a ram area then jumps there
      (there may be some sort of overlay/banking mess going on to allow full 64kb of ram)
      The cpu gets stuck reading i/o port 0x30 in a loop.
      - interrupts and sio lines are not hooked up
    * Sub cpu does a bunch of unknown i/o accesses and also tries to
      sequentially read chunk of address space which it never writes to;
      this seems likely to be a shared ram or i/o mapped area especially since it seems
      to write to i/o port 0x60 before trying to read there.
      - interrupts and sio lines are not hooked up
    * Fdc cpu starts, does a rom checksum (which passes) and tests a ram area


    Board Notes:
    Mainboard (pictures P1010036 & P1010038)
    *28-pin: D27128, L4267096S,...(eprom, occluded by sticker: "965268 1", character set)
    *40-pin: TMS9937NL, DB 336, ENGLAND (VTAC Video Chip)
    *40-pin: P8085AH-2, F4265030, C INTEL '80 (cpus, there are 2 of these)
    *28-pin: JAPAN 8442, 00009SS0, HN4827128G-25 (eprom, sticker: "962107")
    *22-pin: ER3400, GI 8401HHA (EAROM)
    *  -pin: MOSTEK C 8424, MK3887N-4 (Z80-SIO/2 Serial I/O Controller)
    *20-pin: (pal, sticker: "961420 0")
    *24-pin: D2716, L3263271, INTEL '77 (eprom, sticker: "962058 1")
    *3 tiny 16-pins which look socketed (proms)
    *+B8412, DMPAL10L8NC
    *PAL... (can't remove the sticker to read the rest since there's electrical components soldered above the chip)
    *Am27S21DC, 835IDmm
    *AM27S13DC, 8402DM (x2)
    *TBP28L22N, GERMANY 406 A (x2)
    *PAL16L6CNS, 8406

    FD/HD Interface Board P/N 962013 Rev14 (pictures P1010031 & P1010033)
    *28-pin: TMS, 2764JL-25, GHP8414 (@U15, labeled "962014 // -4-", fdc cpu rom)
    *40-pin: MC68B02P, R1H 8340 (fdc cpu)
    *40-pin: WDC '79, FD1797PL-02, 8342 16 (fdc chip)
    *14-pin: MC4024P, MG 8341 (dual voltage controlled multivibrator)
    *24-pin: TMM2016AP-12 (@U14 and @U80, 120ns 2kx8 SRAM)

    Keyboard:
    *40-pin: NEC D8035LC (mcs-48 cpu)
    *24-pin: NEC D2716 (eprom)

    Main CPU:
    - PIT, SIO

    Sub CPU:
    - PIC, PIT, VTAC

*/
/*
'subcpu' (17CD): unmapped i/o memory write to 23 = 36 & FF
'subcpu' (17D1): unmapped i/o memory write to 23 = 76 & FF
'subcpu' (17D5): unmapped i/o memory write to 23 = B6 & FF
'subcpu' (17DB): unmapped i/o memory write to 20 = 1A & FF
'subcpu' (17DE): unmapped i/o memory write to 20 = 00 & FF
'subcpu' (17E0): unmapped i/o memory write to 3E = 00 & FF
'subcpu' (17E2): unmapped i/o memory write to 3A = 00 & FF
'subcpu' (17E6): unmapped i/o memory write to 30 = 74 & FF
'subcpu' (17EA): unmapped i/o memory write to 31 = 7F & FF
'subcpu' (17EE): unmapped i/o memory write to 32 = 6D & FF
'subcpu' (17F2): unmapped i/o memory write to 33 = 18 & FF
'subcpu' (17F6): unmapped i/o memory write to 34 = 49 & FF
'subcpu' (17FA): unmapped i/o memory write to 35 = 20 & FF
'subcpu' (17FE): unmapped i/o memory write to 36 = 18 & FF
'subcpu' (1801): unmapped i/o memory write to 3C = 00 & FF
'subcpu' (1803): unmapped i/o memory write to 3C = 00 & FF
'subcpu' (1805): unmapped i/o memory write to 3E = 00 & FF
'subcpu' (0884): unmapped i/o memory write to 10 = 97 & FF
'subcpu' (0888): unmapped i/o memory write to 10 = 96 & FF

'fdccpu' (E004): unmapped program memory read from 3C05 & FF  0011 1100 0000 0101
'fdccpu' (E007): unmapped program memory read from C000 & FF  1100 0000 0000 0000
'fdccpu' (E00A): unmapped program memory read from A000 & FF  1010 0000 0000 0000
'fdccpu' (E012): unmapped program memory write to F000 = D0 & FF 1111 0000 0000 0000 = 1101 0000
'fdccpu' (E015): unmapped program memory read from 3801 & FF  0011 1000 0000 0001
'fdccpu' (E018): unmapped program memory read from 3C06 & FF  0011 1100 0000 0110
'fdccpu' (E01B): unmapped program memory read from 3C04 & FF  0011 1100 0000 0100

'fdccpu' (E070): unmapped program memory write to 2101 = 01 & FF
'fdccpu' (E07C): unmapped program memory read from 6000 & FF
'fdccpu' (E07F): unmapped program memory read from 380D & FF
'fdccpu' (E082): unmapped program memory read from 380F & FF
'fdccpu' (E085): unmapped program memory read from 3803 & FF
'fdccpu' (E08B): unmapped program memory write to 6000 = 08 & FF
'fdccpu' (E08E): unmapped program memory write to 8000 = 08 & FF
'fdccpu' (E091): unmapped program memory write to 6000 = 00 & FF
'fdccpu' (E099): unmapped program memory write to F800 = 55 & FF
...
*/



#include "emu.h"
#include "tdv2324.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"


uint8_t tdv2324_state::tdv2324_main_io_30()
{
	return 0xff;
}

// Not sure what this is for, i/o read at 0xE6 on maincpu, post fails if it does not return bit 4 set
uint8_t tdv2324_state::tdv2324_main_io_e6()
{
	return 0x10; // TODO: this should actually return something meaningful, for now is enough to pass early boot test
}

void tdv2324_state::tdv2324_main_io_e2(uint8_t data)
{
	printf("%c\n", data);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_mem )
//-------------------------------------------------

void tdv2324_state::tdv2324_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).rom().region(P8085AH_0_TAG, 0);
	/* when copying code to 4000 area it runs right off the end of rom;
	 * I'm not sure if its supposed to mirror or read as open bus */
//  map(0x4000, 0x5fff).ram(); // 0x4000 has the boot code copied to it, 5fff and down are the stack
//  map(0x6000, 0x6fff).ram(); // used by the relocated boot code; shared?
	map(0x0800, 0xffff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_io )
//-------------------------------------------------

void tdv2324_state::tdv2324_io(address_map &map)
{
	//map.global_mask(0xff);
	/* 0x30 is read by main code and if high bit isn't set at some point it will never get anywhere */
	/* e0, e2, e8, ea are written to */
	/* 30, e6 and e2 are readable */
	map(0x30, 0x30).r(FUNC(tdv2324_state::tdv2324_main_io_30));
//  map(0xe2, 0xe2).w(FUNC(tdv2324_state::tdv2324_main_io_e2)); console output
	map(0xe6, 0xe6).r(FUNC(tdv2324_state::tdv2324_main_io_e6));
//  map(0x, 0x).rw(P8253_5_0_TAG, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
//  map(0x, 0x).rw(MK3887N4_TAG, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
//  map(0x, 0x).rw(P8259A_TAG, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_sub_mem )
//-------------------------------------------------

void tdv2324_state::tdv2324_sub_mem(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(P8085AH_1_TAG, 0);
	map(0x4000, 0x47ff).ram();
	map(0x5000, 0x53ff).ram(); // EAROM
	map(0x6000, 0x7fff).ram().share("video_ram");
}


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_sub_io )
//-------------------------------------------------

void tdv2324_state::tdv2324_sub_io(address_map &map)
{
	//map.global_mask(0xff);
	/* 20, 23, 30-36, 38, 3a, 3c, 3e, 60, 70 are written to */
	map(0x20, 0x23).rw(m_pit1, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x30, 0x3f).rw(m_tms, FUNC(tms9927_device::read), FUNC(tms9927_device::write)); // TODO: this is supposed to be a 9937, which is not quite the same as 9927
}


//-------------------------------------------------
//  ADDRESS_MAP( tdv2324_fdc_mem )
//-------------------------------------------------

void tdv2324_state::tdv2324_fdc_mem(address_map &map)
{
	// the following two are probably enabled/disabled via the JP2 jumper block next to the fdc cpu
	//map(0x0000, 0x001f).ram(); // on-6802-die ram (optionally battery backed)
	//map(0x0020, 0x007f).ram(); // on-6802-die ram
	map(0x0000, 0x07ff).ram(); // TMM2016AP-12 @ U14, tested with A5,5A pattern
	//map(0x1000, 0x17ff).ram(); // TMM2016AP-12 @ U80, address is wrong
	// the 3xxx area appears to be closely involved in fdc or other i/o
	// in particular, reads from 30xx, 38xx, 3Cxx may be actually writes to certain fdc registers with data xx?
	// 0x2101 is something writable
	// 0x8000 is either a read from reg 0 (status reg) of the FD1797, OR a read from some sort of status from other cpus
	// 0x8000 can also be written to
	// 0x6000 can also be read from and written to
	// Somewhere in here, the FDC chip and the hard disk interface live
	map(0xe000, 0xffff).rom().region(MC68B02P_TAG, 0); // rom "962014 // -4-" @U15
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( tdv2324 )
//-------------------------------------------------

static INPUT_PORTS_START( tdv2324 )
INPUT_PORTS_END



//**************************************************************************
//  VIDEO
//**************************************************************************

void tdv2324_state::video_start()
{
}


uint32_t tdv2324_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  SLOT_INTERFACE( tdv2324_floppies )
//-------------------------------------------------

static void tdv2324_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  machine_config( tdv2324 )
//-------------------------------------------------

void tdv2324_state::tdv2324(machine_config &config)
{
	// basic system hardware
	i8085a_cpu_device &maincpu(I8085A(config, m_maincpu, 8700000/2)); // ???
	maincpu.set_addrmap(AS_PROGRAM, &tdv2324_state::tdv2324_mem);
	maincpu.set_addrmap(AS_IO, &tdv2324_state::tdv2324_io);
	maincpu.in_inta_func().set(P8259A_TAG, FUNC(pic8259_device::acknowledge));

	I8085A(config, m_subcpu, 8000000/2); // ???
	m_subcpu->set_addrmap(AS_PROGRAM, &tdv2324_state::tdv2324_sub_mem);
	m_subcpu->set_addrmap(AS_IO, &tdv2324_state::tdv2324_sub_io);

	M6802(config, m_fdccpu, 8000000/2); // ???
	m_fdccpu->set_ram_enable(false);
	m_fdccpu->set_addrmap(AS_PROGRAM, &tdv2324_state::tdv2324_fdc_mem);

	// video hardware
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(tdv2324_state::screen_update));
	screen.set_size(800, 400);
	screen.set_visarea(0, 800-1, 0, 400-1);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	TMS9927(config, m_tms, 25.39836_MHz_XTAL / 8).set_char_width(8);

	// devices
	PIC8259(config, m_pic);

	PIT8253(config, m_pit0);

	PIT8253(config, m_pit1);

	Z80SIO(config, MK3887N4_TAG, 8000000/2);

	FD1797(config, FD1797PL02_TAG, 8000000/4);
	FLOPPY_CONNECTOR(config, FD1797PL02_TAG":0", tdv2324_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, FD1797PL02_TAG":1", tdv2324_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("tdv2324");
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( tdv2324 )
//-------------------------------------------------

ROM_START( tdv2324 )
	ROM_REGION( 0x800, P8085AH_0_TAG, 0 )
	ROM_LOAD( "962058-1.21g", 0x000, 0x800, CRC(3771aece) SHA1(36d3f03235f327d6c8682e5c167aed6dddfaa6ec) )

	ROM_REGION( 0x4000, P8085AH_1_TAG, 0 )
	ROM_LOAD( "962107-1.12c", 0x0000, 0x4000, CRC(29c1a139) SHA1(f55fa9075fdbfa6a3e94e5120270179f754d0ea5) )

	ROM_REGION( 0x2000, MC68B02P_TAG, 0 )
	ROM_LOAD( "962014-4.13c", 0x0000, 0x2000, CRC(d01c32cd) SHA1(1f00f5f5ff0c035eec6af820b5acb6d0c207b6db) )

	ROM_REGION( 0x800, "keyboard_8035", 0 )
	ROM_LOAD( "961294-3.u8", 0x000, 0x800, NO_DUMP )

	ROM_REGION( 0x4000, "chargen", 0 )
	ROM_LOAD( "965268-1.4g", 0x0000, 0x4000, CRC(7222a85f) SHA1(e94074b68d90698734ab1fc38d156407846df47c) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "961487-1.3f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.4f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.8g", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom.10f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "961420-0.16f", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "962103-0.15g", 0x0000, 0x0200, NO_DUMP )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "962108-0.2g", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "pal.12d", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "pal.13d", 0x0000, 0x0200, NO_DUMP )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY     FULLNAME    FLAGS
COMP( 1983, tdv2324, 0,      0,      tdv2324, tdv2324, tdv2324_state, empty_init, "Tandberg", "TDV 2324", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
