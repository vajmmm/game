# GB 子项目
# 阶段 1 是纯 console 驱动（无 SDL 依赖），验证 CPU 用 blargg cpu_instrs。
# 阶段 3 接 PPU 显示时才引入 SDL（届时从 chip8 平台层复制改造）。

add_library(gbcore STATIC
    src/bus.c
    src/cpu.c
)

add_executable(gb src/main.c)
target_link_libraries(gb PRIVATE gbcore)
