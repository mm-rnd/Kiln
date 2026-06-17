# Kiln

**Kiln** is a multiband processor built in JUCE/C++ — designed for surgical control over the character and dynamics of individual frequency bands without sacrificing phase coherence.

At its core is a **linear-phase crossover** that splits audio into three bands with no smearing or phase artifacts. Each band passes independently through a **VCA/optical-style compressor**, a **saturation stage** with independent control over odd and even harmonic content, and finally a shared **brickwall limiter** on the output.

## Features

- **Linear-phase crossover** — phase-coherent band splitting with no transient smearing
- **Per-band compression** — VCA and optical-style characters per band
- **Per-band saturation** — independent odd/even harmonic balance for tonal shaping
- **Brickwall output limiter** — transparent 16x oversampled limiting with lookahead
- **Multiband audio analyser** — real-time FFT display with harmonic scatter, gain reduction metering and limiter visualisation per band
- **Dual UI modes** — dark digital aesthetic (default) and a lighter analogue-style display, toggled via `CMD+SHIFT+U` (macOS) or `CTRL+SHIFT+U` (Windows/Linux)

## Built With

- [JUCE](https://juce.com) — C++ audio application framework
- CMake