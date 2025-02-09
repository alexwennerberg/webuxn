#include <stdlib.h>
#include <stdio.h>

#include "../uxn.h"
#include "screen.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

UxnScreen uxn_screen;

/* c = !ch ? (color % 5 ? color >> 2 : 0) : color % 4 + ch == 1 ? 0 : (ch - 2 + (color & 3)) % 3 + 1; */

static Uint8 blending[4][16] = {
	{0, 0, 0, 0, 1, 0, 1, 1, 2, 2, 0, 2, 3, 3, 3, 0},
	{0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3},
	{1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1, 1, 2, 3, 1},
	{2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2, 2, 3, 1, 2}};

void
screen_change(Uint16 x1, Uint16 y1, Uint16 x2, Uint16 y2)
{
	if(x1 > uxn_screen.width && x2 > x1) return;
	if(y1 > uxn_screen.height && y2 > y1) return;
	if(x1 > x2) x1 = 0;
	if(y1 > y2) y1 = 0;
	if(x1 < uxn_screen.x1) uxn_screen.x1 = x1;
	if(y1 < uxn_screen.y1) uxn_screen.y1 = y1;
	if(x2 > uxn_screen.x2) uxn_screen.x2 = x2;
	if(y2 > uxn_screen.y2) uxn_screen.y2 = y2;
}

void
screen_fill(Uint8 *layer, int x1, int y1, int x2, int y2, int color)
{
	int x, y, width = uxn_screen.width, height = uxn_screen.height;
	for(y = y1; y < y2 && y < height; y++)
		for(x = x1; x < x2 && x < width; x++)
			layer[x + y * width] = color;
}

static void
screen_blit(Uint8 *layer, Uint8 *ram, Uint16 addr, int x1, int y1, int color, int flipx, int flipy, int twobpp)
{
	int v, h, width = uxn_screen.width, height = uxn_screen.height, opaque = (color % 5);
	for(v = 0; v < 8; v++) {
		Uint16 c = ram[(addr + v) & 0xffff] | (twobpp ? (ram[(addr + v + 8) & 0xffff] << 8) : 0);
		Uint16 y = y1 + (flipy ? 7 - v : v);
		for(h = 7; h >= 0; --h, c >>= 1) {
			Uint8 ch = (c & 1) | ((c >> 7) & 2);
			if(opaque || ch) {
				Uint16 x = x1 + (flipx ? 7 - h : h);
				if(x < width && y < height)
					layer[x + y * width] = blending[ch][color];
			}
		}
	}
}

/* clang-format off */

static Uint8 icons[] = {
	0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 
	0x10, 0x00, 0x7c, 0x82, 0x02, 0x7c, 0x80, 0x80, 0xfe, 0x00, 0x7c, 0x82, 0x02, 0x1c, 0x02, 
	0x82, 0x7c, 0x00, 0x0c, 0x14, 0x24, 0x44, 0x84, 0xfe, 0x04, 0x00, 0xfe, 0x80, 0x80, 0x7c, 
	0x02, 0x82, 0x7c, 0x00, 0x7c, 0x82, 0x80, 0xfc, 0x82, 0x82, 0x7c, 0x00, 0x7c, 0x82, 0x02, 
	0x1e, 0x02, 0x02, 0x02, 0x00, 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c, 0x00, 0x7c, 0x82, 
	0x82, 0x7e, 0x02, 0x82, 0x7c, 0x00, 0x7c, 0x82, 0x02, 0x7e, 0x82, 0x82, 0x7e, 0x00, 0xfc, 
	0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc, 0x00, 0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00, 
	0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x82, 0x7c,
	0x00, 0x7c, 0x82, 0x80, 0xf0, 0x80, 0x80, 0x80};

/* clang-format on */

static void
draw_byte(Uint8 v, Uint16 x, Uint16 y, Uint8 color)
{
	screen_blit(uxn_screen.fg, icons, v >> 4 << 3, x, y, color, 0, 0, 0);
	screen_blit(uxn_screen.fg, icons, (v & 0xf) << 3, x + 8, y, color, 0, 0, 0);
	screen_change(x, y, x + 0x10, y + 0x8);
}

static void
screen_debugger(Uxn *u)
{
	int i;
	for(i = 0; i < u->wst.ptr; i++)
		draw_byte(u->wst.dat[i], i * 0x18 + 0x8, uxn_screen.height - 0x18, 0x2);
	for(i = 0; i < u->rst.ptr; i++)
		draw_byte(u->rst.dat[i], i * 0x18 + 0x8, uxn_screen.height - 0x10, 0x3);
	for(i = 0; i < 0x40; i++)
		draw_byte(u->ram[i], (i & 0x7) * 0x18 + 0x8, ((i >> 3) << 3) + 0x8, 1 + !!u->ram[i]);
}

