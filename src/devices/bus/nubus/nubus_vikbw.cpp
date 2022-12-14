// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Viking 1024x768 fixed-resolution monochrome board

  VRAM from Fs040000 to Fs0517FF
  Read from Fs000000 enables VBL, write to Fs000000 disables VBL
  Write to Fs080000 acks VBL

***************************************************************************/

#include "emu.h"
#include "nubus_vikbw.h"
#include "screen.h"

#define VIKBW_SCREEN_NAME   "vikbw_screen"
#define VIKBW_ROM_REGION    "vikbw_rom"

#define VRAM_SIZE   (0x18000)  // 1024x768 @ 1bpp is 98,304 bytes (0x18000)


ROM_START( vikbw )
	ROM_REGION(0x2000, VIKBW_ROM_REGION, 0)
	ROM_LOAD( "viking.bin",   0x000000, 0x002000, CRC(92cf04d1) SHA1(d08349edfc82a0bd5ea848e053e1712092308f74) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NUBUS_VIKBW, nubus_vikbw_device, "nb_vikbw", "Moniterm Viking video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nubus_vikbw_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, VIKBW_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(nubus_vikbw_device::screen_update));
	screen.set_size(1024, 768);
	screen.set_visarea(0, 1024-1, 0, 768-1);
	screen.set_refresh_hz(70);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nubus_vikbw_device::device_rom_region() const
{
	return ROM_NAME( vikbw );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_vikbw_device - constructor
//-------------------------------------------------

nubus_vikbw_device::nubus_vikbw_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nubus_vikbw_device(mconfig, NUBUS_VIKBW, tag, owner, clock)
{
}

nubus_vikbw_device::nubus_vikbw_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_nubus_card_interface(mconfig, *this),
	m_vbl_disable(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_vikbw_device::device_start()
{
	uint32_t slotspace;

	install_declaration_rom(VIKBW_ROM_REGION, true);

	slotspace = get_slotspace();

//  printf("[vikbw %p] slotspace = %x\n", this, slotspace);

	m_vram.resize(VRAM_SIZE / sizeof(uint32_t));
	install_bank(slotspace+0x40000, slotspace+0x40000+VRAM_SIZE-1, &m_vram[0]);
	install_bank(slotspace+0x940000, slotspace+0x940000+VRAM_SIZE-1, &m_vram[0]);

	nubus().install_device(slotspace, slotspace+3, read32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_enable_r)), write32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_disable_w)));
	nubus().install_device(slotspace+0x80000, slotspace+0x80000+3, read32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_ack_r)), write32smo_delegate(*this, FUNC(nubus_vikbw_device::viking_ack_w)));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_vikbw_device::device_reset()
{
	m_vbl_disable = 1;
	std::fill(m_vram.begin(), m_vram.end(), 0);

	m_palette[0] = rgb_t(255, 255, 255);
	m_palette[1] = rgb_t(0, 0, 0);
}

/***************************************************************************

  Viking 1024x768 B&W card section

***************************************************************************/

uint32_t nubus_vikbw_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_vbl_disable)
	{
		raise_slot_irq();
	}

	auto const vram8 = util::big_endian_cast<uint8_t const>(&m_vram[0]);
	for (int y = 0; y < 768; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1024/8; x++)
		{
			uint8_t const pixels = vram8[(y * 128) + x];

			*scanline++ = m_palette[BIT(pixels, 7)];
			*scanline++ = m_palette[BIT(pixels, 6)];
			*scanline++ = m_palette[BIT(pixels, 5)];
			*scanline++ = m_palette[BIT(pixels, 4)];
			*scanline++ = m_palette[BIT(pixels, 3)];
			*scanline++ = m_palette[BIT(pixels, 2)];
			*scanline++ = m_palette[BIT(pixels, 1)];
			*scanline++ = m_palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

void nubus_vikbw_device::viking_ack_w(uint32_t data)
{
	lower_slot_irq();
}

uint32_t nubus_vikbw_device::viking_ack_r()
{
	return 0;
}

void nubus_vikbw_device::viking_disable_w(uint32_t data)
{
	m_vbl_disable = 1;
}

uint32_t nubus_vikbw_device::viking_enable_r()
{
	m_vbl_disable = 0;
	return 0;
}
