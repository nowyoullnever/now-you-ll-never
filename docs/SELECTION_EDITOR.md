# P02.10 selection editor

The editor stores selection only as source-frame indices using the half-open interval `[inFrame, outFrame)`. A new source request clears selection and transient history immediately. New ready audio begins with no selection; Select All is explicit.

`SelectionState` is independent of IGraphics and validates every committed edit. Time fields use canonical `HH:MM:SS.mmm`, reject malformed or overflowing values, convert with integer arithmetic, and never make display milliseconds authoritative. The bounded, instance-local history retains at most 100 committed selection changes; it is not persisted to History JSON.

INBOX currently exposes Select All (`Ctrl/Cmd+A`), Clear (`Escape`), Undo (`Ctrl/Cmd+Z`), Redo (`Ctrl/Cmd+Shift+Z`) and selection tint. Playback, playhead, preview, export and source mutation are not part of P02.10.
