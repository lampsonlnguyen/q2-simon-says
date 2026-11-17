# Diamond Says — Q2 Pod
Original Simon-style memory game honoring DiamondBond, with four custom colors: **Q2 Orange**, **Champagne Gold**, **Black**, and **Silver**.

## Controls
- **Center:** start, select a highlighted pad, restart, resume.
- **Wheel:** cycle through the four pads.
- **Touch:** tap a pad after the pattern finishes.
- **Return:** pause/resume; from title or game-over, exit.

Watch the sequence, then repeat it. Each completed round adds one color. Best score saves to `/mnt/mmc/.diamond-says/best` if that directory is present. Maximum sequence length is 128.

## Build
On Ubuntu: `sudo apt install clang lld make gcc zip` then `make check && make`. Install `dist/q2-diamond-says.zip` through the Q2 app manager or extract its hidden `.diamond-says` directory to the SD card root.

## Releases
GitHub Actions tests and builds on every push. Push a `v*` tag to publish the ZIP as a Release asset.
No copyrighted Simon branding, sounds or artwork included. GPL-3.0-only.
