/* main.c — GB 阶段 1：console 驱动。
 * 加载 ROM → 执行指定步数（默认 1000）→ 打印 CPU 状态。
 * 验证目标：blargg cpu_instrs 测试 ROM（见 docs/00-project-plan.md）。 */

#include <stdio.h>
#include <stdlib.h>

#include "bus.h"
#include "cpu.h"

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return NULL; }
    unsigned char *buf = malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static void dump_cpu(const Cpu *cpu) {
    printf("PC=%04X SP=%04X A=%02X F=%02X B=%02X C=%02X D=%02X E=%02X "
           "H=%02X L=%02X IME=%d HALT=%d CY=%llu\n",
           cpu->pc, cpu->sp, cpu->a, cpu->f,
           cpu->b, cpu->c, cpu->d, cpu->e,
           cpu->h, cpu->l, cpu->ime, cpu->halted,
           (unsigned long long)cpu->cycles);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "用法 / Usage: %s <rom.gb> [steps]\n", argv[0]);
        return 1;
    }

    size_t size = 0;
    unsigned char *rom = read_file(argv[1], &size);
    if (!rom) return 1;

    Bus *bus = bus_new();
    if (!bus) { free(rom); return 1; }
    if (bus_load_rom(bus, rom, size) != 0) { free(rom); bus_free(bus); return 1; }
    free(rom);

    Cpu cpu;
    cpu_reset(&cpu, bus);

    long steps = (argc >= 3) ? atol(argv[2]) : 1000;
    for (long i = 0; i < steps && !cpu.halted; i++) {
        cpu_step(&cpu);
    }

    dump_cpu(&cpu);

    bus_free(bus);
    return 0;
}
