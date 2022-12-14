// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Arcade Classics hardware (prototypes)

    Note: this video hardware has some similarities to Shuuz & company
    The sprite offset registers are stored to 3EFF80

****************************************************************************/


#include "emu.h"
#include "atarimo.h"
#include "arcadecl.h"

/*************************************
 *
 *  Video system start
 *
 *************************************/

const atari_motion_objects_config arcadecl_state::s_mob_config =
{
	0,                  /* index to which gfx system */
	1,                  /* number of motion object banks */
	1,                  /* are the entries linked? */
	0,                  /* are the entries split? */
	0,                  /* render in reverse order? */
	0,                  /* render in swapped X/Y order? */
	0,                  /* does the neighbor bit affect the next object? */
	0,                  /* pixels per SLIP entry (0 for no-slip) */
	0,                  /* pixel offset for SLIPs */
	0,                  /* maximum number of links to visit/scanline (0=all) */

	0x100,              /* base palette entry */
	0x100,              /* maximum number of colors */
	0,                  /* transparent pen index */

	{{ 0x00ff,0,0,0 }}, /* mask for the link */
	{{ 0,0x7fff,0,0 }}, /* mask for the code index */
	{{ 0,0,0x000f,0 }}, /* mask for the color */
	{{ 0,0,0xff80,0 }}, /* mask for the X position */
	{{ 0,0,0,0xff80 }}, /* mask for the Y position */
	{{ 0,0,0,0x0070 }}, /* mask for the width, in tiles*/
	{{ 0,0,0,0x0007 }}, /* mask for the height, in tiles */
	{{ 0,0x8000,0,0 }}, /* mask for the horizontal flip */
	{{ 0 }},            /* mask for the vertical flip */
	{{ 0 }},            /* mask for the priority */
	{{ 0 }},            /* mask for the neighbor */
	{{ 0 }},            /* mask for absolute coordinates */

	{{ 0 }},            /* mask for the special value */
	0                  /* resulting value to indicate "special" */
};

void arcadecl_state::video_start()
{
	m_mob->set_scroll(-12, 0x110);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

uint32_t arcadecl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// start drawing
	m_mob->draw_async(cliprect);

	// draw the playfield
	sparkz_state::screen_update(screen, bitmap, cliprect);

	// draw and merge the MO
	bitmap_ind16 &mobitmap = m_mob->bitmap();
	for (const sparse_dirty_rect *rect = m_mob->first_dirty_rect(cliprect); rect != nullptr; rect = rect->next())
		for (int y = rect->top(); y <= rect->bottom(); y++)
		{
			uint16_t const *const mo = &mobitmap.pix(y);
			uint16_t *const pf = &bitmap.pix(y);
			for (int x = rect->left(); x <= rect->right(); x++)
				if (mo[x] != 0xffff)
				{
					// not yet verified
					pf[x] = mo[x];
				}
		}

	return 0;
}



/*************************************
 *
 *  Bitmap rendering
 *
 *************************************/

uint32_t sparkz_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// update any dirty scanlines
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const uint16_t *const src = &m_bitmap[256 * y];
		uint16_t *const dst = &bitmap.pix(y);

		/* regenerate the line */
		for (int x = cliprect.left() & ~1; x <= cliprect.right(); x += 2)
		{
			int const bits = src[(x - 8) / 2];
			dst[x + 0] = bits >> 8;
			dst[x + 1] = bits & 0xff;
		}
	}

	return 0;
}
