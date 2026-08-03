/* chip8.c — 阶段 1 只实现 init / load_rom / 字体加载。
 * 阶段 2 才补 chip8_step，阶段 4 之后才把指令集补全。
 * 本文件刻意留出 chip8_step 的桩函数，先让项目能链接通过。 */

#include "chip8.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

/* CHIP-8 内置字体表，存放在 memory[0x000..0x04F]。
 * 见 docs/01-chip8-architecture.md 的说明。 */
const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70,  /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10,  /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40,  /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90,  /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0,  /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0,  /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80   /* F */
};

static void draw_sprite(Chip8 *c, uint8_t x, uint8_t y, uint8_t height);

void chip8_init(Chip8 *c) {
    srand((unsigned)time(NULL));
    memset(c, 0, sizeof(*c));
    c->pc = 0x200;
    c->waiting_key = 0xFF;
    memcpy(c->memory, CHIP8_FONTSET, sizeof(CHIP8_FONTSET));
}

int chip8_load_rom(Chip8 *c, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        return -1;
    }
    /* ROM 最大容量：4096 - 0x200 = 3584 字节 */
    size_t max_bytes = MEM_SIZE - 0x200;
    size_t n = fread(c->memory + 0x200, 1, max_bytes, f);
    fclose(f);
    if (n == 0) {
        fprintf(stderr, "ROM 读取为空 / ROM read 0 bytes: %s\n", path);
        return -1;
    }
    return (int)n;
}

