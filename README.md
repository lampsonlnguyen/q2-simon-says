# Diamond Says — Q2 Pod

An original Simon-style memory game for the Q2 Pod, using **Q2 Orange, Champagne Gold, Black, and Silver**.

## Install (same as Dino Runner)

1. Download **q2-diamond-says.zip** from [Releases](https://github.com/lampsonlnguyen/q2-simon-says/releases). **Do not download GitHub's “Source code (zip)”**: it contains source files, not the compiled game.
2. Extract **the contents** of `q2-diamond-says.zip` directly to the **root of your microSD card**, the same way as Dino Runner. The archive contains a hidden folder named `.diamond-says`.
3. Confirm the files appear at `/mnt/mmc/.diamond-says/q2app` and `/mnt/mmc/.diamond-says/q2-diamond-says` on the device (the folder may be hidden by Windows/macOS; enable viewing hidden files).
4. Safely eject your microSD, insert it into the Q2 Pod, then open **Library → Games → Diamond Says**. Reopen the Games list or restart the Q2 if the firmware cached the list.

**Expected card layout:**
```text
microSD/
  .diamond-says/
    q2app                 # name=Diamond Says, game=1
    q2-diamond-says       # compiled MIPS executable
    LICENSE.txt
```

Do not put the extracted directory under a second folder such as `q2-diamond-says/` or leave the game as a ZIP on the card. Do not rename the `.diamond-says` directory.

## New arcade effects

- Four equal-sized, wide rounded rectangular buttons inside a compact rounded case, using Q2 Orange, Champagne Gold, Black and Silver. The center medallion contains an original gray tabby with green eyes, a white muzzle, pink ears, and a faceted blue diamond held between its paws, based on the approved design.
- Animated color brightness, a glow halo and expanding ripple when a pad activates.
- Gold particle bursts and a dedicated status strip for level-up and game-over messages (never covering the cat). Round number is displayed together with its label.
- Progress markers, plus progressively faster playback as the sequence grows.
- Sound toggle: tap the **SOUND ON/OFF label at the bottom** to switch MCU click effects on/off.

### Sound compatibility

This build uses the Q2's built-in MCU **buzzer click**, via the firmware's
`cmd_mcu write_str buzzer` command, rather than including audio samples.
This is a simple click with **one pitch**, not four separate musical notes.
It uses no external audio framework and does not take over headphone audio.
The buzzer is best tested on real Q2 hardware; the desktop emulator may
not reproduce MCU clicks. Sound defaults to on and can be disabled in game.

## Controls

- **Center (press):** start, select a highlighted pad, restart, resume.
- **Wheel:** cycle through the four pads.
- **Touch:** tap a color pad directly after the pattern finishes; touch-down activates immediately.
- **Return (press):** pause/resume; from title or game-over, exit.

Watch the flashing rounded-square pads around the cat, then repeat the sequence. Each successful round adds a color. Best score saves to `/mnt/mmc/.diamond-says/best` (if the SD card is writable). Maximum sequence length: 128.

## Build from source

On Ubuntu or WSL:

```sh
sudo apt install clang lld make gcc zip
make check
make
```

The installable package is `dist/q2-diamond-says.zip`. GitHub Actions tests and builds on every push and uploads an Actions artifact; every push to `main` also publishes the ZIP to the **latest** GitHub Release. Tagged versions (`v*`) publish versioned Releases.

GPL-3.0-only. Original game code; no proprietary Simon audio or graphics.
