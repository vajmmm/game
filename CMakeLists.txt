cmake_minimum_required(VERSION 3.10)
project(emulators C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 默认警告全开，把潜在问题尽早暴露出来
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
elseif(MSVC)
    add_compile_options(/W4)
endif()

# 子项目一律走 add_subdirectory 模式。
# 当前主线是 GB（阶段 1：纯 console，无外部依赖，可直接构建）。
add_subdirectory(gb)

# CHIP-8 已收尾，且依赖 SDL3，默认不参与构建。
# 需要启用时：配置好 SDL3 后取消下面一行的注释。
# add_subdirectory(chip8)