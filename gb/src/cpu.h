#ifndef GB_CPU_H
#define GB_CPU_H

#include <stdint.h>
#include <stdbool.h>

/* LR35902 CPU（8080 × Z80 杂交体）。
 *
 * 8-bit 寄存器组：A(累加器) F(标志) B C D E H L
 *   F 只用高 4 位：Bit7 Z, Bit6 N, Bit5 H(半进位), Bit4 C
 *   B/C、D/E、H/L 可组合成 16-bit 对：BC DE HL
 * SP 栈指针，PC 程序计数器。
 *
 * 中断控制：
 *   ime    = DI/EI 控制的主开关
 *   halted = HALT 睡眠中，等中断唤醒
 *   IF/IE 是内存映射寄存器（FF0F/FFFF），存放在 bus 里，CPU 通过 bus 读写。
 *
 * cycles = 累计时钟周期（T-cycle）。每条指令实现时必须返回正确周期数，
 * 这是 GB 时序正确的基础（PPU/APU 阶段 3/5 依赖它）。 */

typedef struct Bus Bus;

typedef struct {
    uint8_t a, f, b, c, d, e, h, l;
    uint16_t sp, pc;
    bool ime;
    bool halted;
    uint64_t cycles;
    Bus *bus;
} Cpu;

/* 复位到真实 GB 上电状态（寄存器初始值见 pandocs，PC=0x0100 跳过启动 ROM）。 */
void cpu_reset(Cpu *cpu, Bus *bus);

/* 执行一条指令，返回消耗的 T-cycle 数，并推进 cpu->cycles。 */
unsigned cpu_step(Cpu *cpu);

#endif /* GB_CPU_H */
