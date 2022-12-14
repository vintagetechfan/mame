// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Monochrome register-addressable framebuffer used in DVK.

    To do: overlay on IE15's screen.

***************************************************************************/

#include "emu.h"

#include "dvk_kgd.h"

#include "emupal.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DVK_KGD, dvk_kgd_device, "kgd", "DVK KGD framebuffer")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dvk_kgd_device - constructor
//-------------------------------------------------

dvk_kgd_device::dvk_kgd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DVK_KGD, tag, owner, clock)
	, device_qbus_card_interface(mconfig, *this)
	, m_screen(*this, "screen")
{
}


uint32_t dvk_kgd_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// CR bit 15: display graphics
	// CR bit 14: pass through video signal (usually from IE15)
	if (!(m_cr & (2 << 14)))
	{
		bitmap.fill(0);
		return 0;
	}

	for (u16 y = 0; y < 286; y++)
	{
		for (u16 x = 0; x < 50; x++)
		{
			u16 const code = m_videoram[y * 50 + x];
			for (u8 b = 0; b < 8; b++)
			{
				bitmap.pix(y, x * 16 + b + b) = BIT(code, b);
				bitmap.pix(y, x * 16 + b + b + 1) = BIT(code, b);
			}
		}
	}
	return 0;
}

void dvk_kgd_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(dvk_kgd_device::screen_update));
	m_screen->set_raw(XTAL(30'800'000) / 2, 800, 0, 800, 286, 0, 286);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dvk_kgd_device::device_start()
{
	m_bus->install_device(0176640, 0176647, read16sm_delegate(*this, FUNC(dvk_kgd_device::read)),
		write16sm_delegate(*this, FUNC(dvk_kgd_device::write)));

	// save state
	save_item(NAME(m_cr));
	save_item(NAME(m_dr));
	save_item(NAME(m_ar));
	save_item(NAME(m_ct));

	m_videoram_base = std::make_unique<uint8_t[]>(16384);
	m_videoram = m_videoram_base.get();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dvk_kgd_device::device_reset()
{
	m_cr = m_dr = m_ar = m_ct = 0;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t dvk_kgd_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset & 3)
	{
	case 0:
		data = m_cr;
		break;

	case 1:
		data = m_videoram[m_ar];
		break;

	case 2:
		data = m_ar;
		break;

	case 3:
		data = m_ct;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void dvk_kgd_device::write(offs_t offset, uint16_t data)
{
	switch (offset & 3)
	{
	case 0:
		m_cr = ((m_cr & ~KGDCR_WR) | (data & KGDCR_WR));
		break;

	case 1:
		m_dr = ((m_dr & ~KGDDR_WR) | (data & KGDDR_WR));
		m_videoram[m_ar] = m_dr;
		break;

	case 2:
		m_ar = ((m_ar & ~KGDAR_WR) | (data & KGDAR_WR));
		break;

	case 3:
		m_ct = ((m_ct & ~KGDCT_WR) | (data & KGDCT_WR));
		break;
	}
}
