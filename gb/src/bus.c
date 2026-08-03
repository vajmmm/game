/* bus.c — 阶段 1 最小总线实现。
 * 只处理带 * 的区间：ROM / WRAM(含镜像) / HRAM / IF / IE。
 * 其余区间暂按"读 0xFF、写忽略"处理，后续阶段逐个接入。
 * 这个文件是"地址空间分发"的骨架，各硬件模块会陆续在这里挂上来。 */

#include "bus.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ROM_SIZE   0x8000  /* 32KB：bank0 + bank1 */
#define WRAM_SIZE  0x2000  /* 8KB：bank0 + bank1 */
#define HRAM_SIZE  0x0080

struct Bus {
    uint8_t rom[ROM_SIZE];
    uint8_t wram[WRAM_SIZE];
    uint8_t hram[HRAM_SIZE];
    uint8_t ie;      /* FFFF */
    uint8_t iflags;  /* FF0F */
};

Bus *bus_new(void) {
    Bus *b = calloc(1, sizeof(*b));
    if (!b) fprintf(stderr, "bus_new: calloc failed\n");
    return b;
}

void bus_free(Bus *b) {
    free(b);
}

uint8_t bus_read(Bus *b, uint16_t addr) {
    if (addr < 0x8000) return b->rom[addr];
    if (addr >= 0xC000 && addr <= 0xDFFF) return b->wram[addr - 0xC000];
    if (addr >= 0xE000 && addr <= 0xFDFF) return b->wram[addr - 0xE000]; /* 镜像 */
    if (addr == 0xFF0F) return b->iflags;
    if (addr >= 0xFF80 && addr <= 0xFFFE) return b->hram[addr - 0xFF80];
    if (addr == 0xFFFF) return b->ie;
    return 0xFF; /* 未接入硬件：占位 */
}

void bus_write(Bus *b, uint16_t addr, uint8_t val) {
    if (addr < 0x8000) return; /* ROM 只读 */
    if (addr >= 0xC000 && addr <= 0xDFFF) { b->wram[addr - 0xC000] = val; return; }
    if (addr >= 0xE000 && addr <= 0xFDFF) { b->wram[addr - 0xE000] = val; return; }
    if (addr == 0xFF0F) { b->iflags = val; return; }
    if (addr >= 0xFF80 && addr <= 0xFFFE) { b->hram[addr - 0xFF80] = val; return; }
    if (addr == 0xFFFF) { b->ie = val; return; }
    /* 其余：暂忽略 */
}

int bus_load_rom(Bus *b, const uint8_t *data, size_t size) {
    if (size > ROM_SIZE) {
        fprintf(stderr, "bus_load_rom: ROM 超过 32KB (%zu B)，需要 MBC 支持（阶段 2）\n", size);
        return -1;
    }
    memcpy(b->rom, data, size);
    return 0;
}
