// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dgnalpha.c

    Dragon Alpha

    The Dragon Alpha was a prototype in development when Dragon Data went bust,
    it is basically an enhanced Dragon 64, with built in modem, disk system, and
    graphical boot rom.

    It has the following extra hardware :-
    A third 6821 PIA mapped between FF24 and FF27
        An AY-8912, connected to the PIA.

    Port A of the PIA is connected as follows :-

        b0  BDIR of AY8912
        b1  BC1 of AY8912
        b2  Rom select, High= boot rom, low=BASIC rom
        b3..7 not used.

    Port B
        b0..7 connected to D0..7 of the AY8912.

    CB1 DRQ of WD2797.

    /irqa
    /irqb   both connected to 6809 FIRQ.


    The analog outputs of the AY-8912 are connected to the standard sound multiplexer.
    The AY8912 output port is used as follows :-

        b0..b3  /DS0../DS3 for the drive interface (through an inverter first).
        b4      /motor for the drive interface (through an inverter first).
        b5..b7  not used as far as I can tell.

    A 6850 for the modem.

    A WD2797, used as an internal disk interface, this is however connected in a slightly strange
    way that I am yet to completely determine.
    19/10/2004, WD2797 is mapped between FF2C and FF2F, however the order of the registers is
    reversed so the command Register is at the highest address instead of the lowest. The Data
    request pin is connected to CB1(pin 18) of PIA2, to cause an firq, the INTRQ, is connected via
    an inverter to the 6809's NMI.

    All these are as yet un-emulated.

    29-Oct-2004, AY-8912 is now emulated.
    30-Oct-2004, Internal disk interface now emulated, Normal DragonDos rom replaced with a re-assembled
                version, that talks to the alpha hardware (verified on a clone of the real machine).

Dragon Alpha code added 21-Oct-2004,
            Phill Harvey-Smith (afra@aurigae.demon.co.uk)

            Added AY-8912 and FDC code 30-Oct-2004.

Fixed Dragon Alpha NMI enable/disable, following circuit traces on a real machine.
    P.Harvey-Smith, 11-Aug-2005.

Re-implemented Alpha NMI enable/disable, using direct PIA reads, rather than
keeping track of it in a variable in the driver.
    P.Harvey-Smith, 25-Sep-2006.

***************************************************************************/

#include "emu.h"
#include "dgnalpha.h"
#include "sound/ay8910.h"

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void dragon_alpha_state::device_start(void)
{
	dragon64_state::device_start();
}



//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void dragon_alpha_state::device_reset(void)
{
	dragon64_state::device_reset();
}


/***************************************************************************
  MODEM
***************************************************************************/

//-------------------------------------------------
//  modem_r
//-------------------------------------------------

uint8_t dragon_alpha_state::modem_r(offs_t offset)
{
	return 0xFF;
}



//-------------------------------------------------
//  modem_w
//-------------------------------------------------

void dragon_alpha_state::modem_w(offs_t offset, uint8_t data)
{
}



/***************************************************************************
  PIA2 ($FF24-$FF28) on Dragon Alpha/Professional

    PIA2 PA0        bcdir to AY-8912
    PIA2 PA1        bc0 to AY-8912
    PIA2 PA2        Rom switch, 0=basic rom, 1=boot rom.
    PIA2 PA3-PA7    Unknown/unused ?
    PIA2 PB0-PB7    connected to D0..7 of the AY8912.
    CB1             DRQ from WD2797 disk controller.
***************************************************************************/

//-------------------------------------------------
//  pia2_pa_w
//-------------------------------------------------

void dragon_alpha_state::pia2_pa_w(uint8_t data)
{
	uint8_t ddr = ~m_pia_2->port_b_z_mask();

	/* If bit 2 of the pia2 ddra is 1 then this pin is an output so use it */
	/* to control the paging of the boot and basic roms */
	/* Otherwise it set as an input, with an internal pull-up so it should */
	/* always be high (enabling boot rom) */
	/* PIA FIXME if (pia_get_ddr_a(2) & 0x04) */
	if(ddr & 0x04)
	{
		page_rom(data & 0x04 ? true : false);   /* bit 2 controls boot or basic rom */
	}

	/* Bits 0 and 1 for pia2 port a control the BCDIR and BC1 lines of the */
	/* AY-8912 */
	switch (data & 0x03)
	{
		case 0x00:      /* Inactive, do nothing */
			break;
		case 0x01:      /* Write to selected port */
			m_ay8912->data_w(m_pia_2->b_output());
			break;
		case 0x02:      /* Read from selected port */
			m_pia_2->portb_w(m_ay8912->data_r());
			break;
		case 0x03:      /* Select port to write to */
			m_ay8912->address_w(m_pia_2->b_output());
			break;
	}
}



/***************************************************************************
  AY8912
***************************************************************************/

//-------------------------------------------------
//  psg_porta_read
//-------------------------------------------------

uint8_t dragon_alpha_state::psg_porta_read()
{
	return 0;
}



//-------------------------------------------------
//  psg_porta_read
//-------------------------------------------------

void dragon_alpha_state::psg_porta_write(uint8_t data)
{
	/* Bits 0..3 are the drive select lines for the internal floppy interface */
	/* Bit 4 is the motor on, in the real hardware these are inverted on their way to the drive */
	/* Bits 5,6,7 are connected to /DDEN, ENP and 5/8 on the WD2797 */

	floppy_image_device *floppy = nullptr;

	for (int n = 0; n < 4; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	// todo: turning the motor on with bit 4 isn't giving the drive enough
	// time to spin up, how does it work in hardware?
	for (auto &f : m_floppy)
		if (f->get_device()) f->get_device()->mon_w(0);

	m_fdc->dden_w(BIT(data, 5));
}

/***************************************************************************
  FDC
***************************************************************************/

//-------------------------------------------------
//  fdc_intrq_w - The NMI line on the Alpha is gated
//  through IC16 (early PLD), and is gated by pia2 CA2
//-------------------------------------------------

WRITE_LINE_MEMBER( dragon_alpha_state::fdc_intrq_w )
{
	if (state)
	{
		if (m_pia_2->ca2_output_z())
			m_nmis->in_w<1>(1);
	}
	else
	{
		m_nmis->in_w<1>(0);
	}
}



//-------------------------------------------------
//  fdc_drq_w - The DRQ line goes through pia2 CB1,
//  in exactly the same way as DRQ from DragonDos
//  does for pia1 CB1
//-------------------------------------------------

WRITE_LINE_MEMBER( dragon_alpha_state::fdc_drq_w )
{
	m_pia_2->cb1_w(state ? ASSERT_LINE : CLEAR_LINE);
}
