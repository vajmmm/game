#ifndef GB_BUS_H
#define GB_BUS_H

#include <stdint.h>
#include <stddef.h>

/* GB 统一 64KB 地址总线。
 *
 * CPU 只通过 bus_read / bus_write 访问任何东西（ROM、WRAM、PPU 寄存器……），
 * 不关心地址背后是哪个硬件。这是"总线"把 CPU 与具体硬件解耦的方式。
 *
 * 地址区间（阶段 1 只实现带 * 的部分，其余在后续阶段接入）：
 *   0x0000-0x7FFF  ROM      (32KB；>32KB 需 MBC，阶段 2)
 *   0x8000-0x9FFF  VRAM     (阶段 3)
 *   0xA000-0xBFFF  卡带 RAM  (阶段 2)
 * * 0xC000-0xDFFF  WRAM     (8KB)
 * * 0xE000-0xFDFF  WRAM 镜像 (读 E000 == 读 C000)
 *   0xFE00-0xFE9F  OAM      (阶段 3)
 *   0xFF00-0xFF7F  I/O 寄存器 (阶段 3 起分批接入)
 *   0xFF0F         IF 中断请求标志
 * * 0xFF80-0xFFFE  HRAM     (127B)
 * * 0xFFFF         IE 中断使能
 */

typedef struct Bus Bus;

Bus *bus_new(void);
void bus_free(Bus *bus);

uint8_t bus_read(Bus *bus, uint16_t addr);
void    bus_write(Bus *bus, uint16_t addr, uint8_t val);

/* 把 ROM 数据装入 0x0000-0x7FFF。超过 32KB 返回 -1（需要 MBC）。 */
int bus_load_rom(Bus *bus, const uint8_t *data, size_t size);

#endif /* GB_BUS_H */
