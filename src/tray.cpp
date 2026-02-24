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

// tray.cpp – Tray icon logic and menu construction
#include "tray.h"
#include "clipboard.h"
#include "i18n.h"

#include <cstring>

// ═══════════════════ Konstruktor / Destruktor ═══════════════════

TrayIcon::TrayIcon(SnippetStore& store,
                   std::function<void()> on_manage,
                   std::function<void()> on_quit)
    : m_store(store),
      m_indicator(nullptr),
      m_on_manage(std::move(on_manage)),
      m_on_quit(std::move(on_quit))
{
    m_indicator = app_indicator_new(
        "textsnippets",
        "accessories-text-editor",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    app_indicator_set_status(m_indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_title(m_indicator, tr("app_title"));

    rebuild_menu();
}

TrayIcon::~TrayIcon() {
    if (m_indicator)
        g_object_unref(m_indicator);
}

// ═══════════════════ Menü (neu) aufbauen ═══════════════════

void TrayIcon::rebuild_menu() {
    GtkWidget* menu = build_menu();
    app_indicator_set_menu(m_indicator, GTK_MENU(menu));
    gtk_widget_show_all(menu);
}

GtkWidget* TrayIcon::build_menu() {
    GtkWidget* menu = gtk_menu_new();

    // ── Gruppenstruktur ──
    for (const auto& group : m_store.groups()) {
        GtkWidget* group_item = gtk_menu_item_new_with_label(group.name.c_str());
        GtkWidget* submenu    = gtk_menu_new();
        build_group_submenu(submenu, group);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(group_item), submenu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), group_item);
    }

    // ── Separator ──
    gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                          gtk_separator_menu_item_new());

    // ── "Manage Snippets…" ──
    GtkWidget* manage_item =
        gtk_menu_item_new_with_label(tr("tray_manage"));
    g_signal_connect(manage_item, "activate",
                     G_CALLBACK(on_manage_activated), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), manage_item);

    // ── "Settings…" ──
    GtkWidget* settings_item =
        gtk_menu_item_new_with_label(tr("tray_settings"));
    g_signal_connect(settings_item, "activate",
                     G_CALLBACK(on_settings_activated), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), settings_item);

    // ── "Quit" ──
    GtkWidget* quit_item = gtk_menu_item_new_with_label(tr("tray_quit"));
    g_signal_connect(quit_item, "activate",
                     G_CALLBACK(on_quit_activated), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    return menu;
}

// Rekursiver Aufbau der Untermenüs für eine Gruppe.
void TrayIcon::build_group_submenu(GtkWidget* menu, const Group& group) {
    // Untergruppen zuerst
    for (const auto& child : group.children) {
        GtkWidget* child_item = gtk_menu_item_new_with_label(child.name.c_str());
        GtkWidget* submenu    = gtk_menu_new();
        build_group_submenu(submenu, child);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(child_item), submenu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), child_item);
    }

    // Optionaler Separator zwischen Untergruppen und Bausteinen
    if (!group.children.empty() && !group.snippets.empty())
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),
                              gtk_separator_menu_item_new());

    // Textbausteine
    for (const auto& snippet : group.snippets) {
        GtkWidget* item = gtk_menu_item_new_with_label(snippet.caption.c_str());

        // Snippet-Text als GObject-Daten anhängen (wird mit g_free freigegeben)
        g_object_set_data_full(G_OBJECT(item), "snippet-text",
                               g_strdup(snippet.text.c_str()), g_free);

        g_signal_connect(item, "activate",
                         G_CALLBACK(on_snippet_activated), nullptr);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

    // Placeholder for empty groups
    if (group.children.empty() && group.snippets.empty()) {
        GtkWidget* empty = gtk_menu_item_new_with_label(tr("tray_empty"));
        gtk_widget_set_sensitive(empty, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), empty);
    }
}

// ═══════════════════ Signal-Callbacks ═══════════════════

void TrayIcon::on_snippet_activated(GtkMenuItem* item, gpointer /*unused*/) {
    const char* text =
        static_cast<const char*>(g_object_get_data(G_OBJECT(item), "snippet-text"));
    if (text)
        insert_snippet(std::string(text));
}

void TrayIcon::on_manage_activated(GtkMenuItem* /*item*/, gpointer user_data) {
    auto* self = static_cast<TrayIcon*>(user_data);
    if (self->m_on_manage)
        self->m_on_manage();
}

void TrayIcon::on_quit_activated(GtkMenuItem* /*item*/, gpointer user_data) {
    auto* self = static_cast<TrayIcon*>(user_data);
    if (self->m_on_quit)
        self->m_on_quit();
}

void TrayIcon::on_settings_activated(GtkMenuItem* /*item*/, gpointer user_data) {
    auto* self = static_cast<TrayIcon*>(user_data);
    if (self->m_on_settings)
        self->m_on_settings();
}
