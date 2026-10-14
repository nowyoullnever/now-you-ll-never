# t o k k e b i

**Status: P02 Audio Editing — canonical 34-task roadmap restored.**

`tokkebi` is a cross-platform audio collection and sample-library product in development.

Current merged implementation:

- Through Development Task 08: local loading, WAV decoding, audio information and async loading.
- Substantial Development Task 10 waveform foundation was implemented early in merged PR #8.

Next planned functional task:

- Development Task 09 — Playback / Stop / Volume / Audio output.

Parked:

- PR #9 — early Development Task 10 selection foundation. It is Draft and must not merge before Task 09 is completed and externally reviewed.

AIFF/AIFC and FLAC are recognized but unsupported. Preview, playhead, remaining selection/IN/OUT interactions, conversion/export, persistence, acquisition and Helper IPC are not implemented.

Repository: <https://github.com/nowyoullnever/tokkebi>

## Canonical documents

- [MASTER_BLUEPRINT.md](MASTER_BLUEPRINT.md): product scope and requirements.
- [AGENTS.md](AGENTS.md): repository workflow and review rules.
- [Build instructions](docs/BUILD.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Dependencies](docs/DEPENDENCIES.md)
- [Font inventory, source, licensing and coverage boundary](docs/FONTS.md)
- [Theme tokens and accessibility](docs/THEMES.md)
- [UI shell and navigation](docs/UI_SHELL.md)
- [UI components](docs/UI_COMPONENTS.md)
- [Local audio decoder](docs/AUDIO_DECODER.md)
- [Waveform foundation](docs/WAVEFORM.md)
- [Preview playback](docs/PREVIEW_PLAYBACK.md)

The product name is final for this repository. The application release remains `0.1.0`; P00 task numbers are implementation-stage labels, not release versions. `OPEN-FONT-04` remains unresolved.
