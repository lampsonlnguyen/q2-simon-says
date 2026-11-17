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

## Controls

- **Center (press):** start, select a highlighted pad, restart, resume.
- **Wheel:** cycle through the four pads.
- **Touch:** tap a color pad directly after the pattern finishes; touch-down activates immediately.
- **Return (press):** pause/resume; from title or game-over, exit.

Watch the sequence, then repeat it. Each successful round adds a color. Best score saves to `/mnt/mmc/.diamond-says/best` (if the SD card is writable). Maximum sequence length: 128.

## Build from source

On Ubuntu or WSL:

```sh
sudo apt install clang lld make gcc zip
make check
make
```

The installable package is `dist/q2-diamond-says.zip`. GitHub Actions tests and builds on every push and uploads an Actions artifact; every push to `main` also publishes the ZIP to the **latest** GitHub Release. Tagged versions (`v*`) publish versioned Releases.

GPL-3.0-only. Original game code; no proprietary Simon audio or graphics.
