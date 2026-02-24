# Textsnippets

A Linux text snippet manager that runs as a system tray application. Snippets can be organized hierarchically in groups and inserted into the active window via the tray menu.

## Features

- **System tray icon** with dynamic context menu (group structure as submenus)
- **Manager dialog** with TreeView + Editor (caption + multi-line text)
- **Hierarchical groups** with unlimited nesting depth
- **Automatic pasting** via clipboard + simulated Ctrl+V (`xdotool`)
- **Persistence** as JSON file (`~/.config/textsnippets/snippets.json`)
- **Multilingual** – supports English, German, Spanish, French, Italian, Hindi, Chinese, and Russian
- Compatible with **GNOME, KDE, XFCE, MATE, Cinnamon** and others

## Dependencies

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config \
    libgtk-3-dev libayatana-appindicator3-dev \
    nlohmann-json3-dev xdotool xclip
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake pkg-config \
    gtk3-devel libayatana-appindicator-gtk3-devel \
    nlohmann-json-devel xdotool xclip
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake pkg-config \
    gtk3 libappindicator-gtk3 \
    nlohmann-json xdotool xclip
```

> **Note:** If `nlohmann-json` is not available as a system package, it will be downloaded automatically via CMake FetchContent. An internet connection is required for the first build.

## Build

```bash
# In the project directory:
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The resulting binary is located at `build/textsnippets`.

## Running

```bash
./build/textsnippets
```

The application starts minimized in the system tray (no visible window). An icon appears in the taskbar.

### Autostart

The `.deb` package automatically installs an autostart entry in `/etc/xdg/autostart/`. For manual setup:

```bash
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/textsnippets.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Textsnippets
Exec=textsnippets
Icon=accessories-text-editor
X-GNOME-Autostart-enabled=true
EOF
```

## Usage

1. **Right-/left-click** on the tray icon opens the context menu
2. **Groups** are displayed as submenus
3. **Click on a snippet** → text is copied to the clipboard and automatically pasted (Ctrl+V)
4. **"Manage Snippets…"** → opens the editor dialog
5. **"Settings…"** → change the application language
6. **"Quit"** → exits the application

### Editor Dialog

- **TreeView** (left): expand/collapse groups, select entries
- **Editor** (right): edit caption and text
- **Save**: apply changes (JSON is written immediately)
- **Discard**: reset fields to the last saved state
- **+ Group**: create a new group
- **+ Snippet**: create a new text snippet
- **Delete**: delete the selected entry (with confirmation)
- **Rename**: rename a group (via dialog)

## Data Format

Snippets are stored in `~/.config/textsnippets/snippets.json`:

```json
{
  "groups": [
    {
      "name": "Greetings",
      "children": [],
      "snippets": [
        {
          "caption": "Greeting",
          "text": "Dear Sir or Madam,\n\nThank you for your message."
        }
      ]
    }
  ]
}
```

## Known Limitations

### Wayland

Under **Wayland**, `xdotool` does not work reliably because Wayland does not allow sending keyboard input globally to other windows. In this case:

- The text is **only copied to the clipboard**
- A desktop notification informs the user
- Manual pasting with **Ctrl+V** is required

Affected desktop environments in Wayland mode: GNOME (default since Ubuntu 21.04), KDE Plasma 6.

**Workaround:** Start the desktop session in X11/Xorg mode.

### AppIndicator Support

Some desktop environments require an extension for system tray icons:

- **GNOME**: Install the [AppIndicator extension](https://extensions.gnome.org/extension/615/appindicator-support/)
- **KDE / XFCE / MATE / Cinnamon**: Usually works out of the box

### Drag & Drop

Drag and drop for moving snippets between groups is not yet implemented.

## Project Structure

```
├── CMakeLists.txt              Build configuration
├── Dockerfile.amd64            Docker build for amd64 cross-compilation
├── README.md                   This file
├── data/
│   ├── textsnippets.desktop            Application menu entry
│   └── textsnippets-autostart.desktop  Autostart entry
└── src/
    ├── main.cpp                Entry point, GTK init, settings dialog
    ├── tray.h / tray.cpp       Tray icon and menu construction
    ├── manager_dialog.h/.cpp   Manager dialog (TreeView + Editor)
    ├── snippet_store.h/.cpp    Data model, JSON load/save
    ├── clipboard.h/.cpp        Clipboard and paste mechanism
    └── i18n.h / i18n.cpp       Internationalization (8 languages)
```

## License

Copyright 2026 Dennis Michael Heine. Licensed under the Apache License, Version 2.0.
