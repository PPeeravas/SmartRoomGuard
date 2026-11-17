# Setup


## NUCLEO
TODO: Peem add instruction for nucleo project setup


## NodeMCU (ESP32-S3)

Project: `esp32/` (VS Code + PlatformIO, Arduino framework)

**Installation**
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
**Build and Upload**
1. Go to `PIO Home → Quick Access: Open Project → SmartRoomGuard/esp32`
2. Open any project file (e.g. `src/main.cpp`). Buttons at the editor's **top-right**
   - `✓` Build (Ctrl+Alt+B) (first build downloads the toolchain, a few minutes)
   - `˅` next to ✓ → `Upload` (Ctrl+Alt+U)
   - `🔌` Serial Monitor
   - [Optional] Same actions: 👽 PlatformIO sidebar → Project Tasks → esp32-s3-devkitc-1 → General → click the menu
3. Plug in the board using the USB-C port labelled `COM` \
   Build → Upload → 🔌 Serial Monitor
