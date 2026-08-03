# roms/

GB 测试 ROM（版权归原作者，放本地，不入 git——见根 `.gitignore`）。

阶段 1 需要：**blargg cpu_instrs**（验证 CPU 指令集与周期）。

来源：`retrio/gb-test-roms`（GitHub），clone 或下载后把 `cpu_instrs` 相关的 `.gb` 文件放进来。

## 用法

```bash
# 跑全部 CPU 测试（大循环，几百万步）
cd gb
./gb.exe roms/cpu_instrs.gb 50000000
```

阶段 1 判定方式见 `docs/00-project-plan.md`（串口输出 FF01 检测）。
