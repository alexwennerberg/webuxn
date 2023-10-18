#include <stdio.h>
#include <stdlib.h>

#include "../uxn.h"
#include "system.h"

/*
Copyright (c) 2022-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

char *boot_rom;
Uint8 dei_masks[0x100], deo_masks[0x100];
Uint16 dev_vers[0x10], dei_mask[0x10], deo_mask[0x10];

void
system_connect(Uint8 device, Uint8 ver, Uint16 dei, Uint16 deo)
{
	int i, d = (device << 0x4);
	for(i = 0; i < 0x10; i++) {
		dei_masks[d + i] = (dei >> i) & 0x1;
		deo_masks[d + i] = (deo >> i) & 0x1;
	}
	dev_vers[device] = ver;
	dei_mask[device] = dei;
	deo_mask[device] = deo;
}
void
system_deo(Uxn *u, Uint8 *d, Uint8 port)
{
	Uint8 *ram;
	Uint16 addr;
	switch(port) {
	case 0x3:
		ram = u->ram;
		addr = PEEK2(d + 2);
		if(ram[addr] == 0x1) {
			Uint16 i, length = PEEK2(ram + addr + 1);
			Uint16 a_page = PEEK2(ram + addr + 1 + 2), a_addr = PEEK2(ram + addr + 1 + 4);
			Uint16 b_page = PEEK2(ram + addr + 1 + 6), b_addr = PEEK2(ram + addr + 1 + 8);
			int src = (a_page % RAM_PAGES) * 0x10000, dst = (b_page % RAM_PAGES) * 0x10000;
			for(i = 0; i < length; i++)
				ram[dst + (Uint16)(b_addr + i)] = ram[src + (Uint16)(a_addr + i)];
		}
		break;
	case 0x4:
		u->wst.ptr = d[4];
		break;
	case 0x5:
		u->rst.ptr = d[5];
		break;
	case 0xe:
		/* system_inspect(u); */
		break;
	}
}

Uint8
system_dei(Uxn *u, Uint8 addr)
{
  switch(addr) {
  case 0x4: return u->wst.ptr;
  case 0x5: return u->rst.ptr;
  default: return u->dev[addr];
  }
}
