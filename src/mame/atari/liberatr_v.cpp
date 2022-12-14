// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

    video/liberatr.c

  Functions to emulate the video hardware of the machine.

   Liberator's screen is 256 pixels by 256 pixels.  The
     round planet in the middle of the screen is 128 pixels
     tall by 96 equivalent (192 at double pixel rate).  The
     emulator needs to account for the aspect ratio of 4/3
     from the arcade video system in order to make the planet
     appear round.

***************************************************************************/

#include "emu.h"
#include "liberatr.h"


#define NUM_PENS    (0x18)




void liberatr_state::bitmap_xy_w(uint8_t data)
{
	m_videoram[(*m_ycoord << 8) | *m_xcoord] = data & 0xe0;
}


uint8_t liberatr_state::bitmap_xy_r()
{
	return m_videoram[(*m_ycoord << 8) | *m_xcoord];
}


void liberatr_state::bitmap_w(offs_t offset, uint8_t data)
{
	uint8_t x, y;

	m_bitmapram[offset] = data;

	offset += 3;
	x = (offset & 0x3f) << 2;
	y = offset >> 6;

	data = data & 0xe0;

	m_videoram[(y << 8) | x | 0] = data;
	m_videoram[(y << 8) | x | 1] = data;
	m_videoram[(y << 8) | x | 2] = data;
	m_videoram[(y << 8) | x | 3] = data;
}


/********************************************************************************************
  liberatr_init_planet()

  The data for the planet is stored in ROM using a run-length type of encoding.  This
  function does the conversion to the above structures and then a smaller
  structure which is quicker to use in real time.

  Its a multi-step process, reflecting the history of the code.  Not quite as efficient
  as it might be, but this is not realtime stuff, so who cares...
 ********************************************************************************************/

