// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo

/* Ramtek - Star Cruiser */

#include "emu.h"
#include "starcrus.h"
#include "sound/samples.h"
#include "screen.h"

/* The collision detection techniques use in this driver
   are well explained in the comments in the sprint2 driver */

void starcrus_state::s1_x_w(uint8_t data){ m_s1_x = data^0xff; }
void starcrus_state::s1_y_w(uint8_t data){ m_s1_y = data^0xff; }
void starcrus_state::s2_x_w(uint8_t data){ m_s2_x = data^0xff; }
void starcrus_state::s2_y_w(uint8_t data){ m_s2_y = data^0xff; }
void starcrus_state::p1_x_w(uint8_t data){ m_p1_x = data^0xff; }
void starcrus_state::p1_y_w(uint8_t data){ m_p1_y = data^0xff; }
void starcrus_state::p2_x_w(uint8_t data){ m_p2_x = data^0xff; }
void starcrus_state::p2_y_w(uint8_t data){ m_p2_y = data^0xff; }

void starcrus_state::video_start()
{
	m_ship1_vid = std::make_unique<bitmap_ind16>(16, 16);
	m_ship2_vid = std::make_unique<bitmap_ind16>(16, 16);

	m_proj1_vid = std::make_unique<bitmap_ind16>(16, 16);
	m_proj2_vid = std::make_unique<bitmap_ind16>(16, 16);

	save_item(NAME(m_s1_x));
	save_item(NAME(m_s1_y));
	save_item(NAME(m_s2_x));
	save_item(NAME(m_s2_y));
	save_item(NAME(m_p1_x));
	save_item(NAME(m_p1_y));
	save_item(NAME(m_p2_x));
	save_item(NAME(m_p2_y));
	save_item(NAME(m_p1_sprite));
	save_item(NAME(m_p2_sprite));
	save_item(NAME(m_s1_sprite));
	save_item(NAME(m_s2_sprite));
	save_item(NAME(m_collision_reg));
}

void starcrus_state::ship_parm_1_w(uint8_t data)
{
	m_s1_sprite = data & 0x1f;
	m_engine[0]->write_line(BIT(data, 5));
}

void starcrus_state::ship_parm_2_w(uint8_t data)
{
	m_s2_sprite = data & 0x1f;
	m_led = !BIT(data, 7); /* game over lamp */
	machine().bookkeeping().coin_counter_w(0, ((data & 0x40) >> 6) ^ 0x01);    /* coin counter */
	m_engine[1]->write_line(BIT(data, 5));
}

void starcrus_state::proj_parm_1_w(uint8_t data)
{
	m_p1_sprite = data & 0x0f;
	m_launch[0]->write_line(BIT(data, 5));
	m_explode[0]->write_line(BIT(data, 4));
}

void starcrus_state::proj_parm_2_w(uint8_t data)
{
	m_p2_sprite = data & 0x0f;
	m_launch[1]->write_line(BIT(data, 5));
	m_explode[1]->write_line(BIT(data, 4));
}

int starcrus_state::collision_check_s1s2()
{
	rectangle clip(0, 15, 0, 15);

	m_ship1_vid->fill(0, clip);
	m_ship2_vid->fill(0, clip);

	/* origin is with respect to ship1 */

	int org_x = m_s1_x;
	int org_y = m_s1_y;

	/* Draw ship 1 */

	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->opaque(*m_ship1_vid,
			clip,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x - org_x, m_s1_y - org_y);

	/* Draw ship 2 */

	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->opaque(*m_ship2_vid,
			clip,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x - org_x, m_s2_y - org_y);

	/* Now check for collisions */
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
		/* Condition 1 - ship 1 = ship 2 */
		if ((m_ship1_vid->pix(sy, sx) == 1) && (m_ship2_vid->pix(sy, sx) == 1))
			return 1;

	return 0;
}

