/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstring>
#include <algorithm>

namespace {

#define TO5BIT(c8) (((c8) * 0x1F * 2 + 0xFF) / (0xFF*2))
#define PACK15_1(rgb24) (TO5BIT((rgb24) & 0xFF) << 10 | TO5BIT((rgb24) >> 8 & 0xFF) << 5 | TO5BIT((rgb24) >> 16 & 0xFF))
#define PACK15_4(c0, c1, c2, c3) \
	PACK15_1(c0), PACK15_1(c1), PACK15_1(c2), PACK15_1(c3)

// Hardware Palettes (TheWolfBunny)
static const unsigned short gbdmg[] = {	// Game Boy | DMG-001
	PACK15_4(0x7F860F, 0x577C44, 0x365D48, 0x2A453B),
	PACK15_4(0x7F860F, 0x577C44, 0x365D48, 0x2A453B),
	PACK15_4(0x7F860F, 0x577C44, 0x365D48, 0x2A453B)
};

static const unsigned short gbpoc[] = {	// Game Boy Pocket | MGB-001
	PACK15_4(0xC4CFA1, 0x8B956D, 0x4D533C, 0x1F1F1F),
	PACK15_4(0xC4CFA1, 0x8B956D, 0x4D533C, 0x1F1F1F),
	PACK15_4(0xC4CFA1, 0x8B956D, 0x4D533C, 0x1F1F1F)
};

static const unsigned short gblit[] = {	// Game Boy Light | MGB-101
	PACK15_4(0x00B581, 0x009A71, 0x00694A, 0x004F3B),
	PACK15_4(0x00B581, 0x009A71, 0x00694A, 0x004F3B),
	PACK15_4(0x00B581, 0x009A71, 0x00694A, 0x004F3B)
};

// Game Boy Color Palettes
static const unsigned short p005[] = {	// Green | Right
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000)
};

static const unsigned short p006[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000)
};

static const unsigned short p007[] = {	// Orange | Down + A
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000)
};

static const unsigned short p008[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000)
};

static const unsigned short p012[] = {	// Brown | Up
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p013[] = {	// Reverse | Right + B
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF)
};

static const unsigned short p016[] = {	// Gray | Left + B
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000),
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000),
	PACK15_4(0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000)
};

static const unsigned short p017[] = {	// Pastel Mix | Down
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000),
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000),
	PACK15_4(0xFFFFA5, 0xFF9494, 0x9494FF, 0x000000)
};

static const unsigned short p01B[] = {
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFCE00, 0x9C6300, 0x000000)
};

static const unsigned short p100[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000)
};

static const unsigned short p10B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p10D[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000)
};

static const unsigned short p110[] = {
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p11C[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000)
};

static const unsigned short p20B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p20C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000)
};

static const unsigned short p300[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000)
};

