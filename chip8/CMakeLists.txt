# CHIP-8 子项目（已收尾，2026-08-03 完结）
# 依赖 SDL3 平台层。CMake 构建需先配置 SDL3；
# 手动编译命令记录在 chip8/docs/ 中，可直接用。
add_library(chip8core STATIC
    src/chip8.c
)

add_executable(chip8 src/main.c)
target_link_libraries(chip8 PRIVATE chip8core)

# 配置好 SDL3 后启用以下三条：
# find_package(SDL3 REQUIRED)
# target_sources(chip8 PRIVATE src/platform.c)
# target_link_libraries(chip8 PRIVATE SDL3::SDL3)