void chip8_step(Chip8 *c) {
    uint16_t opcode = (c->memory[c->pc] << 8 | c->memory[c->pc + 1]); 

    c->pc += 2; /* 默认 pc + 2，后续指令可能会修改 pc */

    switch (opcode & 0xF000) {

    case 0x0000:
        switch (opcode & 0x00FF) {
        case 0xE0: 
            memset(c->display, 0 ,sizeof(c->display)); c->draw_flag = true;/* cls */ 
            break;
        case 0xEE: 
            c->pc = c->stack[--c->sp]; /* RET */ 
            break;
        }
        break;
    case 0x1000: c->pc = opcode & 0x0FFF; break;  /* JP */
    case 0x2000: 
        c->stack[c->sp++] = c->pc;
        c->pc = opcode & 0x0FFF;/* CALL */ 
        break;
    case 0x6000: 
        c->V[(opcode >> 8) & 0x0F] = opcode & 0xFF; /* LD Vx, NN */
        break;
    case 0x7000: 
        c->V[(opcode >> 8) & 0x0F] += opcode & 0xFF; /* ADD Vx, NN */ 
        break;
    case 0xA000: c->I = opcode & 0x0FFF; break;    /* LD I, NNN */
    case 0xD000: 
        draw_sprite(c, (opcode >> 8) & 0x0F, (opcode >> 4) & 0x0F, opcode & 0x0F); /* DRW */ 
        break;
    case 0x8000:
        switch (opcode & 0x000F) {
        case 0x0: c->V[(opcode >> 8) & 0x0F] = c->V[(opcode >> 4) & 0x0F]; break;  /* LD Vx, Vy */
        case 0x1: c->V[(opcode >> 8) & 0x0F] |= c->V[(opcode >> 4) & 0x0F]; break;  /* OR */
        case 0x2: c->V[(opcode >> 8) & 0x0F] &= c->V[(opcode >> 4) & 0x0F]; break;  /* AND */
        case 0x3: c->V[(opcode >> 8) & 0x0F] ^= c->V[(opcode >> 4) & 0x0F]; break;  /* XOR */
        case 0x4: {  /* ADD Vx, Vy */
            uint16_t sum = (uint16_t)c->V[(opcode >> 8) & 0x0F] + c->V[(opcode >> 4) & 0x0F];
            c->V[(opcode >> 8) & 0x0F] = sum & 0xFF;
            c->V[0xF] = (sum > 0xFF) ? 1 : 0;
        } break;
        case 0x5: {  /* SUB Vx, Vy */
            uint8_t x = (opcode >> 8) & 0x0F;
            uint8_t y = (opcode >> 4) & 0x0F;
            uint8_t vf = (c->V[x] >= c->V[y]) ? 1 : 0;
            c->V[x] = c->V[x] - c->V[y];
            c->V[0xF] = vf;
        } break;
        case 0x6: {  /* SHR Vx, Vy (传统方式: Vx = Vy >> 1) */
            uint8_t lsb = c->V[(opcode >> 4) & 0x0F] & 0x01;
            c->V[(opcode >> 8) & 0x0F] = c->V[(opcode >> 4) & 0x0F] >> 1;
            c->V[0xF] = lsb;
        } break;
        case 0x7: {  /* SUBN Vx, Vy = Vx = Vy - Vx */
            uint8_t x = (opcode >> 8) & 0x0F;
            uint8_t y = (opcode >> 4) & 0x0F;
            uint8_t vf = (c->V[y] >= c->V[x]) ? 1 : 0;
            c->V[x] = c->V[y] - c->V[x];
            c->V[0xF] = vf;
        } break;
        case 0xE: {  /* SHL Vx, Vy (传统方式: Vx = Vy << 1) */
            uint8_t msb = (c->V[(opcode >> 4) & 0x0F] >> 7) & 0x01;
            c->V[(opcode >> 8) & 0x0F] = c->V[(opcode >> 4) & 0x0F] << 1;
            c->V[0xF] = msb;
        } break;
        }
        break;

    case 0x3000: if (c->V[(opcode >> 8) & 0x0F] == (opcode & 0xFF)) c->pc += 2; break;  /* SE Vx, NN */
    case 0x4000: if (c->V[(opcode >> 8) & 0x0F] != (opcode & 0xFF)) c->pc += 2; break;  /* SNE Vx, NN */
    case 0x5000: if (c->V[(opcode >> 8) & 0x0F] == c->V[(opcode >> 4) & 0x0F]) c->pc += 2; break;  /* SE Vx, Vy */
    case 0x9000: if (c->V[(opcode >> 8) & 0x0F] != c->V[(opcode >> 4) & 0x0F]) c->pc += 2; break;  /* SNE Vx, Vy */
    case 0xB000: c->pc = (opcode & 0x0FFF) + c->V[0]; break;  /* JP V0, NNN */
    case 0xC000: c->V[(opcode >> 8) & 0x0F] = (rand() & 0xFF) & (opcode & 0xFF); break;  /* RND */
    
    case 0xF000:
        switch (opcode & 0x00FF) {
        case 0x1E: c->I += c->V[(opcode >> 8) & 0x0F]; break;  /* ADD I, Vx */
        case 0x29: c->I = (c->V[(opcode >> 8) & 0x0F] & 0x0F) * 5; break;  /* LD F, Vx - 字体地址 */
        case 0x33: {  /* LD B, Vx - BCD */
            uint8_t vx = c->V[(opcode >> 8) & 0x0F];
            c->memory[c->I]     = vx / 100;
            c->memory[c->I + 1] = (vx / 10) % 10;
            c->memory[c->I + 2] = vx % 10;
        } break;
        case 0x55: {  /* LD [I], Vx - 存 V0..Vx */
            uint8_t x = (opcode >> 8) & 0x0F;
            for (uint8_t i = 0; i <= x; i++) c->memory[c->I + i] = c->V[i];
        } break;
        case 0x65: {  /* LD Vx, [I] - 读 V0..Vx */
            uint8_t x = (opcode >> 8) & 0x0F;
            for (uint8_t i = 0; i <= x; i++) c->V[i] = c->memory[c->I + i];
        } break;
        case 0x07:  /* LD Vx, DT - 读延迟定时器 */
            c->V[(opcode >> 8) & 0x0F] = c->delay_timer;
            printf("[DEBUG] Read DT = %u into V%X\n", c->delay_timer, (opcode >> 8) & 0x0F); // 调试
            break;

        case 0x15:  /* LD DT, Vx - 设延迟定时器 */
            c->delay_timer = c->V[(opcode >> 8) & 0x0F];
            printf("[DEBUG] Set DT = %u\n", c->delay_timer);   // 调试
            break;

        case 0x18:  /* LD ST, Vx - 设蜂鸣定时器 */
            c->sound_timer = c->V[(opcode >> 8) & 0x0F];
            break;

        case 0x0A: {  /* LD Vx, K - 等待按键 */
            /* 简化实现：if 没有键被按，就把 pc 退回去 (pc -= 2)，
            * 下一帧再执行同一条指令，直到检测到有键被按。 */
            bool any_pressed = false;
            for (int i = 0; i < 16; i++) {
                if (c->keys[i]) {
                    c->V[(opcode >> 8) & 0x0F] = i;
                    any_pressed = true;
                    break;
                }
            }
            if (!any_pressed) {
                c->pc -= 2;  /* 退回本条指令，下次再执行 */
            }
        } break;
        }
        break;
    

    case 0xE000:
        switch (opcode & 0x00FF) {
        case 0x9E:  /* SKP Vx - 如果按住 Vx 对应的键，跳过下一条 */
            if (c->keys[c->V[(opcode >> 8) & 0x0F]]) c->pc += 2;
            break;
        case 0xA1:  /* SKNP Vx - 如果没按住 Vx 对应的键，跳过下一条 */
            if (!c->keys[c->V[(opcode >> 8) & 0x0F]]) c->pc += 2;
            break;
        }
    break;
    default: printf("Unknown: 0x%04X at 0x%03X\n", opcode, c->pc - 2); break;

    }
}

static void draw_sprite(Chip8 *c, uint8_t x,uint8_t y,uint8_t height) {
    c->V[0xF] = 0; /* 清碰撞标志 */
    uint8_t vx = c->V[x] % DISP_W; /* 取模，防止越界 */
    uint8_t vy = c->V[y] % DISP_H;

    for (int row = 0;row < height;row++) {
        uint8_t sprite_byte = c->memory[c->I + row];
        for (int col = 0;col < 8;col++) {
            if (sprite_byte & (0x80 >> col)) { /*   改位是1才画*/
                int px = vx+col;
                int py = vy+row;
                if (px >= DISP_W || py >= DISP_H) continue; /* 越界不画 */
                int idx = py * DISP_W + px;
                if (c->display[idx] == 1) c->V[0xF] = 1; // 碰撞标志
                c->display[idx] ^= 1; /* 异或绘制 */
            }
        }
    }
    c->draw_flag = true; /* 设置绘制标志 */
}

void chip8_print_display(const Chip8 *c) {
    for (int y = 0; y < DISP_H; y++) {
        for (int x = 0; x < DISP_W; x++) {
            putchar(c->display[y * DISP_W + x] ? '.' : ' ');
        }
        putchar('\n');
    }
}