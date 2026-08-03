#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE   4096
#define NUM_REGS   16
#define STACK_SIZE 16
#define DISP_W     64
#define DISP_H     32

/* CHIP-8 字体表：每个字符 5 字节，共 16 个字符 = 80 字节 */
extern const uint8_t CHIP8_FONTSET[80];

typedef struct {
    uint8_t  memory[MEM_SIZE]; /*!< 内存 */
    uint8_t  V[NUM_REGS];      /*!< 通用寄存器 */
    uint16_t I;                /*!< 索引寄存器 */
    uint16_t pc;               /*!< 程序计数器 */
    uint16_t stack[STACK_SIZE];/*!< 调用栈 */
    uint8_t  sp;               /*!< 栈指针 */
    uint8_t  delay_timer;      /*!< 延时定时器 */
    uint8_t  sound_timer;   /*!< 声音定时器 */  
    uint8_t  display[DISP_W * DISP_H]; /*!< 显示缓冲区 */
    uint8_t  keys[16];        /*!< 键盘状态 */
    bool     draw_flag;        /*!< 绘制标志 */
    uint8_t  waiting_key;   /*!< FX0A 等待的键，0xFF 表示不等待 */
} Chip8;

/* 生命周期 */
void chip8_init(Chip8 *c);
int  chip8_load_rom(Chip8 *c, const char *path);
void chip8_step(Chip8 *c);

/* 调试辅助 */
void chip8_print_display(const Chip8 *c);

#endif /* CHIP8_H */