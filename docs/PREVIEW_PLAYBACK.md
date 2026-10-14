# Preview playback (Development Task 09)

`Tokkebi` owns `PreviewEngine`; `AppShell` only publishes a Ready `AudioDocument` snapshot and issues transport commands. `ProcessBlock()` alone renders preview samples. Transport states are Unavailable, Stopped, Playing and Paused. Play resumes Paused, Stop resets to zero, Return resets without changing Playing/Paused state, Seek is frame-authoritative, and EOF becomes Stopped at source end.

The control thread retains every published immutable `shared_ptr<const DecodedPcm>` for the processor lifetime, then publishes only its raw address atomically. The audio callback never changes ownership or frees memory, locks, allocates, reads files, logs, or calls UI. New loading requests clear the published source immediately; a failed replacement remains unavailable.

Renderer output is linear-interpolated at `sourceRate / outputRate`; mono is duplicated to all output channels and stereo maps L/R. Gain is clamped to 0..2 and mute contributes zero. Standalone starts from silence and adds preview only; plugin builds first preserve input passthrough and then add preview. Mixing is provisional technical validation; `OPEN-AUDIO-01` remains OPEN pending real DAW validation. There is no loop, selection, export, device-settings UI, or host QA claim.
