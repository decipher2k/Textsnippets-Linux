// Copyright 2026 Dennis Michael Heine
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// clipboard.h – Zwischenablage-Logik und Einfügemechanismus
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>

/// Einmalig aufrufen (prüft xdotool, erkennt Wayland).
void clipboard_init();

/// Textbaustein einfügen: In Zwischenablage kopieren, dann ggf. Ctrl+V senden.
/// Gibt true zurück, wenn der Paste-Vorgang ausgelöst wurde.
bool insert_snippet(const std::string& text);

/// Prüft, ob xdotool verfügbar ist.
bool is_xdotool_available();

/// Prüft, ob eine Wayland-Sitzung aktiv ist.
bool is_wayland_session();

#endif // CLIPBOARD_H