static const unsigned short p304[] = {
	PACK15_4(0xFFFFFF, 0x7BFF00, 0xB57300, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p305[] = {
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p306[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p308[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000)
};

static const unsigned short p30A[] = {
	PACK15_4(0xB5B5FF, 0xFFFF94, 0xAD5A42, 0x000000),
	PACK15_4(0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A),
	PACK15_4(0x000000, 0xFFFFFF, 0xFF8484, 0x943A3A)
};

static const unsigned short p30C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000)
};

static const unsigned short p30D[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p30E[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p30F[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p312[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p319[] = {	// Dark Brown | Up + B
	PACK15_4(0xFFE6C5, 0xCE9C84, 0x846B29, 0x5A3108),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p31C[] = {	// Dark Green (GBC Default) | Right + A
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p405[] = {
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x52FF00, 0xFF4200, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p406[] = {
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF9C00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF )
};

static const unsigned short p407[] = {
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF00, 0xFF0000, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p500[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p501[] = {
	PACK15_4(0xFFFF9C, 0x94B5FF, 0x639473, 0x003A3A),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p502[] = {
	PACK15_4(0x6BFF00, 0xFFFFFF, 0xFF524A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p503[] = {
	PACK15_4(0x52DE00, 0xFF8400, 0xFFFF00, 0xFFFFFF),
	PACK15_4(0xFFFFFF, 0xFFFFFF, 0x63A5FF, 0x0000FF),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000)
};

static const unsigned short p508[] = {
	PACK15_4(0xA59CFF, 0xFFFF00, 0x006300, 0x000000),
	PACK15_4(0xFF6352, 0xD60000, 0x630000, 0x000000),
	PACK15_4(0x0000FF, 0xFFFFFF, 0xFFFF7B, 0x0084FF)
};

static const unsigned short p509[] = {
	PACK15_4(0xFFFFCE, 0x63EFEF, 0x9C8431, 0x5A5A5A),
	PACK15_4(0xFFFFFF, 0xFF7300, 0x944200, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p50B[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFFF7B, 0x0084FF, 0xFF0000)
};

static const unsigned short p50C[] = {
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFC542, 0xFFD600, 0x943A00, 0x4A0000),
	PACK15_4(0xFFFFFF, 0x5ABDFF, 0xFF0000, 0x0000FF)
};

static const unsigned short p50D[] = {	// Dark Blue | Left + A
	PACK15_4(0xFFFFFF, 0x8C8CDE, 0x52528C, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p50E[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p50F[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p510[] = {	// Red | Up + A
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p511[] = {
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x00FF00, 0x318400, 0x004A00),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p512[] = {
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p514[] = {
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFF00, 0xFF0000, 0x630000, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p515[] = {
	PACK15_4(0xFFFFFF, 0xADAD84, 0x42737B, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

static const unsigned short p518[] = {	// Blue | Left
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p51A[] = {	// Yellow | Down + B
	PACK15_4(0xFFFFFF, 0xFFFF00, 0x7B4A00, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000),
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x008400, 0x000000)
};

static const unsigned short p51C[] = {
	PACK15_4(0xFFFFFF, 0x7BFF31, 0x0063C5, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF8484, 0x943A3A, 0x000000),
	PACK15_4(0xFFFFFF, 0x63A5FF, 0x0000FF, 0x000000)
};

// Super Game Boy Palettes
static const unsigned short sgb1A[] = {	// 1-A (SGB Default | 7047-0470-4704)
	PACK15_4(0xFFEFCE, 0xDE944A, 0xAD2921, 0x311852),
	PACK15_4(0xFFEFCE, 0xDE944A, 0xAD2921, 0x311852),
	PACK15_4(0xFFEFCE, 0xDE944A, 0xAD2921, 0x311852)
};

static const unsigned short sgb1B[] = {	// 7057-0570-5705
	PACK15_4(0xDEDEC6, 0xCEB573, 0xB55210, 0x000000),
	PACK15_4(0xDEDEC6, 0xCEB573, 0xB55210, 0x000000),
	PACK15_4(0xDEDEC6, 0xCEB573, 0xB55210, 0x000000)
};

static const unsigned short sgb1C[] = {	// 7067-0670-6706
	PACK15_4(0xFFC6FF, 0xEF9C52, 0x9C3963, 0x39399C),
	PACK15_4(0xFFC6FF, 0xEF9C52, 0x9C3963, 0x39399C),
	PACK15_4(0xFFC6FF, 0xEF9C52, 0x9C3963, 0x39399C)
};

static const unsigned short sgb1D[] = {	// 7077-0770-7707
	PACK15_4(0xFFFFAD, 0xC6844A, 0xFF0000, 0x521800),
	PACK15_4(0xFFFFAD, 0xC6844A, 0xFF0000, 0x521800),
	PACK15_4(0xFFFFAD, 0xC6844A, 0xFF0000, 0x521800)
};

static const unsigned short sgb1E[] = {	// 7087-0870-8708
	PACK15_4(0xFFDEB5, 0x7BC67B, 0x6B8C42, 0x5A3921),
	PACK15_4(0xFFDEB5, 0x7BC67B, 0x6B8C42, 0x5A3921),
	PACK15_4(0xFFDEB5, 0x7BC67B, 0x6B8C42, 0x5A3921)
};

static const unsigned short sgb1F[] = {	// 7097-0970-9709
	PACK15_4(0xDEEFFF, 0xE78C52, 0xAD0000, 0x004210),
	PACK15_4(0xDEEFFF, 0xE78C52, 0xAD0000, 0x004210),
	PACK15_4(0xDEEFFF, 0xE78C52, 0xAD0000, 0x004210)
};

static const unsigned short sgb1G[] = {	// 7107-1071-0710
	PACK15_4(0x000052, 0x00A5EF, 0x7B7B00, 0xFFFF5A),
	PACK15_4(0x000052, 0x00A5EF, 0x7B7B00, 0xFFFF5A),
	PACK15_4(0x000052, 0x00A5EF, 0x7B7B00, 0xFFFF5A)
};

static const unsigned short sgb1H[] = {	// 7117-1171-1711
	PACK15_4(0xFFEFE7, 0xFFBD8C, 0x844200, 0x311800),
	PACK15_4(0xFFEFE7, 0xFFBD8C, 0x844200, 0x311800),
	PACK15_4(0xFFEFE7, 0xFFBD8C, 0x844200, 0x311800)
};

static const unsigned short sgb2A[] = {	// 7127-1271-2712
	PACK15_4(0xF7CEA5, 0xC68C4A, 0x297B00, 0x000000),
	PACK15_4(0xF7CEA5, 0xC68C4A, 0x297B00, 0x000000),
	PACK15_4(0xF7CEA5, 0xC68C4A, 0x297B00, 0x000000)
};

static const unsigned short sgb2B[] = {	// 7137-1371-3713
	PACK15_4(0xFFFFFF, 0xFFEF52, 0xFF3100, 0x52005A),
	PACK15_4(0xFFFFFF, 0xFFEF52, 0xFF3100, 0x52005A),
	PACK15_4(0xFFFFFF, 0xFFEF52, 0xFF3100, 0x52005A)
};

static const unsigned short sgb2C[] = {	// 7147-1471-4714
	PACK15_4(0xFFC6FF, 0xEF8C8C, 0x7B31EF, 0x29299C),
	PACK15_4(0xFFC6FF, 0xEF8C8C, 0x7B31EF, 0x29299C),
	PACK15_4(0xFFC6FF, 0xEF8C8C, 0x7B31EF, 0x29299C)
};

static const unsigned short sgb2D[] = {	// 7157-1571-5715
	PACK15_4(0xFFFFA5, 0x00FF00, 0xFF3100, 0x000052),
	PACK15_4(0xFFFFA5, 0x00FF00, 0xFF3100, 0x000052),
	PACK15_4(0xFFFFA5, 0x00FF00, 0xFF3100, 0x000052)
};

static const unsigned short sgb2E[] = {	// 7167-1671-6716
	PACK15_4(0xFFCE84, 0x94B5E7, 0x291063, 0x100810),
	PACK15_4(0xFFCE84, 0x94B5E7, 0x291063, 0x100810),
	PACK15_4(0xFFCE84, 0x94B5E7, 0x291063, 0x100810)
};

static const unsigned short sgb2F[] = {	// 7177-1771-7717
	PACK15_4(0xD6FFFF, 0xFF9452, 0xA50000, 0x180000),
	PACK15_4(0xD6FFFF, 0xFF9452, 0xA50000, 0x180000),
	PACK15_4(0xD6FFFF, 0xFF9452, 0xA50000, 0x180000)
};

static const unsigned short sgb2G[] = {	// 7187-1871-8718
	PACK15_4(0x6BBD39, 0xE75242, 0xE7BD84, 0x001800),
	PACK15_4(0x6BBD39, 0xE75242, 0xE7BD84, 0x001800),
	PACK15_4(0x6BBD39, 0xE75242, 0xE7BD84, 0x001800)
};

static const unsigned short sgb2H[] = {	// 2-H (SGB Grayscale | 7197-1971-9719)
	PACK15_4(0xFFFFFF, 0xBDBDBD, 0x737373, 0x000000),
	PACK15_4(0xFFFFFF, 0xBDBDBD, 0x737373, 0x000000),
	PACK15_4(0xFFFFFF, 0xBDBDBD, 0x737373, 0x000000)
};

static const unsigned short sgb3A[] = {	// 7207-2072-0720
	PACK15_4(0xFFD69C, 0x73C6C6, 0xFF6329, 0x314A63),
	PACK15_4(0xFFD69C, 0x73C6C6, 0xFF6329, 0x314A63),
	PACK15_4(0xFFD69C, 0x73C6C6, 0xFF6329, 0x314A63)
};

static const unsigned short sgb3B[] = {	// 7217-2172-1721
	PACK15_4(0xDEDEC6, 0xE78421, 0x005200, 0x001010),
	PACK15_4(0xDEDEC6, 0xE78421, 0x005200, 0x001010),
	PACK15_4(0xDEDEC6, 0xE78421, 0x005200, 0x001010)
};

static const unsigned short sgb3C[] = {	// 7227-2272-2722
	PACK15_4(0xE7ADCE, 0xFFFF7B, 0x00BDFF, 0x21215A),
	PACK15_4(0xE7ADCE, 0xFFFF7B, 0x00BDFF, 0x21215A),
	PACK15_4(0xE7ADCE, 0xFFFF7B, 0x00BDFF, 0x21215A)
};

static const unsigned short sgb3D[] = {	// 7237-2372-3723
	PACK15_4(0xF7FFBD, 0xE7AD7B, 0x08CE00, 0x000000),
	PACK15_4(0xF7FFBD, 0xE7AD7B, 0x08CE00, 0x000000),
	PACK15_4(0xF7FFBD, 0xE7AD7B, 0x08CE00, 0x000000)
};

static const unsigned short sgb3E[] = {	// 7247-2472-4724
	PACK15_4(0xFFFFC6, 0xE7B56B, 0xB57B21, 0x524A73),
	PACK15_4(0xFFFFC6, 0xE7B56B, 0xB57B21, 0x524A73),
	PACK15_4(0xFFFFC6, 0xE7B56B, 0xB57B21, 0x524A73)
};

static const unsigned short sgb3F[] = {	// 7257-2572-5725
	PACK15_4(0x7B7BCE, 0xFF6BFF, 0xFFD600, 0x424242),
	PACK15_4(0x7B7BCE, 0xFF6BFF, 0xFFD600, 0x424242),
	PACK15_4(0x7B7BCE, 0xFF6BFF, 0xFFD600, 0x424242)
};

static const unsigned short sgb3G[] = {	// 7267-2672-6726
	PACK15_4(0x63DE52, 0xFFFFFF, 0xCE3139, 0x390000),
	PACK15_4(0x63DE52, 0xFFFFFF, 0xCE3139, 0x390000),
	PACK15_4(0x63DE52, 0xFFFFFF, 0xCE3139, 0x390000)
};

static const unsigned short sgb3H[] = {	// 7277-2772-7727
	PACK15_4(0xE7FFA5, 0x7BCE39, 0x4A8C18, 0x081800),
	PACK15_4(0xE7FFA5, 0x7BCE39, 0x4A8C18, 0x081800),
	PACK15_4(0xE7FFA5, 0x7BCE39, 0x4A8C18, 0x081800)
};

static const unsigned short sgb4A[] = {	// 7287-2872-8728
	PACK15_4(0xF7AD6B, 0x7BADFF, 0xD600D6, 0x00007B),
	PACK15_4(0xF7AD6B, 0x7BADFF, 0xD600D6, 0x00007B),
	PACK15_4(0xF7AD6B, 0x7BADFF, 0xD600D6, 0x00007B)
};

static const unsigned short sgb4B[] = {	// 7297-2972-9729
	PACK15_4(0xF7EFF7, 0xEFA563, 0x427B39, 0x180808),
	PACK15_4(0xF7EFF7, 0xEFA563, 0x427B39, 0x180808),
	PACK15_4(0xF7EFF7, 0xEFA563, 0x427B39, 0x180808)
};

static const unsigned short sgb4C[] = {	// 7307-3073-0730
	PACK15_4(0xFFE7E7, 0xDEA5D6, 0x9CA5E7, 0x080000),
	PACK15_4(0xFFE7E7, 0xDEA5D6, 0x9CA5E7, 0x080000),
	PACK15_4(0xFFE7E7, 0xDEA5D6, 0x9CA5E7, 0x080000)
};

static const unsigned short sgb4D[] = {	// 7317-3173-1731
	PACK15_4(0xFFFFBD, 0x94CECE, 0x4A6B7B, 0x08214A),
	PACK15_4(0xFFFFBD, 0x94CECE, 0x4A6B7B, 0x08214A),
	PACK15_4(0xFFFFBD, 0x94CECE, 0x4A6B7B, 0x08214A)
};

static const unsigned short sgb4E[] = {	// 7327-3273-2732
	PACK15_4(0xFFDEAD, 0xE7AD7B, 0x7B5A8C, 0x002131),
	PACK15_4(0xFFDEAD, 0xE7AD7B, 0x7B5A8C, 0x002131),
	PACK15_4(0xFFDEAD, 0xE7AD7B, 0x7B5A8C, 0x002131)
};

static const unsigned short sgb4F[] = {	// 7337-3373-3733
	PACK15_4(0xBDD6D6, 0xDE84DE, 0x8400A5, 0x390000),
	PACK15_4(0xBDD6D6, 0xDE84DE, 0x8400A5, 0x390000),
	PACK15_4(0xBDD6D6, 0xDE84DE, 0x8400A5, 0x390000)
};

static const unsigned short sgb4G[] = {	// 7347-3473-4734
	PACK15_4(0xB5E718, 0xBD215A, 0x291000, 0x008463),
	PACK15_4(0xB5E718, 0xBD215A, 0x291000, 0x008463),
	PACK15_4(0xB5E718, 0xBD215A, 0x291000, 0x008463)
};

static const unsigned short sgb4H[] = {	// 4-H (SGB Greenscale | RGB208 | 7357-3573-5735)
	PACK15_4(0xFFFFCE, 0xBDC65A, 0x848C42, 0x425229),
	PACK15_4(0xFFFFCE, 0xBDC65A, 0x848C42, 0x425229),
	PACK15_4(0xFFFFCE, 0xBDC65A, 0x848C42, 0x425229)
};

// Virtual Console
static const unsigned short vcGreen[] = {	// DMG_COLOR
	PACK15_4(0xBDFF21, 0x9CEF29, 0x5A8C42, 0x4A4A4A),
	PACK15_4(0xBDFF21, 0x9CEF29, 0x5A8C42, 0x4A4A4A),
	PACK15_4(0xBDFF21, 0x9CEF29, 0x5A8C42, 0x4A4A4A)
};

static const unsigned short vcGreenAlt[] = {	// inspired by Dream Land GB (Super Smash Bros. Ultimate)
	PACK15_4(0xF6FF70, 0xB9D03A, 0x788B1D, 0x48530E),
	PACK15_4(0xF6FF70, 0xB9D03A, 0x788B1D, 0x48530E),
	PACK15_4(0xF6FF70, 0xB9D03A, 0x788B1D, 0x48530E)
};

static const unsigned short vcGray[] = {	// DMG_COLOR_GRAY
	PACK15_4(0xCECEAD, 0xA5A58C, 0x6B6B52, 0x292919),
	PACK15_4(0xCECEAD, 0xA5A58C, 0x6B6B52, 0x292919),
	PACK15_4(0xCECEAD, 0xA5A58C, 0x6B6B52, 0x292919)
};

static const unsigned short vcGrayAlt[] = {	// DMG_COLOR_COLOR (Kirby's Dream Collection)
	PACK15_4(0xBDBD9C, 0x8C8C73, 0x5A5A4A, 0x313119),
	PACK15_4(0xBDBD9C, 0x8C8C73, 0x5A5A4A, 0x313119),
	PACK15_4(0xBDBD9C, 0x8C8C73, 0x5A5A4A, 0x313119)
};

static const unsigned short vcSaGa[] = {	// Collection of SaGa: Final Fantasy Legend
	PACK15_4(0xB2C0A8, 0x769A67, 0x345D51, 0x041820),
	PACK15_4(0xB2C0A8, 0x769A67, 0x345D51, 0x041820),
	PACK15_4(0xB2C0A8, 0x769A67, 0x345D51, 0x041820)
};

// Gunpei Yokoi Palettes
static const unsigned short pDemoVision[] = {	// Demo Vision (.db $38,$28,$18,$09, ran through NES Classic Mini)
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x0D410E),
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x0D410E),
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x0D410E)
};

static const unsigned short pWideBoyGrayscale[] = {	// Wide-Boy Grayscale (.db $30,$10,$00,$0F, ran through NES Classic Mini)
	PACK15_4(0xFCFCFC, 0xAAAAAA, 0x616161, 0x000000),
	PACK15_4(0xFCFCFC, 0xAAAAAA, 0x616161, 0x000000),
	PACK15_4(0xFCFCFC, 0xAAAAAA, 0x616161, 0x000000)
};

static const unsigned short pWideBoyGreenscale[] = {	// Wide-Boy Greenscale (.db $38,$28,$18,$08, ran through NES Classic Mini)
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x21350C),
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x21350C),
	PACK15_4(0xDADB9C, 0xA8B100, 0x5D6913, 0x21350C)
};

static const unsigned short twbGW[] = {	// Game & Watch Classic LCD
	PACK15_4(0xC6CBAD, 0x979B84, 0x545F51, 0x22231E),
	PACK15_4(0xC6CBAD, 0x979B84, 0x545F51, 0x22231E),
	PACK15_4(0xC6CBAD, 0x979B84, 0x545F51, 0x22231E)
};

static const unsigned short twbSWJ[] = {	// WonderSwan Monochrome
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D),
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D),
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D)
};

static const unsigned short twbVUE[] = {	// Virtual Boy
	PACK15_4(0xFF0000, 0xAA0000, 0x550000, 0x000000),
	PACK15_4(0xFF0000, 0xAA0000, 0x550000, 0x000000),
	PACK15_4(0xFF0000, 0xAA0000, 0x550000, 0x000000)
};

static const unsigned short twbWB[] = {	// Giga Kiwi DMG (Wide-Boy / Kiosk)
	PACK15_4(0xD0E040, 0xA0A830, 0x607028, 0x384828),
	PACK15_4(0xD0E040, 0xA0A830, 0x607028, 0x384828),
	PACK15_4(0xD0E040, 0xA0A830, 0x607028, 0x384828)
};

// Additional Super Game Boy Enhanced Palettes
static const unsigned short sgbACR[] = {	// Conker's Pocket Tales
	PACK15_4(0xD6DE63, 0x8CA500, 0x3A5200, 0x000000),
	PACK15_4(0xD6DE63, 0x8CA500, 0x3A5200, 0x000000),
	PACK15_4(0xD6DE63, 0x8CA500, 0x3A5200, 0x000000)
};

static const unsigned short sgbAP[] = {	// Pokémon <https://github.com/pret/pokered/blob/master/data/sgb/sgb_palettes.asm>
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010),
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010),
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010)
};

static const unsigned short sgbAPS[] = {	// Pokémon Yellow <https://github.com/pret/pokeyellow/blob/master/data/sgb/sgb_palettes.asm>
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131),
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131),
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131)
};

static const unsigned short sgbAW[] = {	// Alleyway (nensondubois improvement | 2997-1273-2810)
	PACK15_4(0x4A634A, 0xC68C4A, 0x7B5A8C, 0x311852),
	PACK15_4(0x4A634A, 0xC68C4A, 0x7B5A8C, 0x311852),
	PACK15_4(0x4A634A, 0xC68C4A, 0x7B5A8C, 0x311852)
};

static const unsigned short sgbAYL[] = {	// Tetris Attack (nensondubois improvement)
	PACK15_4(0xFFFFE6, 0x52F708, 0xB510EF, 0x94104A),
	PACK15_4(0xFFFFE6, 0x52F708, 0xB510EF, 0x94104A),
	PACK15_4(0xFFFFE6, 0x52F708, 0xB510EF, 0x94104A)
};

static const unsigned short sgbQD[] = {	// Donkey Kong '94 (TheWolfBunny)
	PACK15_4(0xC0F0F8, 0xD89078, 0x805850, 0x204008),
	PACK15_4(0xC0F0F8, 0xD89078, 0x805850, 0x204008),
	PACK15_4(0xC0F0F8, 0xD89078, 0x805850, 0x204008)
};

static const unsigned short sgbQP[] = {	// Cool Spot (Nintendo Power #63 | 2421-9219-9243)
	PACK15_4(0xFFFFFF, 0xFF0000, 0x009C00, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF0000, 0x009C00, 0x000000),
	PACK15_4(0xFFFFFF, 0xFF0000, 0x009C00, 0x000000)
};

static const unsigned short sgbWM[] = {	// Mega Man World V
	PACK15_4(0xD6D6D6, 0x73A5E6, 0x426B94, 0x082131),
	PACK15_4(0xD6D6D6, 0x73A5E6, 0x426B94, 0x082131),
	PACK15_4(0xD6D6D6, 0x73A5E6, 0x426B94, 0x082131)
};

static const unsigned short sgbYT[] = {	// Donkey Kong Land
	PACK15_4(0xA5B500, 0x738C00, 0x4A6B00, 0x214200),
	PACK15_4(0xA5B500, 0x738C00, 0x4A6B00, 0x214200),
	PACK15_4(0xA5B500, 0x738C00, 0x4A6B00, 0x214200)
};

static const unsigned short sgbZL[] = {	// Link's Awakening DX
	PACK15_4(0xFFFFB5, 0x7BC57B, 0x6B8C42, 0x5A3A21),
	PACK15_4(0xFFFFB5, 0x7BC57B, 0x6B8C42, 0x5A3A21),
	PACK15_4(0xFFFFB5, 0x7BC57B, 0x6B8C42, 0x5A3A21)
};

// Extra Palettes
static const unsigned short pExt1[] = {	// DMG-esque
	PACK15_4(0xE5EA93, 0xC4C641, 0x5E7C39, 0x21442A),
	PACK15_4(0xE5EA93, 0xC4C641, 0x5E7C39, 0x21442A),
	PACK15_4(0xE5EA93, 0xC4C641, 0x5E7C39, 0x21442A)
};

static const unsigned short pExt2[] = {
	PACK15_4(0xF8F8F8, 0x83C656, 0x187890, 0x000000),
	PACK15_4(0xF8F8F8, 0xE18096, 0x7F3848, 0x000000),
	PACK15_4(0xF8F8F8, 0xFFDA03, 0x958401, 0x000000)
};

static const unsigned short pExt3[] = {
	PACK15_4(0xF8F8F8, 0xA59E8C, 0x49726C, 0x000000),
	PACK15_4(0xF8F8F8, 0xE49685, 0x6E241E, 0x000000),
	PACK15_4(0xF8F8F8, 0xD7543C, 0x7D3023, 0x000000)
};

static const unsigned short pGBStudio[] = {	// GB Studio
	PACK15_4(0xE8F8E0, 0xB0F088, 0x509878, 0x202850),
	PACK15_4(0xE8F8E0, 0xB0F088, 0x509878, 0x202850),
	PACK15_4(0xE8F8E0, 0xB0F088, 0x509878, 0x202850)
};

static const unsigned short pSML2013[] = {	// Arne's Unnamed Graphics Hack (Super Mario Land)
	PACK15_4(0xF8F8F8, 0xC8B060, 0x486820, 0x283050),
	PACK15_4(0x000000, 0xE0C048, 0xB05030, 0x283018),
	PACK15_4(0x000000, 0xE0C048, 0xB05030, 0x283018)
};

// GBC Special, from Goomba Color & BGB
static const unsigned short bgbT3[] = {	// Adventure Island
	PACK15_4(0xFFFFFF, 0x9CB5FF, 0x319400, 0x000000),
	PACK15_4(0xFFFFDE, 0xEFC673, 0xFF6352, 0x000029),
	PACK15_4(0xFFFFFF, 0xE7A5A5, 0x7B2929, 0x420000)
};
static const unsigned short bgbGQ[] = {	// Adventure Island II: Aliens in Paradise
	PACK15_4(0xFFFFFF, 0xF7EF75, 0x296BBD, 0x000000),
	PACK15_4(0xFFFFDE, 0xEFC673, 0xFF6352, 0x000029),
	PACK15_4(0xFFFFFF, 0xE7A5A5, 0x7B2929, 0x420000)
};
static const unsigned short bgbBA[] = { // Batman: The Video Game
	PACK15_4(0xFFF7EF, 0xC89088, 0x845044, 0x421000),
	PACK15_4(0xFFFFFF, 0xA5A5C6, 0x52528C, 0x00005A),
	PACK15_4(0xFFFFFF, 0xADB5BD, 0x5A6B7B, 0x082142)
};
static const unsigned short bgbB5[] = { // Batman: Return of the Joker
	PACK15_4(0xFFFFFF, 0xA5ADBD, 0x525A7B, 0x001039),
	PACK15_4(0xFFFFFF, 0xA5ADBD, 0x525A7B, 0x001039),
	PACK15_4(0xFFFFFF, 0xADB5BD, 0x5A6B7B, 0x082142)
};
static const unsigned short bgbB0[] = {	// Bionic Commando
	PACK15_4(0xEFF7FF, 0xCEB5AD, 0xC62129, 0x390000),
	PACK15_4(0xFFFFFF, 0xFFAD84, 0x5A3900, 0x000000),
	PACK15_4(0xEFEFEF, 0xADA59C, 0x6B5A5A, 0x421008)
};
static const unsigned short bgbCV[] = {	// The Castlevania Adventure
	PACK15_4(0xD6D6E7, 0x8CA5B5, 0x42526B, 0x001018),
	PACK15_4(0xFFFFFF, 0xFFE784, 0xFF5242, 0x5A0000),
	PACK15_4(0xFFFFFF, 0xF7EFCE, 0xF7DE9C, 0xF7B56B)
};
static const unsigned short bgbUF[] = {	// The Smurfs
	PACK15_4(0xEFFFF1, 0x7BE75A, 0x3B9C18, 0x1D2A1F),
	PACK15_4(0xEFEFFF, 0x5A8CE7, 0x184A9C, 0x000000),
	PACK15_4(0xFFEFEF, 0xDC714B, 0xAD1018, 0x000000)
};

#undef PACK15_4
#undef PACK15_1
#undef TO5BIT

struct GbcPaletteEntry { const char *title; const unsigned short *p; };

static const GbcPaletteEntry gbcDirPalettes[] = {
	{ "Demo Vision", pDemoVision },
	{ "GB - DMG", gbdmg },	// GB Pea Soup Green
	{ "GB - Light", gblit },	// GB Light Aquamarine
	{ "GB - MGB", gbpoc },	// GB Pocket Newsprint
	{ "GB Studio", pGBStudio },
	{ "GBC - Dark Green", p31C },	// Right + A (default GBC)
	{ "GBC - Gray", p016 },	// Left + B
	{ "GW Classic LCD", twbGW },
	{ "Pocket Tales", sgbACR },	// Conker's Pocket Tales
	{ "Portable Power", sgbYT },	// Donkey Kong Land
	{ "SGB - 1A", sgb1A },	// 1-A (default SGB)
	{ "SGB - 2H", sgb2H },	// SGB Grayscale
	{ "SGB - 4H", sgb4H },	// SGB Greenscale
	{ "VC - Gray", vcGrayAlt },	// Virtual Console Grayscale (Kirby's Dream Collection)
	{ "VC - Green", vcGreen },	// Virtual Console Greenscale
	{ "Virtual Boy", twbVUE },
	{ "Wide-Boy", twbWB },	// Giga Kiwi DMG
	{ "WonderSwan", twbSWJ }
};

static const GbcPaletteEntry gbcTitlePalettes[] = {
	{ "ALLEY WAY", p008 },
	{ "ARNE'SMARIOLAND", pSML2013 },	// homebrew (Arne's Unnamed Graphics Hack)
	{ "ASTEROIDS", p30E },	// unofficial ("ASTEROIDS/MISCMD" alt.)
	{ "ASTEROIDS/MISCMD", p30E },
	{ "ATOMIC PUNK", p30F },	// unofficial ("DYNABLASTER" alt.)
	{ "BA.TOSHINDEN", p50F },
	{ "BALLOON KID", p006 },
	{ "BASEBALL", p503 },
	{ "BATTLETOADS", p312 },	// unofficial
	{ "BOMBER BOY", p30F },	// unofficial ("DYNABLASTER" alt.)
	{ "BOMBERMAN GB", p31C },	// unofficial ("WARIO BLAST" alt.)
	{ "BOY AND BLOB GB1", p512 },
	{ "BOY AND BLOB GB2", p512 },
	{ "BT2RAGNAROKWORLD", p312 },
	{ "CENTIPEDE", p31C },	// unofficial ("MILLI/CENTI/PEDE" alt.)
	{ "DEFENDER/JOUST", p50F },
	{ "DMG FOOTBALL", p30E },
	{ "DONKEY KONG", p306 },
	{ "DONKEYKONGLAND", p50C },
	{ "DONKEYKONGLAND 2", p50C },
	{ "DONKEYKONGLAND 3", p50C },
	{ "DONKEYKONGLAND95", p501 },
	{ "DR.MARIO", p20B },
	{ "DYNABLASTER", p30F },
	{ "ELITE SOCCER", p502 },	// unofficial ("SOCCER" alt.)
	{ "EMPIRE STRIKES", p512 },	// unofficial
	{ "F1RACE", p012 },
	{ "FUSHIGINA BULOBI", p512 },	// unofficial ("BOY AND BLOB GB" alt.)
	{ "G&W GALLERY", p304 },
	{ "GALAGA&GALAXIAN", p013 },
	{ "Game and Watch 2", p304 },
	{ "GAME&WATCH", p012 },
	{ "GAMEBOY GALLERY", p304 },
	{ "GAMEBOY GALLERY2", p304 },
	{ "GAMEBOYCAMERA", p110 },	// unofficial improvement, really p01B
	{ "GAMEBOYCAMERA G", p01B },	// unofficial improvement, really p110
	{ "GBWARS", p500 },
	{ "GBWARST", p500 },	// unofficial ("GBWARS" alt.)
	{ "GOLF", p30E },
	{ "HOSHINOKA-BI", p508 },
	{ "JAMES  BOND  007", p11C },
	{ "KAERUNOTAMENI", p10D },
	{ "KEN GRIFFEY JR", p31C },
	{ "KID ICARUS", p30D },
	{ "KILLERINSTINCT95", p50D },
	{ "KINGOFTHEZOO", p30F },
	{ "KIRAKIRA KIDS", p012 },
	{ "KIRBY BLOCKBALL", p508 },
	{ "KIRBY DREAM LAND", p508 },
	{ "KIRBY'S PINBALL", p308 },
	{ "KIRBY2", p508 },
	{ "KOF HEATOFBATTLE", p50F },	// unofficial
	{ "LOLO", p50F },	// unofficial ("LOLO2" alt.)
	{ "LOLO2", p50F },
	{ "LUNAR CHASE", p016 },	// unofficial ("X" alt.)
	{ "MAGNETIC SOCCER", p50E },
	{ "MANSELL", p012 },
	{ "MARIO & YOSHI", p305 },
	{ "MARIO'S PICROSS", p012 },
	{ "MARIOLAND2", p509 },
	{ "MEGA MAN 2", p50F },
	{ "MEGAMAN", p50F },
	{ "MEGAMAN3", p50F },
	{ "MEGAMAN4", p50F },	// unofficial
	{ "MEGAMAN5", p50F },	// unofficial
	{ "METROID2", p514 },
	{ "MILLI/CENTI/PEDE", p31C },
	{ "MISSILE COMMAND", p30E },	// unofficial ("ASTEROIDS/MISCMD" alt.)
	{ "MOGURANYA", p300 },
	{ "MYSTIC QUEST", p50E },
	{ "NETTOU KOF 95", p50F },
	{ "NETTOU KOF 96", p50F },	// unofficial
	{ "NETTOU TOSHINDEN", p50F },	// unofficial ("BA.TOSHINDEN" alt.)
	{ "NEW CHESSMASTER", p30F },
	{ "NIGEL MANSELL", p012 },	// unofficial ("MANSELL" alt.)
	{ "OTHELLO", p50E },
	{ "PAC-IN-TIME", p51C },
	{ "PENGUIN WARS", p30F },	// unofficial ("KINGOFTHEZOO" alt.)
	{ "PENGUINKUNWARSVS", p30F },	// unofficial ("KINGOFTHEZOO" alt.)
	{ "PICROSS 2", p012 },
	{ "PINOCCHIO", p20C },
	{ "POCKETCAMERA", p110 },	// unofficial ("GAMEBOYCAMERA" alt.)
	{ "POKEBOM", p30C },
	{ "POKEMON BLUE", p10B },
	{ "POKEMON GREEN", p11C },
	{ "POKEMON RED", p110 },
	{ "POKEMON YELLOW", p007 },
	{ "QIX", p407 },
	{ "RADARMISSION", p100 },
	{ "ROCKMAN WORLD", p50F },	// unofficial ("MEGAMAN" alt.)
	{ "ROCKMAN WORLD2", p50F },	// unofficial ("MEGA MAN 2" alt.)
	{ "ROCKMANWORLD3", p50F },	// unofficial ("MEGAMAN3" alt.)
	{ "ROCKMANWORLD4", p50F },	// unofficial
	{ "ROCKMANWORLD5", p50F },	// unofficial
	{ "SEIKEN DENSETSU", p50E },
	{ "SOCCER", p502 },
	{ "SOLARSTRIKER", p013 },
	{ "SPACE INVADERS", p013 },
	{ "STAR STACKER", p012 },
	{ "STAR WARS", p512 },
	{ "STAR WARS-NOA", p512 },
	{ "STREET FIGHTER 2", p50F },
	{ "SUPER BOMBLISS", p006 },	// unofficial ("TETRIS BLAST" alt.)
	{ "SUPER MARIOLAND", p30A },
	{ "SUPER RC PRO-AM", p50F },
	{ "SUPERDONKEYKONG", p501 },
	{ "SUPERMARIOLAND3", p500 },
	{ "TENNIS", p502 },
	{ "TETRIS", p007 },
	{ "TETRIS ATTACK", p405 },
	{ "TETRIS BLAST", p006 },
	{ "TETRIS FLASH", p407 },
	{ "TETRIS PLUS", p31C },
	{ "TETRIS2", p407 },
	{ "THE CHESSMASTER", p30F },
	{ "TOPRANKINGTENNIS", p502 },
	{ "TOPRANKTENNIS", p502 },
	{ "TOY STORY", p30E },
	{ "TRIP WORLD", p500 },	// unofficial
	{ "VEGAS STAKES", p50E },
	{ "WARIO BLAST", p31C },
	{ "WARIOLAND2", p515 },
	{ "WAVERACE", p50B },
	{ "WORLD CUP", p30E },
	{ "WORLD STRIKER", p502 },	// unofficial ("SOCCER" alt.)
	{ "X", p016 },
	{ "YAKUMAN", p012 },
	{ "YOSHI'S COOKIE", p406 },
	{ "YOSSY NO COOKIE", p406 },
	{ "YOSSY NO PANEPON", p405 },
	{ "YOSSY NO TAMAGO", p305 },
	{ "ZELDA", p511 }
};

static const GbcPaletteEntry sgbTitlePalettes[] = {
	{ "ALLEY WAY", sgbAW },	// unofficial improvement by nensondubois, really sgb3F
	{ "ARNE'SMARIOLAND", sgb1F },	// homebrew (Arne's Unnamed Graphics Hack)
	{ "BALLôôN KID", sgb1A },
	{ "BALLOON KID", sgb1A },	// unofficial ("BALLôôN KID" alt.)
	{ "BASEBALL", sgb2G },
	{ "CASINO FUNPAK", sgb1A },	// unofficial (Nintendo Power Issue #67)
	{ "CONTRA ALIEN WAR", sgb1F },	// unofficial (Nintendo Power Issue #66)
	{ "CONTRA SPIRITS", sgb1F },	// unofficial ("CONTRA ALIEN WAR" alt.)
	{ "COOL SPOT", sgbQP },	// unofficial (Nintendo Power Issue #63)
	{ "CUTTHROAT ISLAND", sgb3E },	// unofficial (Nintendo Power Issue #82)
	{ "DMG FOOTBALL", sgb4B },	// unofficial (nensondubois)
	{ "DONKEY KONG", sgbQD },	// unofficial (TheWolfBunny - Donkey Kong '94')
	{ "DR.MARIO", sgb3B },
	{ "F1RACE", sgb4B },	// unofficial improvement by nensondubois, really sgb4F
	{ "FRANK THOMAS BB", sgb1B },	// unofficial (Nintendo Power Issue #80)
	{ "GBWARS", sgb3E },
	{ "GBWARST", sgb3E },	// unofficial ("GBWARS" alt.)
	{ "GOLF", sgb3H },
	{ "HOSHINOKA-BI", sgb2C },
	{ "ITCHY & SCRATCHY", sgb4B },	// unofficial (Nintendo Power Issue #63)
	{ "JEOPARDY", sgb2F },	// unofficial ("JEOPARDY SPORTS" alt.)
	{ "JEOPARDY SPORTS", sgb2F },	// unofficial (Nintendo Power Issue #62)
	{ "JUNGLE BOOK", sgb4B },	// unofficial (Nintendo Power Issue #62)
	{ "KAERUNOTAMENI", sgb2A },
	{ "KID ICARUS", sgb2F },
	{ "KIRBY BLOCKBALL", sgb4D },	// unofficial (Nintendo Power Issue #83)
	{ "KIRBY DREAM LAND", sgb2C },
	{ "KIRBY'S PINBALL", sgb1C },
	{ "LUNAR CHASE", sgb4D },	// unofficial ("X" alt.)
	{ "MARIO & YOSHI", sgb2D },
	{ "MARIOLAND2", sgb3D },
	{ "MEGA MAN 2", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN3", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN4", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN5", sgbWM },	// unofficial (TheWolfBunny - Mega Man World V)
	{ "METROID2", sgb4G },
	{ "MORTAL KOMBAT", sgb4D },	// unofficial ("MORTAL KOMBAT II" alt.)
	{ "MORTAL KOMBAT 3", sgb1B },	// unofficial (Nintendo Power Issue #79)
	{ "MORTAL KOMBAT II", sgb4D },	// unofficial (Nintendo Power Issue #65)
	{ "MORTALKOMBAT DUO", sgb4D },	// unofficial ("MORTAL KOMBAT II" alt.)
	{ "MORTALKOMBATI&II", sgb4D },	// unofficial ("MORTAL KOMBAT II" alt.)
	{ "NBA JAM", sgb2F },	// unofficial (Nintendo Power Issue #68)
	{ "NBA JAM TE", sgb2F },	// unofficial (Nintendo Power Issue #68)
	{ "PLAT JEOPARDY!", sgb2F },	// unofficial ("JEOPARDY SPORTS" alt.)
	{ "POCAHONTAS", sgb4D },	// unofficial (Nintendo Power Issue #83)
	{ "POKEMON BLUE", sgbAP },	// unofficial (SGB - MEWMON)
	{ "POKEMON GREEN", sgbAP },	// unofficial (SGB - MEWMON)
	{ "POKEMON RED", sgbAP },	// unofficial (SGB - MEWMON)
	{ "POKEMON YELLOW", sgbAPS },	// unofficial (SGB - MEWMON)
	{ "PROBOTECTOR 2", sgb1F },	// unofficial ("CONTRA ALIEN WAR" alt.)
	{ "QIX", sgb4A },
	{ "RADARMISSION", sgb4B},	// unofficial (nensondubois)
	{ "ROCKMAN WORLD", sgbWM },	// unofficial (SGB)
	{ "ROCKMAN WORLD2", sgbWM },	// unofficial (SGB)
	{ "ROCKMANWORLD3", sgbWM },	// unofficial (SGB)
	{ "ROCKMANWORLD4", sgbWM },	// unofficial (SGB)
	{ "ROCKMANWORLD5", sgbWM },	// unofficial (TheWolfBunny - Rockman World V)
	{ "RVT", sgb4B },	// unofficial (Nintendo Power Issue #63 - Robocop Versus The Terminator)
	{ "SAGA", vcSaGa },	// unofficial (Collection of SaGa: Final Fantasy Legend)
	{ "SAGA2", vcSaGa },	// unofficial (Collection of SaGa: Final Fantasy Legend)
	{ "SAGA3", vcSaGa },	// unofficial (Collection of SaGa: Final Fantasy Legend)
	{ "SOLARSTRIKER", sgb1G },
	{ "SPACE INVADERS", sgb4D },	// unofficial (Nintendo Power Issue #62)
	{ "SUPER MARIOLAND", sgb1F },
	{ "SUPERMARIOLAND3", sgb1B },
	{ "TARZAN", sgb2A },	// unofficial (Nintendo Power Issue #62)
	{ "TAZ-MANIA", sgb1A },	// unofficial (Nintendo Power Issue #64)
	{ "TEEN JEOPARDY!", sgb2F },	// unofficial ("JEOPARDY SPORTS" alt.)
	{ "TENNIS", sgb3G },
	{ "TETRIS", sgb3A },
	{ "TETRIS ATTACK", sgbAYL },	// unofficial (nensondubois)
	{ "TETRIS FLASH", sgb2B },	// unofficial ("TETRIS2" alt.)
	{ "TETRIS2", sgb2B },	// unofficial (nensondubois)
	{ "THE GETAWAY", sgb1B },	// unofficial (Nintendo Power Issue #80)
	{ "TOPRANKINGTENNIS", sgb4B },	// unofficial (nensondubois)
	{ "TOPRANKTENNIS", sgb4B },	// unofficial ("TOPRANKINGTENNIS" alt.)
	{ "WAVERACE", sgb4C },	// unofficial (nensondubois)
	{ "WORLD CUP", sgb4H },	// unofficial (nensondubois)
	{ "X", sgb4D },
	{ "YAKUMAN", sgb3C },
	{ "YOGIS GOLDRUSH", sgb3B },	// unofficial (Nintendo Power Issue #65)
	{ "YOSHI'S COOKIE", sgb1D },
	{ "YOSSY NO COOKIE", sgb1D },
	{ "YOSSY NO PANEPON", sgbAYL },	// unofficial "Tetris Attack" alt. (nensondubois)
	{ "YOSSY NO TAMAGO", sgb2D },
	{ "ZELDA", sgbZL }	// unofficial (TheWolfBunny - The Legend of Zelda: Link's Awakening DX)
};

static inline std::size_t gbcDirPalettesSize() { return (sizeof gbcDirPalettes) / (sizeof gbcDirPalettes[0]); }
static inline const struct GbcPaletteEntry * gbcDirPalettesEnd() { return gbcDirPalettes + gbcDirPalettesSize(); }
static inline std::size_t gbcTitlePalettesSize() { return (sizeof gbcTitlePalettes) / (sizeof gbcTitlePalettes[0]); }
static inline const struct GbcPaletteEntry * gbcTitlePalettesEnd() { return gbcTitlePalettes + gbcTitlePalettesSize(); }

static inline std::size_t sgbTitlePalettesSize() { return (sizeof sgbTitlePalettes) / (sizeof sgbTitlePalettes[0]); }
static inline const struct GbcPaletteEntry * sgbTitlePalettesEnd() { return sgbTitlePalettes + sgbTitlePalettesSize(); }

struct GbcPaletteEntryLess {
	bool operator()(const GbcPaletteEntry &lhs, const char *const rhstitle) const {
		return std::strcmp(lhs.title, rhstitle) < 0;
	}
};

static const unsigned short * findGbcDirPal(const char *const title) {
	const GbcPaletteEntry *const r = std::lower_bound(gbcDirPalettes, gbcDirPalettesEnd(), title, GbcPaletteEntryLess());
	return r < gbcDirPalettesEnd() && !std::strcmp(r->title, title) ? r->p : 0;
}

static const unsigned short * findGbcTitlePal(const char *const title) {
	const GbcPaletteEntry *const r = std::lower_bound(gbcTitlePalettes, gbcTitlePalettesEnd(), title, GbcPaletteEntryLess());
	return r < gbcTitlePalettesEnd() && !std::strcmp(r->title, title) ? r->p : 0;
}

static const unsigned short * findSgbTitlePal(const char *const title) {
	const GbcPaletteEntry *const r = std::lower_bound(sgbTitlePalettes, sgbTitlePalettesEnd(), title, GbcPaletteEntryLess());
	return r < sgbTitlePalettesEnd() && !std::strcmp(r->title, title) ? r->p : 0;
}

static const unsigned short * findGbcPal(const char *const title) {
	if (const unsigned short *const pal = findGbcDirPal(title))
		return pal;

	return findGbcTitlePal(title);
}

static unsigned long gbcToRgb32(const unsigned bgr15) {
	const unsigned long r = bgr15       & 0x1F;
	const unsigned long g = bgr15 >>  5 & 0x1F;
	const unsigned long b = bgr15 >> 10 & 0x1F;

	unsigned rFinal = ((r * 13) + (g * 2) + b) >> 1;
  unsigned gFinal = ((g * 3) + b);
	unsigned bFinal = ((r * 3) + (g * 2) + (b * 11));

	return (r << 19) | (g << 11) | (b << 3);	// true color
}

}
