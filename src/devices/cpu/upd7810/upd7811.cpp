// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller

/*

    uPD7811 - variant of uPD7810 with internal ROM

*/

#include "emu.h"
#include "upd7811.h"

/*
    the MODE pins can cause this to work with external ROM instead
    todo: document MODE pins

    M0  M1

    0   0 -
    0   1 -
    1   0 -
    1   1 -

*/

DEFINE_DEVICE_TYPE(UPD7811,  upd7811_device,  "upd7811",  "NEC uPD7811")
DEFINE_DEVICE_TYPE(UPD78C11, upd78c11_device, "upd78c11", "NEC uPD78C11")

upd7811_device::upd7811_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7810_device(mconfig, UPD7811, tag, owner, clock, address_map_constructor(FUNC(upd7811_device::upd_internal_4096_rom_256_ram_map), this))
{
}

upd78c11_device::upd78c11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd78c10_device(mconfig, UPD78C11, tag, owner, clock, address_map_constructor(FUNC(upd78c11_device::upd_internal_4096_rom_256_ram_map), this))
{
}
