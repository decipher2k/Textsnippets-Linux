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
#include <string>

// ═══════════════════ Internal state ═══════════════════

static bool s_xdotool_available = false;
static bool s_wtype_available   = false;
static bool s_ydotool_available = false;
static bool s_wlcopy_available  = false;
static bool s_wayland           = false;
static bool s_initialized       = false;

/// Check whether a command-line tool is in PATH.
static bool check_command(const char* cmd) {
    char buf[256];
    std::snprintf(buf, sizeof(buf), "which %s > /dev/null 2>&1", cmd);
    return std::system(buf) == 0;
}

// ═══════════════════ Public API ═══════════════════

void clipboard_init() {
    if (s_initialized) return;
    s_initialized = true;

    const char* session = std::getenv("XDG_SESSION_TYPE");
    s_wayland = (session && std::strcmp(session, "wayland") == 0);

    s_xdotool_available = check_command("xdotool");
    s_wtype_available   = check_command("wtype");
    s_ydotool_available = check_command("ydotool");
    s_wlcopy_available  = check_command("wl-copy");

    if (s_wayland) {
        if (!s_wtype_available && !s_ydotool_available)
            g_warning("Wayland session: neither wtype nor ydotool found – "
                      "text will only be copied to the clipboard.");
    } else {
        if (!s_xdotool_available)
            g_warning("xdotool not found – text will only be copied to the clipboard.");
    }
}

bool is_paste_tool_available() {
    if (s_wayland)
        return s_wtype_available || s_ydotool_available;
    return s_xdotool_available;
}

bool is_wayland_session() { return s_wayland; }

// ─────────── Delayed Ctrl+V keystroke ───────────

static gboolean do_paste_x11(gpointer /*unused*/) {
    GError* err = nullptr;
    g_spawn_command_line_async("xdotool key --clearmodifiers ctrl+v", &err);
    if (err) {
        g_warning("xdotool error: %s", err->message);
        g_error_free(err);
    }
    return G_SOURCE_REMOVE;
}

static gboolean do_paste_wtype(gpointer /*unused*/) {
    GError* err = nullptr;
    g_spawn_command_line_async("wtype -M ctrl -P v -p v -m ctrl", &err);
    if (err) {
        g_warning("wtype error: %s", err->message);
        g_error_free(err);
    }
    return G_SOURCE_REMOVE;
}

static gboolean do_paste_ydotool(gpointer /*unused*/) {
    GError* err = nullptr;
    g_spawn_command_line_async("ydotool key 29:1 47:1 47:0 29:0", &err);
    if (err) {
        g_warning("ydotool error: %s", err->message);
        g_error_free(err);
    }
    return G_SOURCE_REMOVE;
}

// ─────────── Notification (fallback) ───────────

static void show_clipboard_notification() {
    char cmd[512];
    std::snprintf(cmd, sizeof(cmd),
        "notify-send '%s' '%s'",
        tr("notif_title"), tr("notif_clipboard"));
    g_spawn_command_line_async(cmd, nullptr);
}

// ─────────── Set clipboard under Wayland via wl-copy ───────────

static void set_wayland_clipboard(const std::string& text) {
    // Use wl-copy if available for reliable Wayland clipboard
    if (!s_wlcopy_available) return;

    int stdin_fd;
    GError* err = nullptr;
    gchar* argv[] = {
        const_cast<gchar*>("wl-copy"),
        const_cast<gchar*>("--"),
        nullptr
    };

    GPid pid;
    gboolean ok = g_spawn_async_with_pipes(
        nullptr, argv, nullptr,
        static_cast<GSpawnFlags>(G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD),
        nullptr, nullptr, &pid,
        &stdin_fd, nullptr, nullptr, &err);

    if (ok) {
        // Write text to wl-copy's stdin
        (void)write(stdin_fd, text.c_str(), text.size());
        close(stdin_fd);
        g_spawn_close_pid(pid);
    } else {
        if (err) {
            g_warning("wl-copy error: %s", err->message);
            g_error_free(err);
        }
    }
}

// ─────────── Main function ───────────

bool insert_snippet(const std::string& text) {
    // 1. Copy to clipboard
    GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, text.c_str(), static_cast<gint>(text.size()));
    gtk_clipboard_store(cb);

    // Under Wayland, also use wl-copy for reliable clipboard access
    if (s_wayland && s_wlcopy_available) {
        set_wayland_clipboard(text);
    }

    // 2. Simulate paste
    if (s_wayland) {
        // Wayland: prefer wtype, fall back to ydotool
        if (s_wtype_available) {
            g_timeout_add(250, do_paste_wtype, nullptr);
            return true;
        } else if (s_ydotool_available) {
            g_timeout_add(250, do_paste_ydotool, nullptr);
            return true;
        }
    } else {
        // X11: use xdotool
        if (s_xdotool_available) {
            g_timeout_add(250, do_paste_x11, nullptr);
            return true;
        }
    }

    // 3. Fallback: clipboard only + notification
    show_clipboard_notification();
    return false;
}
