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

// clipboard.cpp – Clipboard and simulated paste
#include "clipboard.h"
#include "i18n.h"

#include <gtk/gtk.h>

#include <cstdlib>
#include <cstring>

// ═══════════════════ Interne Zustandsvariablen ═══════════════════

static bool s_xdotool_available = false;
static bool s_wayland           = false;
static bool s_initialized       = false;

// Prüft, ob ein Kommandozeilen-Tool im PATH liegt.
static bool check_command(const char* cmd) {
    char buf[256];
    std::snprintf(buf, sizeof(buf), "which %s > /dev/null 2>&1", cmd);
    return std::system(buf) == 0;
}

// ═══════════════════ Öffentliches API ═══════════════════

void clipboard_init() {
    if (s_initialized) return;
    s_initialized = true;

    s_xdotool_available = check_command("xdotool");

    const char* session = std::getenv("XDG_SESSION_TYPE");
    s_wayland = (session && std::strcmp(session, "wayland") == 0);

    if (!s_xdotool_available)
        g_warning("xdotool nicht gefunden – Text wird nur in die "
                  "Zwischenablage kopiert.");
    if (s_wayland)
        g_warning("Wayland-Sitzung erkannt – xdotool-Einfügen kann "
                  "eingeschränkt sein.");
}

bool is_xdotool_available() { return s_xdotool_available; }
bool is_wayland_session()   { return s_wayland; }

// ─────────── Verzögerter Ctrl+V-Tastendruck ───────────

static gboolean do_paste(gpointer /*unused*/) {
    GError* err = nullptr;
    g_spawn_command_line_async("xdotool key --clearmodifiers ctrl+v", &err);
    if (err) {
        g_warning("xdotool-Fehler: %s", err->message);
        g_error_free(err);
    }
    return G_SOURCE_REMOVE; // Einmaliger Timer
}

// ─────────── Benachrichtigung (Fallback) ───────────

static void show_clipboard_notification() {
    char cmd[512];
    std::snprintf(cmd, sizeof(cmd),
        "notify-send '%s' '%s'",
        tr("notif_title"), tr("notif_clipboard"));
    g_spawn_command_line_async(cmd, nullptr);
}

// ─────────── Hauptfunktion ───────────

bool insert_snippet(const std::string& text) {
    // 1. In GTK-Zwischenablage kopieren
    GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, text.c_str(), static_cast<gint>(text.size()));
    gtk_clipboard_store(cb); // überlebt App-Beendigung via Clipboard-Manager

    // 2. Einfügen simulieren (nur unter X11 mit xdotool)
    if (s_xdotool_available && !s_wayland) {
        // 250 ms Verzögerung, damit das vorherige Fenster den Fokus zurückerhält
        g_timeout_add(250, do_paste, nullptr);
        return true;
    }

    // 3. Fallback: Nur Zwischenablage + Benachrichtigung
    show_clipboard_notification();
    return false;
}
