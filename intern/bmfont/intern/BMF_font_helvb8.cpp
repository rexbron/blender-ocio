/**
 * $Id$
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include "BMF_FontData.h"

#include "BMF_Settings.h"

#if BMF_INCLUDE_HELVB8

static unsigned char bitmap_data[]= {
	0x00,0x80,0x00,0x80,0x80,0x80,0x80,0xa0,
	0xa0,0xa0,0x50,0xf8,0x50,0xf8,0x50,0x40,
	0xe0,0x10,0x60,0x80,0x70,0x20,0x5c,0x54,
	0x2c,0xd0,0xa8,0xe8,0x58,0xb0,0xa8,0x48,
	0xa0,0x40,0x80,0x80,0x80,0x40,0x80,0x80,
	0x80,0x80,0x80,0x40,0x80,0x40,0x40,0x40,
	0x40,0x40,0x80,0x40,0xe0,0x40,0x20,0x20,
	0xf8,0x20,0x20,0x80,0x40,0x40,0xf0,0x80,
	0x80,0x80,0x80,0x80,0x40,0x40,0x40,0x60,
	0x90,0x90,0x90,0x90,0x60,0x40,0x40,0x40,
	0x40,0xc0,0x40,0xf0,0x40,0x20,0x10,0x90,
	0x60,0xc0,0x20,0x20,0xc0,0x20,0xc0,0x20,
	0x20,0xf0,0x60,0x20,0x20,0xc0,0x20,0x20,
	0xc0,0x80,0xe0,0x60,0x90,0x90,0xe0,0x80,
	0x70,0x40,0x40,0x40,0x20,0x10,0xf0,0x60,
	0x90,0x90,0x60,0x90,0x60,0x60,0x10,0x70,
	0x90,0x90,0x60,0x80,0x00,0x00,0x80,0x80,
	0x40,0x40,0x00,0x00,0x40,0x20,0x40,0x80,
	0x40,0x20,0xe0,0x00,0xe0,0x80,0x40,0x20,
	0x40,0x80,0x40,0x00,0x40,0x20,0xc0,0x78,
	0x80,0x9e,0xa5,0x99,0x41,0x3e,0x88,0x88,
	0x70,0x50,0x20,0x20,0xe0,0x90,0x90,0xe0,
	0x90,0xe0,0x70,0x88,0x80,0x80,0x88,0x70,
	0xf0,0x88,0x88,0x88,0x88,0xf0,0xf0,0x80,
	0x80,0xe0,0x80,0xf0,0x80,0x80,0x80,0xe0,
	0x80,0xf0,0x70,0x88,0x88,0x98,0x80,0x70,
	0x88,0x88,0x88,0xf8,0x88,0x88,0x80,0x80,
	0x80,0x80,0x80,0x80,0x40,0xa0,0x20,0x20,
	0x20,0x20,0x90,0x90,0xe0,0xc0,0xa0,0x90,
	0xe0,0x80,0x80,0x80,0x80,0x80,0xa8,0xa8,
	0xa8,0xa8,0xd8,0x88,0x88,0x98,0xa8,0xa8,
	0xc8,0x88,0x70,0x88,0x88,0x88,0x88,0x70,
	0x80,0x80,0xe0,0x90,0x90,0xe0,0x10,0x20,
	0x70,0x88,0x88,0x88,0x88,0x70,0x90,0x90,
	0xe0,0x90,0x90,0xe0,0xe0,0x10,0x10,0xe0,
	0x80,0x70,0x40,0x40,0x40,0x40,0x40,0xe0,
	0x70,0x88,0x88,0x88,0x88,0x88,0x40,0xa0,
	0x90,0x90,0x90,0x90,0x48,0x48,0x6c,0x92,
	0x92,0x92,0x90,0x90,0x60,0x60,0x90,0x90,
	0x20,0x20,0x30,0x48,0x48,0xc8,0xf0,0x80,
	0x40,0x20,0x10,0xf0,0xc0,0x80,0x80,0x80,
	0x80,0x80,0xc0,0x40,0x40,0x40,0x40,0x80,
	0x80,0x80,0xc0,0x40,0x40,0x40,0x40,0x40,
	0xc0,0x88,0x50,0x20,0xf8,0x80,0x80,0x80,
	0xd0,0xa0,0xe0,0x20,0xc0,0xe0,0x90,0x90,
	0x90,0xe0,0x80,0x80,0x60,0x80,0x80,0x80,
	0x60,0x70,0x90,0x90,0x90,0x70,0x10,0x10,
	0x60,0x80,0xe0,0xa0,0x40,0x40,0x40,0x40,
	0x40,0xe0,0x40,0x20,0x60,0x10,0x70,0x90,
	0x90,0x70,0x90,0x90,0x90,0x90,0xe0,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x80,
	0x80,0x40,0x40,0x40,0x40,0x40,0x40,0x00,
	0x40,0xa0,0xa0,0xc0,0xc0,0xa0,0x80,0x80,
	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xa8,
	0xa8,0xa8,0xa8,0xf0,0x90,0x90,0x90,0x90,
	0xe0,0x60,0x90,0x90,0x90,0x60,0x80,0xe0,
	0x90,0x90,0x90,0xe0,0x10,0x70,0x90,0x90,
	0x90,0x70,0x80,0x80,0x80,0xc0,0xa0,0xc0,
	0x20,0x60,0x80,0x60,0x40,0x40,0x40,0x40,
	0xe0,0x40,0x40,0x60,0xa0,0xa0,0xa0,0xa0,
	0x40,0xa0,0x90,0x90,0x90,0x50,0x50,0xa8,
	0xa8,0xa8,0x90,0x90,0x60,0x90,0x90,0x80,
	0x40,0x60,0x90,0x90,0x90,0xe0,0x80,0x40,
	0x20,0xe0,0x20,0x40,0x40,0xc0,0x40,0x40,
	0x20,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
	0x80,0x40,0x40,0x60,0x40,0x40,0x80,0xb0,
	0x48,0x00,0x80,0x80,0x80,0x80,0x80,0x00,
	0x80,0x40,0x40,0xa0,0x80,0xa0,0x40,0x40,
	0xf0,0x40,0x40,0xe0,0x40,0x30,0x88,0x70,
	0x50,0x70,0x88,0x20,0x20,0xf8,0x50,0x88,
	0x80,0x80,0x80,0x00,0x80,0x80,0x80,0xe0,
	0x10,0x30,0x60,0x90,0x60,0x80,0x70,0x90,
	0x78,0x84,0xb4,0xa4,0xb4,0x84,0x78,0xe0,
	0x00,0xe0,0x20,0xc0,0x50,0xa0,0x50,0x10,
	0x10,0xf0,0xc0,0x78,0x84,0xac,0xb4,0xb4,
	0x84,0x78,0xe0,0x40,0xa0,0x40,0xf0,0x00,
	0x20,0xf0,0x20,0xc0,0x80,0x40,0x80,0xc0,
	0x20,0x60,0xe0,0x80,0x40,0x80,0x80,0xe0,
	0xa0,0xa0,0xa0,0x50,0x50,0x50,0x50,0xd0,
	0xd0,0xd0,0x78,0x80,0x80,0x80,0x40,0x40,
	0x40,0xc0,0x40,0xe0,0x00,0xe0,0xa0,0xe0,
	0xa0,0x50,0xa0,0x04,0x5e,0x2c,0x54,0x48,
	0xc4,0x40,0x0e,0x44,0x22,0x5c,0x48,0xc4,
	0x40,0x04,0x5e,0x2c,0xd4,0x28,0x64,0xe0,
	0x60,0x90,0x40,0x20,0x00,0x20,0x88,0x88,
	0x70,0x50,0x20,0x20,0x00,0x20,0x40,0x88,
	0x88,0x70,0x50,0x20,0x20,0x00,0x20,0x10,
	0x88,0x88,0x70,0x50,0x20,0x20,0x00,0x50,
	0x20,0x88,0x88,0x70,0x50,0x20,0x20,0x00,
	0x50,0x28,0x88,0x88,0x70,0x50,0x20,0x20,
	0x00,0x50,0x88,0x88,0x70,0x50,0x20,0x20,
	0x20,0x50,0x20,0x9e,0x90,0x7c,0x50,0x30,
	0x3e,0x80,0x40,0x70,0x88,0x80,0x88,0x88,
	0x70,0xf0,0x80,0x80,0xe0,0x80,0xf0,0x00,
	0x20,0x40,0xf0,0x80,0x80,0xe0,0x80,0xf0,
	0x00,0x40,0x20,0xf0,0x80,0x80,0xe0,0x80,
	0xf0,0x00,0xa0,0x40,0xf0,0x80,0x80,0xe0,
	0x80,0xf0,0x00,0xa0,0x40,0x40,0x40,0x40,
	0x40,0x40,0x00,0x40,0x80,0x80,0x80,0x80,
	0x80,0x80,0x80,0x00,0x80,0x40,0x40,0x40,
	0x40,0x40,0x40,0x40,0x00,0xa0,0x40,0x40,
	0x40,0x40,0x40,0x40,0x40,0x00,0xa0,0x70,
	0x48,0x48,0xe8,0x48,0x70,0x88,0x98,0xa8,
	0xa8,0xc8,0x88,0x00,0x50,0x28,0x70,0x88,
	0x88,0x88,0x88,0x70,0x00,0x20,0x40,0x70,
	0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x10,
	0x70,0x88,0x88,0x88,0x88,0x70,0x00,0x50,
	0x20,0x70,0x88,0x88,0x88,0x88,0x70,0x00,
	0x50,0x28,0x70,0x88,0x88,0x88,0x88,0x70,
	0x00,0x50,0x90,0x60,0x60,0x90,0x80,0xf0,
	0xc8,0xa8,0x98,0x88,0x78,0x08,0x70,0x88,
	0x88,0x88,0x88,0x88,0x00,0x20,0x40,0x70,
	0x88,0x88,0x88,0x88,0x88,0x00,0x20,0x10,
	0x70,0x88,0x88,0x88,0x88,0x88,0x00,0x50,
	0x20,0x70,0x88,0x88,0x88,0x88,0x88,0x00,
	0x50,0x20,0x20,0x30,0x48,0x48,0xc8,0x00,
	0x10,0x08,0x80,0xf0,0x88,0x88,0xf0,0x80,
	0x80,0xa0,0x90,0x90,0xa0,0x90,0x60,0xd0,
	0xa0,0xe0,0x20,0xc0,0x00,0x40,0x80,0xd0,
	0xa0,0xe0,0x20,0xc0,0x00,0x40,0x20,0xd0,
	0xa0,0xe0,0x20,0xc0,0x00,0xa0,0x40,0x68,
	0x50,0x70,0x10,0x60,0x00,0xb0,0x68,0xd0,
	0xa0,0xe0,0x20,0xc0,0x00,0xa0,0xd0,0xa0,
	0xe0,0x20,0xc0,0x40,0xa0,0x40,0xd8,0xa0,
	0xf8,0x28,0xd0,0x80,0x40,0x60,0x80,0x80,
	0x80,0x60,0x60,0x80,0xe0,0xa0,0x40,0x00,
	0x20,0x40,0x60,0x80,0xe0,0xa0,0x40,0x00,
	0x40,0x20,0x60,0x80,0xe0,0xa0,0x40,0x00,
	0xa0,0x40,0x60,0x80,0xe0,0xa0,0x40,0x00,
	0xa0,0x40,0x40,0x40,0x40,0x40,0x00,0x40,
	0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x80,
	0x40,0x40,0x40,0x40,0x40,0x40,0x00,0xa0,
	0x40,0x40,0x40,0x40,0x40,0x40,0x00,0xa0,
	0x60,0x90,0x90,0x90,0x70,0xa0,0x60,0x90,
	0x90,0x90,0x90,0x90,0xe0,0x00,0xa0,0x50,
	0x60,0x90,0x90,0x90,0x60,0x00,0x20,0x40,
	0x60,0x90,0x90,0x90,0x60,0x00,0x20,0x10,
	0x60,0x90,0x90,0x90,0x60,0x00,0xa0,0x40,
	0x60,0x90,0x90,0x90,0x60,0x00,0xa0,0x50,
	0x60,0x90,0x90,0x90,0x60,0x00,0x90,0x20,
	0x00,0xf0,0x00,0x20,0x80,0x70,0x68,0x58,
	0x48,0x3c,0x02,0x60,0xa0,0xa0,0xa0,0xa0,
	0x00,0x40,0x80,0x60,0xa0,0xa0,0xa0,0xa0,
	0x00,0x40,0x20,0x60,0xa0,0xa0,0xa0,0xa0,
	0x00,0xa0,0x40,0x60,0xa0,0xa0,0xa0,0xa0,
	0x00,0xa0,0x80,0x40,0x60,0x90,0x90,0x90,
	0x00,0x20,0x10,0x80,0xe0,0x90,0x90,0x90,
	0xe0,0x80,0x80,0x40,0x60,0x90,0x90,0x90,
	0x00,0x50,
};

BMF_FontData BMF_font_helvb8 = {
	0, -2,
	9, 9,
	{
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0, 0, 0, 0, 8, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{1, 1, 0, 0, 2, 0},
		{1, 6, -1, 0, 2, 1},
		{3, 3, -1, -3, 3, 7},
		{5, 5, 0, 0, 5, 10},
		{4, 7, -1, 1, 5, 15},
		{6, 6, -1, 0, 7, 22},
		{5, 6, -1, 0, 6, 28},
		{1, 3, -1, -3, 2, 34},
		{2, 7, -1, 1, 3, 37},
		{2, 7, -1, 1, 3, 44},
		{3, 3, -1, -2, 3, 51},
		{5, 5, -1, 0, 5, 54},
		{2, 3, 0, 2, 2, 59},
		{4, 1, -2, -2, 6, 62},
		{1, 1, -1, 0, 2, 63},
		{2, 7, -1, 1, 2, 64},
		{4, 6, -1, 0, 5, 71},
		{2, 6, -2, 0, 5, 77},
		{4, 6, -1, 0, 5, 83},
		{3, 6, -2, 0, 5, 89},
		{4, 6, -1, 0, 5, 95},
		{3, 6, -2, 0, 5, 101},
		{4, 6, -1, 0, 5, 107},
		{4, 6, -1, 0, 5, 113},
		{4, 6, -1, 0, 5, 119},
		{4, 6, -1, 0, 5, 125},
		{1, 4, -1, 0, 2, 131},
		{2, 6, 0, 2, 2, 135},
		{3, 5, -1, 0, 5, 141},
		{3, 3, -1, -1, 4, 146},
		{3, 5, -2, 0, 5, 149},
		{3, 5, -2, 0, 5, 154},
		{8, 7, -1, 1, 9, 159},
		{5, 6, -1, 0, 6, 166},
		{4, 6, -2, 0, 6, 172},
		{5, 6, -1, 0, 6, 178},
		{5, 6, -1, 0, 6, 184},
		{4, 6, -2, 0, 6, 190},
		{4, 6, -2, 0, 5, 196},
		{5, 6, -1, 0, 6, 202},
		{5, 6, -1, 0, 6, 208},
		{1, 6, -1, 0, 2, 214},
		{3, 6, -1, 0, 4, 220},
		{4, 6, -2, 0, 6, 226},
		{3, 6, -2, 0, 5, 232},
		{5, 6, -2, 0, 7, 238},
		{5, 6, -1, 0, 6, 244},
		{5, 6, -1, 0, 6, 250},
		{4, 6, -2, 0, 6, 256},
		{5, 8, -1, 2, 6, 262},
		{4, 6, -2, 0, 6, 270},
		{4, 6, -2, 0, 6, 276},
		{3, 6, -1, 0, 4, 282},
		{5, 6, -1, 0, 6, 288},
		{4, 6, -2, 0, 6, 294},
		{7, 6, -1, 0, 7, 300},
		{4, 6, -2, 0, 6, 306},
		{5, 6, -1, 0, 6, 312},
		{4, 6, -2, 0, 6, 318},
		{2, 7, -1, 1, 2, 324},
		{2, 7, 0, 1, 2, 331},
		{2, 7, 0, 1, 2, 338},
		{5, 3, 0, -2, 5, 345},
		{5, 1, 0, 1, 5, 348},
		{1, 3, -1, -3, 2, 349},
		{4, 5, -1, 0, 4, 352},
		{4, 7, -1, 0, 5, 357},
		{3, 5, -1, 0, 4, 364},
		{4, 7, -1, 0, 5, 369},
		{3, 5, -1, 0, 4, 376},
		{3, 7, -1, 0, 3, 381},
		{4, 6, -1, 1, 5, 388},
		{4, 7, -1, 0, 5, 394},
		{1, 7, -1, 0, 2, 401},
		{2, 9, 0, 2, 2, 408},
		{3, 7, -1, 0, 4, 417},
		{1, 7, -1, 0, 2, 424},
		{5, 5, -1, 0, 6, 431},
		{4, 5, -1, 0, 5, 436},
		{4, 5, -1, 0, 5, 441},
		{4, 6, -1, 1, 5, 446},
		{4, 6, -1, 1, 5, 452},
		{3, 5, -1, 0, 3, 458},
		{3, 5, -1, 0, 4, 463},
		{3, 7, -1, 0, 3, 468},
		{3, 5, -1, 0, 4, 475},
		{4, 5, -1, 0, 5, 480},
		{5, 5, -1, 0, 6, 485},
		{4, 5, -1, 0, 5, 490},
		{4, 6, -1, 1, 4, 495},
		{3, 5, -1, 0, 4, 501},
		{3, 7, 0, 1, 2, 506},
		{1, 7, -1, 1, 2, 513},
		{3, 7, 0, 1, 2, 520},
		{5, 2, -1, -2, 6, 527},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{0,0,0,0,0, -1},
		{1, 1, 0, 0, 2, 529},
		{1, 7, -1, 2, 2, 530},
		{3, 7, -1, 1, 5, 537},
		{4, 6, -1, 0, 5, 544},
		{5, 5, 0, 0, 4, 550},
		{5, 5, -1, 0, 6, 555},
		{1, 7, -1, 1, 2, 560},
		{4, 8, -1, 2, 5, 567},
		{4, 1, 0, -5, 2, 575},
		{6, 7, -1, 1, 7, 576},
		{3, 5, 0, -1, 3, 583},
		{4, 3, -1, -1, 5, 588},
		{4, 3, -1, -1, 6, 591},
		{2, 1, 0, -2, 3, 594},
		{6, 7, -1, 1, 7, 595},
		{3, 1, 0, -5, 2, 602},
		{3, 3, -1, -3, 3, 603},
		{4, 5, -1, 0, 5, 606},
		{2, 4, -1, -2, 2, 611},
		{3, 4, 0, -2, 2, 615},
		{2, 2, 0, -4, 2, 619},
		{3, 6, -1, 2, 4, 621},
		{5, 8, 0, 2, 5, 627},
		{1, 2, -1, -1, 2, 635},
		{2, 2, 0, 2, 2, 637},
		{2, 4, 0, -2, 2, 639},
		{3, 5, 0, -1, 3, 643},
		{4, 3, -1, -1, 5, 648},
		{7, 7, 0, 1, 7, 651},
		{7, 7, 0, 1, 7, 658},
		{7, 7, 0, 1, 7, 665},
		{4, 6, -1, 1, 5, 672},
		{5, 9, -1, 0, 6, 678},
		{5, 9, -1, 0, 6, 687},
		{5, 9, -1, 0, 6, 696},
		{5, 9, -1, 0, 6, 705},
		{5, 8, -1, 0, 6, 714},
		{5, 9, -1, 0, 6, 722},
		{7, 6, -1, 0, 8, 731},
		{5, 8, -1, 2, 6, 737},
		{4, 9, -2, 0, 6, 745},
		{4, 9, -2, 0, 6, 754},
		{4, 9, -2, 0, 6, 763},
		{4, 8, -2, 0, 6, 772},
		{2, 9, 0, 0, 2, 780},
		{2, 9, -1, 0, 2, 789},
		{3, 9, 0, 0, 2, 798},
		{3, 8, 0, 0, 2, 807},
		{5, 6, -1, 0, 6, 815},
		{5, 9, -1, 0, 6, 821},
		{5, 9, -1, 0, 6, 830},
		{5, 9, -1, 0, 6, 839},
		{5, 9, -1, 0, 6, 848},
		{5, 9, -1, 0, 6, 857},
		{5, 8, -1, 0, 6, 866},
		{4, 4, -1, 0, 5, 874},
		{5, 8, -1, 1, 6, 878},
		{5, 9, -1, 0, 6, 886},
		{5, 9, -1, 0, 6, 895},
		{5, 9, -1, 0, 6, 904},
		{5, 8, -1, 0, 6, 913},
		{5, 9, -1, 0, 6, 921},
		{5, 6, -1, 0, 6, 930},
		{4, 7, -2, 1, 6, 936},
		{4, 8, -1, 0, 4, 943},
		{4, 8, -1, 0, 4, 951},
		{4, 8, -1, 0, 4, 959},
		{5, 8, 0, 0, 4, 967},
		{4, 7, -1, 0, 4, 975},
		{4, 8, -1, 0, 4, 982},
		{5, 5, -1, 0, 6, 990},
		{3, 7, -1, 2, 4, 995},
		{3, 8, -1, 0, 4, 1002},
		{3, 8, -1, 0, 4, 1010},
		{3, 8, -1, 0, 4, 1018},
		{3, 7, -1, 0, 4, 1026},
		{2, 8, 0, 0, 2, 1033},
		{2, 8, -1, 0, 2, 1041},
		{3, 8, 0, 0, 2, 1049},
		{3, 7, 0, 0, 2, 1057},
		{4, 8, -1, 0, 5, 1064},
		{4, 8, -1, 0, 5, 1072},
		{4, 8, -1, 0, 5, 1080},
		{4, 8, -1, 0, 5, 1088},
		{4, 8, -1, 0, 5, 1096},
		{4, 8, -1, 0, 5, 1104},
		{4, 7, -1, 0, 5, 1112},
		{4, 5, -1, 0, 5, 1119},
		{7, 7, 0, 1, 5, 1124},
		{3, 8, -1, 0, 4, 1131},
		{3, 8, -1, 0, 4, 1139},
		{3, 8, -1, 0, 4, 1147},
		{3, 7, -1, 0, 4, 1155},
		{4, 9, -1, 1, 4, 1162},
		{4, 7, -1, 1, 5, 1171},
		{4, 8, -1, 1, 4, 1178},
	},
	bitmap_data
};

#endif

