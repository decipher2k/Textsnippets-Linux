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

// tray.h – System tray icon and dynamic context menu
#ifndef TRAY_H
#define TRAY_H

#include "snippet_store.h"

#include <functional>
#include <gtk/gtk.h>

#ifdef USE_AYATANA_APPINDICATOR
#include <libayatana-appindicator/app-indicator.h>
#else
#include <libappindicator/app-indicator.h>
#endif

/**
 * Erzeugt ein AppIndicator-Tray-Icon mit einem Kontextmenü, das die
 * Gruppenstruktur der Textbausteine als verschachtelte Untermenüs abbildet.
 */
class TrayIcon {
public:
    TrayIcon(SnippetStore& store,
             std::function<void()> on_manage,
             std::function<void()> on_quit);
    ~TrayIcon();

    /// Rebuild menu completely (e.g. after changes in the editor).
    void rebuild_menu();

    /// Set callback for settings menu item
    void set_on_settings(std::function<void()> cb) { m_on_settings = std::move(cb); }

private:
    SnippetStore&          m_store;
    AppIndicator*          m_indicator;
    std::function<void()>  m_on_manage;
    std::function<void()>  m_on_quit;
    std::function<void()>  m_on_settings;

    GtkWidget* build_menu();
    void       build_group_submenu(GtkWidget* menu, const Group& group);

    // GTK-Signal-Callbacks (statisch, da C-Funktionszeiger erwartet)
    static void on_snippet_activated (GtkMenuItem* item, gpointer user_data);
    static void on_manage_activated  (GtkMenuItem* item, gpointer user_data);
    static void on_quit_activated    (GtkMenuItem* item, gpointer user_data);
    static void on_settings_activated(GtkMenuItem* item, gpointer user_data);
};

#endif // TRAY_H
