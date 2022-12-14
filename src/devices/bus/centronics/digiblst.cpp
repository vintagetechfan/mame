// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * digiblst.cpp
 *
 *  Created on: 23/08/2014
 */

#include "emu.h"
#include "digiblst.h"
#include "speaker.h"

//**************************************************************************
//  DIGIBLASTER DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(CENTRONICS_DIGIBLASTER, centronics_digiblaster_device, "cpcdigiblst", "Digiblaster (DIY)")


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
//-------------------------------------------------
//  centronics_digiblaster_device - constructor
//-------------------------------------------------

centronics_digiblaster_device::centronics_digiblaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CENTRONICS_DIGIBLASTER, tag, owner, clock),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_dac(*this, "dac"),
	m_data(0)
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void centronics_digiblaster_device::device_add_mconfig(machine_config &config)
{
	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void centronics_digiblaster_device::device_start()
{
	save_item(NAME(m_data));
}

void centronics_digiblaster_device::update_dac()
{
	if (started())
		m_dac->write(m_data);
}
