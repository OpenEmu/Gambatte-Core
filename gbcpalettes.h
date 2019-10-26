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

// Hardware Palettes
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

static const unsigned short p007[] = {	// Orange | A + Down
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

static const unsigned short p013[] = {	// Inverted | B + Right
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF),
	PACK15_4(0x000000, 0x008484, 0xFFDE00, 0xFFFFFF)
};

static const unsigned short p016[] = {	// Grayscale | B + Left
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

static const unsigned short p319[] = {	// Dark Brown | B + Up
	PACK15_4(0xFFE6C5, 0xCE9C84, 0x846B29, 0x5A3108),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000),
	PACK15_4(0xFFFFFF, 0xFFAD63, 0x843100, 0x000000)
};

static const unsigned short p31C[] = {	// Dark Green (GBC Default) | A + Right
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

static const unsigned short p50D[] = {	// Dark Blue | A + Left
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

static const unsigned short p510[] = {	// Red | A + Up
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

static const unsigned short p51A[] = {	// Yellow | B + Down
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
static const unsigned short sgb1A[] = {	// 1-A (SGB Default)
	PACK15_4(0xF8E8C8, 0xD89048, 0xA82820, 0x301850),
	PACK15_4(0xF8E8C8, 0xD89048, 0xA82820, 0x301850),
	PACK15_4(0xF8E8C8, 0xD89048, 0xA82820, 0x301850)
};

static const unsigned short sgb1B[] = {
	PACK15_4(0xD8D8C0, 0xC8B070, 0xB05010, 0x000000),
	PACK15_4(0xD8D8C0, 0xC8B070, 0xB05010, 0x000000),
	PACK15_4(0xD8D8C0, 0xC8B070, 0xB05010, 0x000000)
};

static const unsigned short sgb1C[] = {
	PACK15_4(0xF8C0F8, 0xE89850, 0x983860, 0x383898),
	PACK15_4(0xF8C0F8, 0xE89850, 0x983860, 0x383898),
	PACK15_4(0xF8C0F8, 0xE89850, 0x983860, 0x383898)
};

static const unsigned short sgb1D[] = {
	PACK15_4(0xF8F8A8, 0xC08048, 0xF80000, 0x501800),
	PACK15_4(0xF8F8A8, 0xC08048, 0xF80000, 0x501800),
	PACK15_4(0xF8F8A8, 0xC08048, 0xF80000, 0x501800)
};

static const unsigned short sgb1E[] = {
	PACK15_4(0xF8D8B0, 0x78C078, 0x688840, 0x583820),
	PACK15_4(0xF8D8B0, 0x78C078, 0x688840, 0x583820),
	PACK15_4(0xF8D8B0, 0x78C078, 0x688840, 0x583820)
};

static const unsigned short sgb1F[] = {
	PACK15_4(0xD8E8F8, 0xE08850, 0xA80000, 0x004010),
	PACK15_4(0xD8E8F8, 0xE08850, 0xA80000, 0x004010),
	PACK15_4(0xD8E8F8, 0xE08850, 0xA80000, 0x004010)
};

static const unsigned short sgb1G[] = {
	PACK15_4(0x000050, 0x00A0E8, 0x787800, 0xF8F858),
	PACK15_4(0x000050, 0x00A0E8, 0x787800, 0xF8F858),
	PACK15_4(0x000050, 0x00A0E8, 0x787800, 0xF8F858)
};

static const unsigned short sgb1H[] = {	// 1-H (SGB Brownscale)
	PACK15_4(0xF8E8E0, 0xF8B888, 0x804000, 0x301800),
	PACK15_4(0xF8E8E0, 0xF8B888, 0x804000, 0x301800),
	PACK15_4(0xF8E8E0, 0xF8B888, 0x804000, 0x301800)
};

static const unsigned short sgb2A[] = {
	PACK15_4(0xF0C8A0, 0xC08848, 0x287800, 0x000000),
	PACK15_4(0xF0C8A0, 0xC08848, 0x287800, 0x000000),
	PACK15_4(0xF0C8A0, 0xC08848, 0x287800, 0x000000)
};

static const unsigned short sgb2B[] = {
	PACK15_4(0xF8F8F8, 0xF8E850, 0xF83000, 0x500058),
	PACK15_4(0xF8F8F8, 0xF8E850, 0xF83000, 0x500058),
	PACK15_4(0xF8F8F8, 0xF8E850, 0xF83000, 0x500058)
};

static const unsigned short sgb2C[] = {
	PACK15_4(0xF8C0F8, 0xE88888, 0x7830E8, 0x282898),
	PACK15_4(0xF8C0F8, 0xE88888, 0x7830E8, 0x282898),
	PACK15_4(0xF8C0F8, 0xE88888, 0x7830E8, 0x282898)
};

static const unsigned short sgb2D[] = {
	PACK15_4(0xF8F8A0, 0x00F800, 0xF83000, 0x000050),
	PACK15_4(0xF8F8A0, 0x00F800, 0xF83000, 0x000050),
	PACK15_4(0xF8F8A0, 0x00F800, 0xF83000, 0x000050)
};

static const unsigned short sgb2E[] = {
	PACK15_4(0xF8C880, 0x90B0E0, 0x281060, 0x100810),
	PACK15_4(0xF8C880, 0x90B0E0, 0x281060, 0x100810),
	PACK15_4(0xF8C880, 0x90B0E0, 0x281060, 0x100810)
};

static const unsigned short sgb2F[] = {
	PACK15_4(0xD0F8F8, 0xF89050, 0xA00000, 0x180000),
	PACK15_4(0xD0F8F8, 0xF89050, 0xA00000, 0x180000),
	PACK15_4(0xD0F8F8, 0xF89050, 0xA00000, 0x180000)
};

static const unsigned short sgb2G[] = {
	PACK15_4(0x68B838, 0xE05040, 0xE0B880, 0x001800),
	PACK15_4(0x68B838, 0xE05040, 0xE0B880, 0x001800),
	PACK15_4(0x68B838, 0xE05040, 0xE0B880, 0x001800)
};

static const unsigned short sgb2H[] = {	// 2-H (SGB Grayscale)
	PACK15_4(0xF8F8F8, 0xB8B8B8, 0x707070, 0x000000),
	PACK15_4(0xF8F8F8, 0xB8B8B8, 0x707070, 0x000000),
	PACK15_4(0xF8F8F8, 0xB8B8B8, 0x707070, 0x000000)
};

static const unsigned short sgb3A[] = {
	PACK15_4(0xF8D098, 0x70C0C0, 0xF86028, 0x304860),
	PACK15_4(0xF8D098, 0x70C0C0, 0xF86028, 0x304860),
	PACK15_4(0xF8D098, 0x70C0C0, 0xF86028, 0x304860)
};

static const unsigned short sgb3B[] = {
	PACK15_4(0xD8D8C0, 0xE08020, 0x005000, 0x001010),
	PACK15_4(0xD8D8C0, 0xE08020, 0x005000, 0x001010),
	PACK15_4(0xD8D8C0, 0xE08020, 0x005000, 0x001010)
};

static const unsigned short sgb3C[] = {
	PACK15_4(0xE0A8C8, 0xF8F878, 0x00B8F8, 0x202058),
	PACK15_4(0xE0A8C8, 0xF8F878, 0x00B8F8, 0x202058),
	PACK15_4(0xE0A8C8, 0xF8F878, 0x00B8F8, 0x202058)
};

static const unsigned short sgb3D[] = {
	PACK15_4(0xF0F8B8, 0xE0A878, 0x08C800, 0x000000),
	PACK15_4(0xF0F8B8, 0xE0A878, 0x08C800, 0x000000),
	PACK15_4(0xF0F8B8, 0xE0A878, 0x08C800, 0x000000)
};

static const unsigned short sgb3E[] = {
	PACK15_4(0xF8F8C0, 0xE0B068, 0xB07820, 0x504870),
	PACK15_4(0xF8F8C0, 0xE0B068, 0xB07820, 0x504870),
	PACK15_4(0xF8F8C0, 0xE0B068, 0xB07820, 0x504870)
};

static const unsigned short sgb3F[] = {
	PACK15_4(0x7878C8, 0xF868F8, 0xF8D000, 0x404040),
	PACK15_4(0x7878C8, 0xF868F8, 0xF8D000, 0x404040),
	PACK15_4(0x7878C8, 0xF868F8, 0xF8D000, 0x404040)
};

static const unsigned short sgb3G[] = {
	PACK15_4(0x60D850, 0xF8F8F8, 0xC83038, 0x380000),
	PACK15_4(0x60D850, 0xF8F8F8, 0xC83038, 0x380000),
	PACK15_4(0x60D850, 0xF8F8F8, 0xC83038, 0x380000)
};

static const unsigned short sgb3H[] = {	// 3-H (SGB Greenscale)
	PACK15_4(0xE0F8A0, 0x78C838, 0x488818, 0x081800),
	PACK15_4(0xE0F8A0, 0x78C838, 0x488818, 0x081800),
	PACK15_4(0xE0F8A0, 0x78C838, 0x488818, 0x081800)
};

static const unsigned short sgb4A[] = {
	PACK15_4(0xF0A868, 0x78A8F8, 0xD000D0, 0x000078),
	PACK15_4(0xF0A868, 0x78A8F8, 0xD000D0, 0x000078),
	PACK15_4(0xF0A868, 0x78A8F8, 0xD000D0, 0x000078)
};

static const unsigned short sgb4B[] = {
	PACK15_4(0xF0E8F0, 0xE8A060, 0x407838, 0x180808),
	PACK15_4(0xF0E8F0, 0xE8A060, 0x407838, 0x180808),
	PACK15_4(0xF0E8F0, 0xE8A060, 0x407838, 0x180808)
};

static const unsigned short sgb4C[] = {
	PACK15_4(0xF8E0E0, 0xD8A0D0, 0x98A0E0, 0x080000),
	PACK15_4(0xF8E0E0, 0xD8A0D0, 0x98A0E0, 0x080000),
	PACK15_4(0xF8E0E0, 0xD8A0D0, 0x98A0E0, 0x080000)
};

static const unsigned short sgb4D[] = {
	PACK15_4(0xF8F8B8, 0x90C8C8, 0x486878, 0x082048),
	PACK15_4(0xF8F8B8, 0x90C8C8, 0x486878, 0x082048),
	PACK15_4(0xF8F8B8, 0x90C8C8, 0x486878, 0x082048)
};

static const unsigned short sgb4E[] = {
	PACK15_4(0xF8D8A8, 0xE0A878, 0x785888, 0x002030),
	PACK15_4(0xF8D8A8, 0xE0A878, 0x785888, 0x002030),
	PACK15_4(0xF8D8A8, 0xE0A878, 0x785888, 0x002030)
};

static const unsigned short sgb4F[] = {
	PACK15_4(0xB8D0D0, 0xD880D8, 0x8000A0, 0x380000),
	PACK15_4(0xB8D0D0, 0xD880D8, 0x8000A0, 0x380000),
	PACK15_4(0xB8D0D0, 0xD880D8, 0x8000A0, 0x380000)
};

static const unsigned short sgb4G[] = {
	PACK15_4(0xB0E018, 0xB82058, 0x281000, 0x008060),
	PACK15_4(0xB0E018, 0xB82058, 0x281000, 0x008060),
	PACK15_4(0xB0E018, 0xB82058, 0x281000, 0x008060)
};

static const unsigned short sgb4H[] = {	// 4-H (SGB DMG-esque)
	PACK15_4(0xF8F8C8, 0xB8C058, 0x808840, 0x405028),
	PACK15_4(0xF8F8C8, 0xB8C058, 0x808840, 0x405028),
	PACK15_4(0xF8F8C8, 0xB8C058, 0x808840, 0x405028)
};

// Special Palettes <https://www.deviantart.com/thewolfbunny/gallery/69987002/game-boy-palettes>
static const unsigned short sgbZL[] = {	// Link's Awakening DX
	PACK15_4(0xF8F8B0, 0x78C078, 0x688840, 0x583820),
	PACK15_4(0xF8F8B0, 0x78C078, 0x688840, 0x583820),
	PACK15_4(0xF8F8B0, 0x78C078, 0x688840, 0x583820)
};

static const unsigned short sgbWM[] = {	// Mega Man World V
	PACK15_4(0xD0D0D0, 0x70A0E0, 0x406890, 0x082030),
	PACK15_4(0xD0D0D0, 0x70A0E0, 0x406890, 0x082030),
	PACK15_4(0xD0D0D0, 0x70A0E0, 0x406890, 0x082030)
};

static const unsigned short sgbAP[] = {	// Pokémon <https://github.com/pret/pokered>
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010),
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010),
	PACK15_4(0xFFEFFF, 0xF7B58C, 0x84739C, 0x191010)
};

static const unsigned short sgbAPS[] = {	// Pokémon Yellow <https://github.com/pret/pokeyellow>
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131),
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131),
	PACK15_4(0xFFFFF7, 0xFFF7B5, 0xDE8484, 0x313131)
};