void liberatr_state::init_planet(planet &liberatr_planet, uint8_t *planet_rom)
{
	const uint8_t *const latitude_scale = memregion("user1")->base();
	const uint8_t *const longitude_scale = memregion("user2")->base();

	// for each starting longitude
	for (uint16_t longitude = 0; longitude < 0x100; longitude++)
	{
		planet_frame frame;
		uint16_t total_segment_count = 0;

		// for each latitude
		for (uint8_t latitude = 0; latitude < 0x80; latitude++)
		{
			uint8_t x_array[32], color_array[32], visible_array[32];

			// point to the structure which will hold the data for this line
			planet_frame_line *line = &frame.lines[latitude];

			uint8_t latitude_scale_factor = latitude_scale[latitude];

			// for this latitude, load the 32 segments into the arrays
			for (uint8_t segment = 0; segment < 0x20; segment++)
			{
				uint16_t address;

				// read the planet picture ROM and get the latitude and longitude scaled from the scaling PROMS
				address = (latitude << 5) + segment;
				uint16_t planet_data = (planet_rom[address] << 8) | planet_rom[address + 0x1000];

				uint8_t  color  =  (planet_data >> 8) & 0x0f;
				uint16_t length = ((planet_data << 1) & 0x1fe) + ((planet_data >> 15) & 0x01);


				// scale the longitude limit (adding the starting longitude)
				address = longitude + (length >> 1) + (length & 1);     // shift with rounding
				visible_array[segment] = BIT(address, 8);
				uint8_t longitude_scale_factor;
				if (address & 0x80)
				{
					longitude_scale_factor = 0xff;
				}
				else
				{
					address = ((address & 0x7f) << 1) + (((length & 1) || visible_array[segment]) ? 0 : 1);
					longitude_scale_factor = longitude_scale[address];
				}

				x_array[segment] = (((uint16_t)latitude_scale_factor * (uint16_t)longitude_scale_factor) + 0x80) >> 8;  /* round it */
				color_array[segment] = color;
			}

			// determine which segment is the western horizon and leave 'start_segment' indexing it.
			uint8_t start_segment;
			for (start_segment = 0; start_segment < 0x1f; start_segment++)    // if not found, 'start_segment' = 0x1f
				if (visible_array[start_segment]) break;

			// transfer from the temporary arrays to the structure
			line->max_x = (latitude_scale_factor * 0xc0) >> 8;
			if (line->max_x & 1)
				line->max_x += 1;               // make it even

			/*
			   as part of the quest to reduce memory usage (and to a lesser degree
			     execution time), stitch together segments that have the same color
			*/
			uint8_t segment = start_segment;
			uint8_t segment_count = 0;
			uint8_t i = 0;
			uint8_t x = 0;

			do
			{
				uint8_t color = color_array[segment];
				while (color == color_array[segment])
				{
					x = x_array[segment];
					segment = (segment+1) & 0x1f;
					if (segment == start_segment)
						break;
				}

				line->color_array[i] = color;
				line->x_array[i] = (x > line->max_x) ? line->max_x : x;
				i++;
				segment_count++;
			} while ((i < 32) && (x <= line->max_x));

			total_segment_count += segment_count;
			line->segment_count = segment_count;
		}

		/* now that the all the lines have been processed, and we know how
		   many segments it will take to store the description, allocate the
		   space for it and copy the data to it.
		*/
		liberatr_planet.frames[longitude] = std::make_unique<uint8_t []>(2 * (128 + total_segment_count));
		uint8_t *buffer = liberatr_planet.frames[longitude].get();

		for (uint8_t latitude = 0; latitude < 0x80; latitude++)
		{
			planet_frame_line *line = &frame.lines[latitude];
			uint8_t segment_count = line->segment_count;
			*buffer++ = segment_count;

			/* calculate the bitmap's x coordinate for the western horizon
			   center of bitmap - (the number of planet pixels) / 4 */
			*buffer++ = (m_screen->width() / 2) - ((line->max_x + 2) / 4);

			for (uint8_t i = 0, last_x = 0; i < segment_count; i++)
			{
				uint8_t current_x = (line->x_array[i] + 1) / 2;

				*buffer++ = line->color_array[i];
				*buffer++ = current_x - last_x;

				last_x = current_x;
			}
		}
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void liberatr_state::video_start()
{
	// for each planet in the planet ROMs
	init_planet(m_planets[0], &memregion("gfx1")->base()[0x2000]);
	init_planet(m_planets[1], &memregion("gfx1")->base()[0x0000]);

	save_item(NAME(m_planet_select));
}


void liberatr_state::get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		uint8_t r,g,b;

		/* handle the hardware flip of the bit order from 765 to 576 that
		   hardware does between vram and color ram */
		static const offs_t penmap[] = { 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
									0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
									0x10, 0x12, 0x14, 0x16, 0x11, 0x13, 0x15, 0x17 };

		uint8_t data = m_colorram[i];

		/* scale it from 0x00-0xff */
		r = ((~data >> 3) & 0x07) * 0x24 + 3;  if (r == 3)  r = 0;
		g = ((~data >> 0) & 0x07) * 0x24 + 3;  if (g == 3)  g = 0;
		b = ((~data >> 5) & 0x06) * 0x24 + 3;  if (b == 3)  b = 0;

		pens[penmap[i]] = rgb_t(r, g, b);
	}
}


void liberatr_state::draw_planet(bitmap_rgb32 &bitmap, pen_t *pens)
{
	uint8_t const *buffer = m_planets[m_planet_select].frames[*m_planet_frame].get();

	/* for each latitude */
	for (uint8_t latitude = 0; latitude < 0x80; latitude++)
	{
		/* grab the color value for the base (if any) at this latitude */
		uint8_t const base_color = m_base_ram[latitude >> 3] ^ 0x0f;

		uint8_t const segment_count = *buffer++;
		uint8_t x = *buffer++;
		uint8_t const y = 64 + latitude;

		/* run through the segments, drawing its color until its x_array value comes up. */
		for (uint8_t segment = 0; segment < segment_count; segment++)
		{
			uint8_t color = *buffer++;
			uint8_t segment_length = *buffer++;

			if ((color & 0x0c) == 0x0c)
				color = base_color;

			for (uint8_t i = 0; i < segment_length; i++, x++)
				bitmap.pix(y, x) = pens[color];
		}
	}
}


void liberatr_state::draw_bitmap(bitmap_rgb32 &bitmap, pen_t *pens)
{
	for (offs_t offs = 0; offs < 0x10000; offs++)
	{
		uint8_t const data = m_videoram[offs];

		uint8_t const y = offs >> 8;
		uint8_t const x = offs & 0xff;

		if (data)
			bitmap.pix(y, x) = pens[(data >> 5) | 0x10];
	}
}


uint32_t liberatr_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[NUM_PENS];
	get_pens(pens);

	bitmap.fill(rgb_t::black(), cliprect);
	draw_planet(bitmap, pens);
	draw_bitmap(bitmap, pens);

	return 0;
}
