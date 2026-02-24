# Textbaustein-Manager

Ein Linux-Textbaustein-Manager, der als System-Tray-Anwendung läuft. Textbausteine können hierarchisch in Gruppen organisiert und per Klick im Tray-Menü in das aktive Fenster eingefügt werden.

## Features

- **System-Tray-Icon** mit dynamischem Kontextmenü (Gruppenstruktur als Untermenüs)
- **Verwaltungsdialog** mit TreeView + Editor (Caption + mehrzeiliger Text)
- **Hierarchische Gruppen** mit beliebiger Verschachtelungstiefe
- **Automatisches Einfügen** via Zwischenablage + simuliertem Ctrl+V (`xdotool`)
- **Persistenz** als JSON-Datei (`~/.config/textbaustein-manager/snippets.json`)
- Kompatibel mit **GNOME, KDE, XFCE, MATE, Cinnamon** u. a.

## Abhängigkeiten

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

> **Hinweis:** Falls `nlohmann-json` nicht als Systempaket verfügbar ist, wird es automatisch per CMake FetchContent heruntergeladen. Internetverbindung beim ersten Build erforderlich.

## Build

```bash
# Im Projektverzeichnis:
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Das fertige Binary liegt unter `build/textbaustein-manager`.

## Starten

```bash
./build/textbaustein-manager
```

Die Anwendung startet minimiert im System Tray (kein sichtbares Fenster). Ein Icon erscheint in der Taskleiste.

### Autostart einrichten

Erstellen Sie eine `.desktop`-Datei:

```bash
mkdir -p ~/.config/autostart
cat > ~/.config/autostart/textbaustein-manager.desktop <<EOF
[Desktop Entry]
Type=Application
Name=Textbaustein-Manager
Exec=/pfad/zu/textbaustein-manager
Icon=accessories-text-editor
X-GNOME-Autostart-enabled=true
EOF
```

## Benutzung

1. **Rechts-/Linksklick** auf das Tray-Icon öffnet das Kontextmenü
2. **Gruppen** werden als Untermenüs dargestellt
3. **Klick auf einen Textbaustein** → Text wird in die Zwischenablage kopiert und automatisch eingefügt (Ctrl+V)
4. **„Textbausteine verwalten…"** → Öffnet den Editor-Dialog
5. **„Beenden"** → Beendet die Anwendung

### Editor-Dialog

- **TreeView** (links): Gruppen auf-/zuklappen, Eintrag auswählen
- **Editor** (rechts): Caption und Text bearbeiten
- **Speichern**: Änderungen übernehmen (JSON wird sofort geschrieben)
- **Verwerfen**: Felder auf letzten Stand zurücksetzen
- **+ Gruppe**: Neue Gruppe anlegen
- **+ Baustein**: Neuen Textbaustein anlegen
- **Löschen**: Ausgewählten Eintrag löschen (mit Bestätigung)
- **Umbenennen**: Gruppe umbenennen (per Dialog)

## Datenformat

Die Textbausteine werden in `~/.config/textbaustein-manager/snippets.json` gespeichert:

```json
{
  "groups": [
    {
      "name": "Grüße",
      "children": [],
      "snippets": [
        {
          "caption": "Begrüßung",
          "text": "Sehr geehrte Damen und Herren,\n\nvielen Dank für Ihre Nachricht."
        }
      ]
    }
  ]
}
```

## Bekannte Einschränkungen

### Wayland

Unter **Wayland** funktioniert `xdotool` nicht zuverlässig, da Wayland kein globales Senden von Tastatureingaben an andere Fenster erlaubt. In diesem Fall:

- Der Text wird **nur in die Zwischenablage** kopiert
- Eine Desktop-Benachrichtigung informiert den Benutzer
- Manuelles Einfügen mit **Ctrl+V** ist erforderlich

Betroffene Desktop-Umgebungen im Wayland-Modus: GNOME (Standard ab Ubuntu 21.04), KDE Plasma 6.

**Workaround:** Starten Sie die Desktop-Sitzung im X11-/Xorg-Modus.

### AppIndicator-Unterstützung

Manche Desktop-Umgebungen benötigen eine Erweiterung für System-Tray-Icons:

- **GNOME**: [AppIndicator-Erweiterung](https://extensions.gnome.org/extension/615/appindicator-support/) installieren
- **KDE / XFCE / MATE / Cinnamon**: Funktioniert in der Regel ohne Zusatz

### Drag & Drop

Drag & Drop zum Verschieben von Bausteinen zwischen Gruppen ist derzeit nicht implementiert.

## Projektstruktur

```
├── CMakeLists.txt              Build-Konfiguration
├── README.md                   Diese Datei
└── src/
    ├── main.cpp                Einstiegspunkt, GTK-Init
    ├── tray.h / tray.cpp       Tray-Icon und Menüaufbau
    ├── manager_dialog.h/.cpp   Verwaltungsdialog (TreeView + Editor)
    ├── snippet_store.h/.cpp    Datenmodell, JSON laden/speichern
    └── clipboard.h/.cpp        Zwischenablage und Einfügemechanismus
```

## Lizenz

Dieses Projekt steht unter der MIT-Lizenz.
