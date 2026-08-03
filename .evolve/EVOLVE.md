# EVOLVE.md - Evolution Control

This file is the project-local control entrypoint for evolution mode. It keeps
long-term evolution memory, indexes, waterlines, and run state. Ordinary work
does not need to load this file unless it depends on project history.

## Loading

- Work rules: `AGENTS.md`
- Skill index: `.evolve/SKILLS.md`
- Knowledge schema index: `.evolve/SCHEMAS.md`
- Evolution state: `.evolve/EVOLVE.md`
- Recent continuity: `.evolve/TASKS.md`

## Stable Rules

- `.evolve/` is local to this project.
- Evolution logs summarize sessions and audit evolution executions.
- Feedback, issues, notes, experience, schemas, and skills have separate owners.
- New durable knowledge should replace stale guidance instead of accumulating beside it.

## Recent Evolution Index

- [2026-08-02 13:57](logs/2026-08/2026-08-02-1357.md) — First run. CHIP-8 Phase 4 complete. All 35 instructions, SDL3 platform layer, IBM Logo verified. Delay Timer Test failing. Feedback extracted: teaching style, don't do work for user.

## Run State

```yaml
project_root: "D:\\vajs\\game"
timezone: "Asia/Hong_Kong"
last_evolved_at: "2026-08-02T13:57:00+08:00"
last_successful_run: "2026-08-02T13:57:00+08:00"
last_run_status: "completed"
processed_sessions:
  - "claude:fd9b230a-546b-4420-a75e-5ecd545449b3"
pending_sessions: []
open_risks:
  - "Delay Timer Test ROM shows no visual change — timer function unverified"
  - "Multiple game ROMs (Pong/Breakout/Tetris/Space Invaders) not yet tested"
  - "User SDL3 platform layer understanding is incomplete"
project_names:
  - "game"
  - "chip8"
  - "chip-8"
keywords:
  - "chip8"
  - "chip-8"
  - "模拟器"
  - "emulator"
  - "SDL3"
session_sources: "all"
automation_exclude_texts: []
notes: "First evolution run. CHIP-8 emulator Phase 4 complete. All 35 instructions implemented. SDL3 platform layer integrated. IBM Logo ROM verified. Delay Timer Test ROM failing — needs debugging."
```

## State Field Notes

- `last_evolved_at`: last successful evolution scan waterline.
- `processed_sessions`: sessions already included in evolution output.
- `pending_sessions`: relevant sessions found but not fully processed.
- `open_risks`: active evolution risks that still need attention.
