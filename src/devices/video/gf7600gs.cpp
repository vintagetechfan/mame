// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "gf7600gs.h"

DEFINE_DEVICE_TYPE(GEFORCE_7600GS, geforce_7600gs_device, "geforce_7600gs", "NVIDIA GeForce 7600GS")

void geforce_7600gs_device::map1(address_map &map)
{
}

void geforce_7600gs_device::map2(address_map &map)
{
}

void geforce_7600gs_device::map3(address_map &map)
{
}

geforce_7600gs_device::geforce_7600gs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, GEFORCE_7600GS, tag, owner, clock)
	, m_vga(*this, finder_base::DUMMY_TAG)
{
}

void geforce_7600gs_device::device_start()
{
	pci_device::device_start();
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map1));
	add_map(256*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map2));
	add_map( 16*1024*1024, M_MEM, FUNC(geforce_7600gs_device::map3));
	add_rom_from_region();
}

void geforce_7600gs_device::legacy_memory_map(address_map &map)
{
	map(0xa0000, 0xbffff).rw(FUNC(geforce_7600gs_device::vram_r), FUNC(geforce_7600gs_device::vram_w));
}

void geforce_7600gs_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03bf).rw(FUNC(geforce_7600gs_device::vga_3b0_r), FUNC(geforce_7600gs_device::vga_3b0_w));
	map(0x03c0, 0x03cf).rw(FUNC(geforce_7600gs_device::vga_3c0_r), FUNC(geforce_7600gs_device::vga_3c0_w));
	map(0x03d0, 0x03df).rw(FUNC(geforce_7600gs_device::vga_3d0_r), FUNC(geforce_7600gs_device::vga_3d0_w));
}

void geforce_7600gs_device::map_extra(
	uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space
)
{
	// command extensions
	// VGA control - forward legacy VGA addresses to AGP
	// TODO: doc implies that is unaffected by base and limit?
//	if (BIT(bridge_control, 3))
	{
		memory_space->install_device(0, 0xfffff, *this, &geforce_7600gs_device::legacy_memory_map);
		io_space->install_device(0, 0x0fff, *this, &geforce_7600gs_device::legacy_io_map);
	}

	// TODO: ISA control
	// forward to "primary PCI" (host & LPC?) for A8 or A9 blocks for each 1KB blocks in I/O spaces,
	// (i.e. $100-$3ff, $500-$7ff, $900-$bff etc.)
	// even if I/O range is inside base and limits
//	if (BIT(bridge_control, 2))
	// ...
}

void geforce_7600gs_device::device_reset()
{
	pci_device::device_reset();
}

uint8_t geforce_7600gs_device::vram_r(offs_t offset)
{
	return downcast<vga_device *>(m_vga.target())->mem_r(offset);
}

void geforce_7600gs_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<vga_device *>(m_vga.target())->mem_w(offset, data);
}

u32 geforce_7600gs_device::vga_3b0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<vga_device *>(m_vga.target())->port_03b0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3b0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<vga_device *>(m_vga.target())->port_03b0_w(offset * 4 + 3, data >> 24);
}


u32 geforce_7600gs_device::vga_3c0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<vga_device *>(m_vga.target())->port_03c0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<vga_device *>(m_vga.target())->port_03c0_w(offset * 4 + 3, data >> 24);
}

u32 geforce_7600gs_device::vga_3d0_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;
	if (ACCESSING_BITS_0_7)
		result |= downcast<vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 0) << 0;
	if (ACCESSING_BITS_8_15)
		result |= downcast<vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 1) << 8;
	if (ACCESSING_BITS_16_23)
		result |= downcast<vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 2) << 16;
	if (ACCESSING_BITS_24_31)
		result |= downcast<vga_device *>(m_vga.target())->port_03d0_r(offset * 4 + 3) << 24;
	return result;
}

void geforce_7600gs_device::vga_3d0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		downcast<vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 0, data >> 0);
	if (ACCESSING_BITS_8_15)
		downcast<vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 1, data >> 8);
	if (ACCESSING_BITS_16_23)
		downcast<vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 2, data >> 16);
	if (ACCESSING_BITS_24_31)
		downcast<vga_device *>(m_vga.target())->port_03d0_w(offset * 4 + 3, data >> 24);
}
