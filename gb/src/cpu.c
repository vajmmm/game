/* cpu.c — 阶段 1 骨架。
 * cpu_step 目前是桩：取一个字节、返回占位周期数。
 * 你的工作是实现完整的 LR35902 指令集（约 250+ 条，含 CB 前缀位操作），
 * 每执行一条指令返回正确的 T-cycle 数。 */
#include "cpu.h"

#include "bus.h"

void cpu_reset(Cpu *cpu, Bus *bus) {
    cpu->a = 0x01;
    cpu->f = 0xB0;   /* Z=1 N=0 H=1 C=1 */
    cpu->b = 0x00; cpu->c = 0x13;
    cpu->d = 0x00; cpu->e = 0xD8;
    cpu->h = 0x01; cpu->l = 0x4D;
    cpu->sp = 0xFFFE;
    cpu->pc = 0x0100;
    cpu->ime = false;
    cpu->halted = false;
    cpu->cycles = 0;
    cpu->bus = bus;
}

unsigned cpu_step(Cpu *cpu) {
    /* 取指：读出操作码（1~3 字节指令先读首字节）
     * 译码执行由你来实现，这里是桩 */
    uint8_t opcode = bus_read(cpu->bus, cpu->pc);
    cpu->pc += 1;
    (void)opcode; /* 桩占位，实现时删除 */

    /* 占位：1 个 M-cycle = 4 T-cycle。实现后按每条指令真实周期返回。 */
    unsigned cycles = 4;
    cpu->cycles += cycles;
    return cycles;
}
