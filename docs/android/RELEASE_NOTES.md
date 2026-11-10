# Hokkaido 0.6.2

Tested on a Samsung Galaxy S26 (Exynos 2600, Xclipse 960, 12 GB) and a Galaxy S24 (Exynos 2400, Xclipse 940, 8 GB), both on Android 17.

## Highlights

- **Fire Emblem: Fortune's Weave is playable.**
  - The game's DLSS is replaced again on its current update. The replacement had stopped recognizing it, so the game's own DLSS network ran: ghosting around moving characters, and on the custom RADV driver a black screen with the whole phone stuttering.
  - The replacement now upscales over time with the game's motion vectors (FSR 3.1) by default on Android, so the picture is stable and sharp even at 0.5x.
  - It presents a frame ahead when "Present a frame ahead" is on Auto.
  - It no longer stalls on GPU occlusion queries that the Xclipse driver never answers.
- **Orbitals** gets past its logos (port of an upstream fix for Unreal Engine's frame timing). Gameplay is still black.
- **Language lists fixed.** Choosing a language in System → Language (the language games use) or the app's own language list could pick a different one, for example Spanish or Portuguese. Every entry now picks its own language.
- **Game folders**
  - Loose Switch 2 dumps (an `exefs` folder with `main`): Hokkaido offers "All files access" when a game folder holds one, then lists the game.
  - Game folders get a `.nomedia` file, so Android doesn't index thousands of game files and stall the game list. The shared media folders (Download, Pictures and others) are left alone.
  - The game list no longer walks through a dump's `romfs` and `exefs` folders.
- **Less work per frame for the GPU thread:** the game's writes to GPU memory are joined page by page before the caches see them (from xDarkWindx/eden-fefw).

## Known issues

- Orbitals shows a black screen after its logos.
- Fast Fusion launches but does not get further.
- Ocarina of Time sometimes starts with graphical glitches; restarting the game helps. On a 12 GB phone it can still be closed for low memory on the first start while shaders compile. Use **0.5x**.
- Pokopia's sky is black on Xclipse phones.
- Microphone input is not supported on Android.
- The phone heats up during long sessions and slows down until it cools.

## For testers

- Install the APK, add your games folder, and set up keys and firmware as usual. None are included.
- Recommended: Handheld mode, NCE CPU backend, 0.5x resolution, and a custom RADV driver on Xclipse GPUs.
- New diagnostic switches: `EDEN_NX2_WAIT_TRACE=1` names what a stuck game thread waits on, and `EDEN_NX2_COMPUTE_KEYS=1` lists compute program keys. Set them in `Android/data/dev.lichii.android/files/env.txt` (see the README).
- Report problems at https://github.com/pavelgajdo/Hokkaido/issues with your phone, driver and `eden_log.txt`.
