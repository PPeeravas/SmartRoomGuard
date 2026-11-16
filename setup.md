# Setup


## NUCLEO
TODO: Peem add instruction for nucleo project setup


## NodeMCU (ESP32-S3)

Project: `esp32/` (VS Code + PlatformIO, Arduino framework)

1. Install the VS Code extension `PlatformIO IDE`. Restart VS Code and wait for the 🏠 icon in the status bar.
2. Allow access to the USB serial port (one time only):
   ```bash
   # Linux — log out and back in afterwards
   sudo usermod -aG dialout $USER
   curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
   sudo udevadm control --reload-rules && sudo udevadm trigger

   # Windows
   ## Nothings todo
   ```
3. Go to File → Open Folder → `SmartRoomGuard/esp32`
4. Open any project file (e.g. `src/main.cpp`). Buttons at the editor's top-right:
   - `✓` Build (first build downloads the toolchain, a few minutes)
   - `˅` next to ✓ → `Upload`
   - `🔌` Serial Monitor
   - Same actions: 👽 PlatformIO sidebar → Project Tasks → esp32-s3-devkitc-1 → General
5. Plug in the board using the USB-C port labelled `COM`. Build → Upload → 🔌 Serial Monitor