static const unsigned short pSWJ[] = {	// WonderSwan Monochrome
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D),
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D),
	PACK15_4(0xFEFEFE, 0xC2C2C2, 0x686868, 0x1D1D1D)
};

static const unsigned short sgbACR[] = {	// Conker's Pocket Tales
	PACK15_4(0xD0D860, 0x88A000, 0x385000, 0x000000),
	PACK15_4(0xD0D860, 0x88A000, 0x385000, 0x000000),
	PACK15_4(0xD0D860, 0x88A000, 0x385000, 0x000000)
};

static const unsigned short p799510[] = {	// Squidlit
	PACK15_4(0x9CBE0C, 0x6E870A, 0x2C6234, 0x0C360C),
	PACK15_4(0x9CBE0C, 0x6E870A, 0x2C6234, 0x0C360C),
	PACK15_4(0x9CBE0C, 0x6E870A, 0x2C6234, 0x0C360C)
};

static const unsigned short pAMA[] = {	// Superball Ivory (Super Mario Maker 2)
	PACK15_4(0xEEF0BC, 0xBCBC8A, 0x828250, 0x646432),
	PACK15_4(0xEEF0BC, 0xBCBC8A, 0x828250, 0x646432),
	PACK15_4(0xEEF0BC, 0xBCBC8A, 0x828250, 0x646432)
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

#undef PACK15_4
#undef PACK15_1
#undef TO5BIT

struct GbcPaletteEntry { const char *title; const unsigned short *p; };

static const GbcPaletteEntry gbcDirPalettes[] = {
	{ "GB - DMG", gbdmg },	// GB Pea Soup Green
	{ "GB - Light", gblit },	// GB Light Aquamarine
	{ "GBC - Dark Green", p31C },	// A + Right (default GBC)
	{ "SGB - 1A", sgb1A },	// 1-A (default SGB)
	{ "SGB - 2H", sgb2H }	// SGB Grayscale
};

static const GbcPaletteEntry gbcTitlePalettes[] = {
	{ "ALLEY WAY", p008 },
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
	{ "ALLEY WAY", sgb3F },
	{ "BALLOON KID", sgb1A },	// unofficial ("BALLôôN KID" alt.)
	{ "BALLôôN KID", sgb1A },
	{ "BASEBALL", sgb2G },
	{ "CASINO FUNPAK", sgb1A },	// unofficial (Nintendo Power Issue #67)
	{ "CONTRA ALIEN WAR", sgb1F },	// unofficial (Nintendo Power Issue #66)
	{ "CONTRA SPIRITS", sgb1F },	// unofficial ("CONTRA ALIEN WAR" alt.)
	{ "CUTTHROAT ISLAND", sgb3E },	// unofficial (Nintendo Power Issue #82)
	{ "DMG FOOTBALL", sgb4B },	// unofficial (nensondubois)
	{ "DR.MARIO", sgb3B },
	{ "F1RACE", sgb4B },	// unofficial improvement, really sgb4F
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
	{ "MARIO & YOSHI", sgb2D },
	{ "MARIOLAND2", sgb3D },
	{ "MEGA MAN 2", sgbWM },	//  unofficial (SGB)
	{ "MEGAMAN", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN3", sgbWM },	//  unofficial (SGB)
	{ "MEGAMAN4", sgbWM },	// unofficial (SGB)
	{ "MEGAMAN5", sgbWM },	// Super Game Boy
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
	{ "ROCKMANWORLD5", sgbWM },	// Super Game Boy
	{ "RVT", sgb4B },	// unofficial (Nintendo Power Issue #63)
	{ "SOLARSTRIKER", sgb1G },
	{ "SPACE INVADERS", sgb4D },	// unofficial (Nintendo Power Issue #62)
	{ "SUPER MARIOLAND", sgb1F },
	{ "SUPERMARIOLAND3", sgb1B },
	{ "TARZAN", sgb2A },	// unofficial (Nintendo Power Issue #62)
	{ "TAZ-MANIA", sgb1A },	// unofficial (Nintendo Power Issue #64)
	{ "TEEN JEOPARDY!", sgb2F },	// unofficial ("JEOPARDY SPORTS" alt.)
	{ "TENNIS", sgb3G },
	{ "TETRIS", sgb3A },
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
	{ "YOSSY NO TAMAGO", sgb2D },
	{ "ZELDA", sgbZL }	// unofficial (DX alt.)
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

	return rFinal << 16 | gFinal << 9 | bFinal >> 1;
}

}