void
screen_palette(Uint8 *addr)
{
	int i, shift;
	for(i = 0, shift = 4; i < 4; ++i, shift ^= 4) {
		Uint8
			r = (addr[0 + i / 2] >> shift) & 0xf,
			g = (addr[2 + i / 2] >> shift) & 0xf,
			b = (addr[4 + i / 2] >> shift) & 0xf;
		uxn_screen.palette[i] = 0x0f000000 | r << 16 | g << 8 | b;
		uxn_screen.palette[i] |= uxn_screen.palette[i] << 4;
	}
	screen_change(0, 0, uxn_screen.width, uxn_screen.height);
}
void
screen_redraw(Uxn *u)
{
	int i, j, o, y;
	Uint8 *fg = uxn_screen.fg, *bg = uxn_screen.bg;
	Uint16 w = uxn_screen.width, h = uxn_screen.height;
	Uint16 x1 = uxn_screen.x1, y1 = uxn_screen.y1;
	Uint16 x2 = uxn_screen.x2 > w ? w : uxn_screen.x2, y2 = uxn_screen.y2 > h ? h : uxn_screen.y2;
	Uint32 palette[16], *pixels = uxn_screen.pixels;
	uxn_screen.x1 = uxn_screen.y1 = 0xffff;
	uxn_screen.x2 = uxn_screen.y2 = 0;
	if(u->dev[0x0e])
		screen_debugger(u);
	for(i = 0; i < 16; i++)
		palette[i] = uxn_screen.palette[(i >> 2) ? (i >> 2) : (i & 3)];
	for(y = y1; y < y2; y++)
		for(o = y * w, i = x1 + o, j = x2 + o; i < j; i++)
			pixels[i] = palette[fg[i] << 2 | bg[i]];
}

Uint8
screen_dei(Uxn *u, Uint8 addr)
{
	switch(addr) {
	case 0x22: return uxn_screen.width >> 8;
	case 0x23: return uxn_screen.width;
	case 0x24: return uxn_screen.height >> 8;
	case 0x25: return uxn_screen.height;
	default: return u->dev[addr];
	}
}

void
screen_deo(Uint8 *ram, Uint8 *d, Uint8 port)
{
	Uint8 *port_width, *port_height, *port_x, *port_y, *port_addr;
	Uint16 x, y, dx, dy, dxy, dyx, addr, addr_incr;
	switch(port) {
	case 0x3:
		port_width = d + 0x2;
		/* screen_resize(PEEK2(port_width), uxn_screen.height); */
		break;
	case 0x5:
		port_height = d + 0x4;
		/* screen_resize(uxn_screen.width, PEEK2(port_height)); */
		break;
	case 0xe: {
		Uint8 ctrl = d[0xe];
		Uint8 color = ctrl & 0x3;
		Uint8 *layer = (ctrl & 0x40) ? uxn_screen.fg : uxn_screen.bg;
		port_x = d + 0x8;
		port_y = d + 0xa;
		x = PEEK2(port_x);
		y = PEEK2(port_y);
		/* fill mode */
		if(ctrl & 0x80) {
			Uint16 x2 = uxn_screen.width;
			Uint16 y2 = uxn_screen.height;
			if(ctrl & 0x10) x2 = x, x = 0;
			if(ctrl & 0x20) y2 = y, y = 0;
			screen_fill(layer, x, y, x2, y2, color);
			screen_change(x, y, x2, y2);
		}
		/* pixel mode */
		else {
			Uint16 width = uxn_screen.width;
			Uint16 height = uxn_screen.height;
			if(x < width && y < height)
				layer[x + y * width] = color;
			screen_change(x, y, x + 1, y + 1);
			if(d[0x6] & 0x1) POKE2(port_x, x + 1);
			if(d[0x6] & 0x2) POKE2(port_y, y + 1);
		}
		break;
	}
	case 0xf: {
		Uint8 i;
		Uint8 ctrl = d[0xf];
		Uint8 move = d[0x6];
		Uint8 length = move >> 4;
		Uint8 twobpp = !!(ctrl & 0x80);
		Uint8 *layer = (ctrl & 0x40) ? uxn_screen.fg : uxn_screen.bg;
		Uint8 color = ctrl & 0xf;
		int flipx = (ctrl & 0x10), fx = flipx ? -1 : 1;
		int flipy = (ctrl & 0x20), fy = flipy ? -1 : 1;
		port_x = d + 0x8;
		port_y = d + 0xa;
		port_addr = d + 0xc;
		x = PEEK2(port_x), dx = (move & 0x1) << 3, dxy = dx * fy;
		y = PEEK2(port_y), dy = (move & 0x2) << 2, dyx = dy * fx;
		addr = PEEK2(port_addr), addr_incr = (move & 0x4) << (1 + twobpp);
		for(i = 0; i <= length; i++) {
			screen_blit(layer, ram, addr, x + dyx * i, y + dxy * i, color, flipx, flipy, twobpp);
			addr += addr_incr;
		}
		screen_change(x, y, x + dyx * length + 8, y + dxy * length + 8);
		if(move & 0x1) {
			x = x + dx * fx;
			POKE2(port_x, x);
		}
		if(move & 0x2) {
			y = y + dy * fy;
			POKE2(port_y, y);
		}
		if(move & 0x4) POKE2(port_addr, addr); /* auto addr+length */
		break;
	}
	}
}
