# t o k k e b i

**Status: P02.9 — waveform peak-cache and rendering foundation.**

`tokkebi` is a cross-platform audio collection and sample-library product in development. P02.9.1 loads a selected WAV asynchronously, reports detected metadata, and builds a local immutable peak-cache waveform. AIFF/AIFC and FLAC are recognized but unsupported. It does not implement preview, playhead, selection, IN/OUT, conversion/export, persistence, acquisition or Helper IPC.

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

The product name is final for this repository. The application release remains `0.1.0`; P00 task numbers are implementation-stage labels, not release versions. `OPEN-FONT-04` remains unresolved.
