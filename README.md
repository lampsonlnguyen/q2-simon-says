# Diamond Says — Q2 Pod

An original Simon-style memory game for the Q2 Pod, using **Q2 Orange, Champagne Gold, Black, and Silver**.

## A tribute to DiamondBond

**Diamond Says** was created to honor **DiamondBond** and celebrate all the awesomeness, dedication, and hard work they have put into the Q2 Pod community. Their contributions and passion for improving the Q2 Pod inspired this small game. Thank you, DiamondBond, for everything you do! The mascot thinking cat in the middle button was based on Diamond's avatar in Q2 Pod discord server.

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

## Look

Diamond Says is drawn the way Q2 Pod draws its own games (Snake, Vortex): **Round** and **Best** along the top, a flat board in a hairline frame, and the theme's own pop-up for the title, the pause menu and the game over. On Q2 Pod firmware that passes `Q2_POPUP`, the game takes its colours and pop-up style from your theme (Minimal, Modern or Classic, in Light or Dark Mode); older firmware gets the game's own dark or light look from `[IPOD] LIGHT`. Text is set in Liberation Sans Bold, Q2 Pod's game-menu face. New UI is flat, as Q2 Pod asks: no gloss, glow or gradients.

- Four rounded pads in Q2 Orange, Champagne Gold, Black and Silver; a lit pad brightens, and the pad the wheel is on is ringed.
- The cat mascot sits in the middle hub, recoloured for Light and Dark Mode.
- Under the board, **Watch** / **Your turn** / **Level up!** and a dot for each step of the round.

### Sound compatibility

This build uses the Q2's built-in MCU **buzzer click**, via the firmware's
`cmd_mcu write_str buzzer` command, rather than including audio samples.
This is a simple click with **one pitch**, not four separate musical notes.
It uses no external audio framework and does not take over headphone audio.
The buzzer is best tested on real Q2 hardware; the desktop emulator may
not reproduce MCU clicks. Sound defaults to on; turn it off from the pause menu (**Sound: On/Off**).

The sprite is embedded as simple run-length color pairs in `src/cat_sprite.h` (no PNG decoder or extra runtime dependencies). The firmware theme is read from `[IPOD] LIGHT` in `/mnt/data/config.ini` when the game opens. Explicit Light/Dark modes are supported; automatic mode currently falls back to dark.

## Controls

- **Center (press):** start, select the ringed pad, pick a pause-menu row, play again.
- **Wheel:** cycle through the four pads; move through the pause menu.
- **Touch:** tap a color pad directly after the pattern finishes; tap a pause-menu row.
- **Return (press):** open the pause menu (**Continue**, **Sound**, **Quit**); press again to exit. From the title or game over, exit.

Watch the pads light up around the cat, then repeat the sequence. Each successful round adds a color. Best score saves to `/mnt/mmc/.diamond-says/best` (if the SD card is writable). Maximum sequence length: 128.

## Build from source

On Ubuntu or WSL:

```sh
sudo apt install clang lld make gcc zip
make check
make
```

The installable package is `dist/q2-diamond-says.zip`. GitHub Actions tests and builds on every push and uploads an Actions artifact; every push to `main` also publishes the ZIP to the **latest** GitHub Release. Version tags (`1.2.0` or `v1.2.0`) publish versioned Releases.

GPL-3.0-only. Original game code; no proprietary Simon audio or graphics.