int starcrus_state::collision_check_p1p2()
{
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ( ((m_p1_sprite & 0x08) == 0) &&
			((m_p2_sprite & 0x08) == 0) )
	{
		return 0;
	}

	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	/* origin is with respect to proj1 */

	int org_x = m_p1_x;
	int org_y = m_p1_y;

	if (m_p1_sprite & 0x08)  /* if p1 is a projectile */
	{
		/* Draw score/projectile 1 */

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08)  /* if p2 is a projectile */
	{
		/* Draw score/projectile 2 */

		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	/* Now check for collisions */
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			/* Condition 1 - proj 1 = proj 2 */
			if ((m_proj1_vid->pix(sy, sx) == 1) && (m_proj2_vid->pix(sy, sx) == 1))
				return 1;

	return 0;
}

int starcrus_state::collision_check_s1p1p2()
{
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ((m_p1_sprite & 0x08) == 0 && (m_p2_sprite & 0x08) == 0)
	{
		return 0;
	}

	m_ship1_vid->fill(0, clip);
	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	/* origin is with respect to ship1 */

	int org_x = m_s1_x;
	int org_y = m_s1_y;

	/* Draw ship 1 */

	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->opaque(*m_ship1_vid,
			clip,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x - org_x, m_s1_y - org_y);

	if (m_p1_sprite & 0x08)  /* if p1 is a projectile */
	{
		/* Draw projectile 1 */

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08)  /* if p2 is a projectile */
	{
		/* Draw projectile 2 */

		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	/* Now check for collisions */
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			if (m_ship1_vid->pix(sy, sx) == 1)
			{
				/* Condition 1 - ship 1 = proj 1 */
				if (m_proj1_vid->pix(sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 1 = proj 2 */
				if (m_proj2_vid->pix(sy, sx) == 1)
					return 1;
			}

	return 0;
}

int starcrus_state::collision_check_s2p1p2()
{
	rectangle clip(0, 15, 0, 15);

	/* if both are scores, return */
	if ((m_p1_sprite & 0x08) == 0 && (m_p2_sprite & 0x08) == 0)
	{
		return 0;
	}

	m_ship2_vid->fill(0, clip);
	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	/* origin is with respect to ship2 */

	int org_x = m_s2_x;
	int org_y = m_s2_y;

	/* Draw ship 2 */

	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->opaque(*m_ship2_vid,
			clip,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x - org_x, m_s2_y - org_y);

	if (m_p1_sprite & 0x08)  /* if p1 is a projectile */
	{
		/* Draw projectile 1 */

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08)  /* if p2 is a projectile */
	{
		/* Draw projectile 2 */
		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	/* Now check for collisions */
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			if (m_ship2_vid->pix(sy, sx) == 1)
			{
				/* Condition 1 - ship 2 = proj 1 */
				if (m_proj1_vid->pix(sy, sx) == 1)
					return 1;
				/* Condition 2 - ship 2 = proj 2 */
				if (m_proj2_vid->pix(sy, sx) == 1)
					return 1;
			}

	return 0;
}

uint32_t starcrus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	/* Draw ship 1 */
	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->transpen(bitmap,
			cliprect,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x, m_s1_y,
			0);

	/* Draw ship 2 */
	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->transpen(bitmap,
			cliprect,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x, m_s2_y,
			0);

	/* Draw score/projectile 1 */
	m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->transpen(bitmap,
			cliprect,
			(m_p1_sprite & 0x03) ^ 0x03,
			0,
			0, 0,
			m_p1_x, m_p1_y,
			0);

	/* Draw score/projectile 2 */
	m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->transpen(bitmap,
			cliprect,
			(m_p2_sprite & 0x03) ^ 0x03,
			0,
			0, 0,
			m_p2_x, m_p2_y,
			0);

	/* Collision detection */
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		m_collision_reg = 0x00;

		/* Check for collisions between ship1 and ship2 */
		if (collision_check_s1s2())
		{
			m_collision_reg |= 0x08;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s1p1p2())
		{
			m_collision_reg |= 0x02;
		}
		/* Check for collisions between ship1 and projectiles */
		if (collision_check_s2p1p2())
		{
			m_collision_reg |= 0x01;
		}
		/* Check for collisions between ship1 and projectiles */
		/* Note: I don't think this is used by the game */
		if (collision_check_p1p2())
		{
			m_collision_reg |= 0x04;
		}
	}

	return 0;
}

uint8_t starcrus_state::coll_det_r()
{
	return m_collision_reg ^ 0xff;
}
