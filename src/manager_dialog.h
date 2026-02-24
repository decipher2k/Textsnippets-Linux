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

// manager_dialog.h – Verwaltungsdialog (TreeView + Editor)
#ifndef MANAGER_DIALOG_H
#define MANAGER_DIALOG_H

#include "snippet_store.h"

#include <functional>
#include <optional>
#include <vector>

#include <gtk/gtk.h>

/// Spalten des GtkTreeStore
enum TreeColumns {
    COL_ICON_NAME = 0, ///< Icon-Name (z. B. "folder", "text-x-generic")
    COL_DISPLAY_NAME,  ///< Anzeigename (Gruppen-Name oder Caption)
    COL_IS_GROUP,      ///< TRUE = Gruppe, FALSE = Textbaustein
    NUM_COLS
};

/**
 * GTK-Fenster für die Verwaltung der Textbausteine.
 *
 * Aufbau:
 *   Links  – TreeView mit Gruppenstruktur
 *   Rechts – Editor (Caption + Text)
 *   Unten  – Toolbar (+ Gruppe, + Baustein, Löschen, Umbenennen)
 */
class ManagerDialog {
public:
    ManagerDialog(SnippetStore& store,
                  std::function<void()> on_data_changed);
    ~ManagerDialog();

    void show();
    void hide();
    bool is_visible() const;

private:
    SnippetStore&          m_store;
    std::function<void()>  m_on_data_changed;

    // ── Widgets ──
    GtkWidget*    m_window         = nullptr;
    GtkWidget*    m_tree_view      = nullptr;
    GtkTreeStore* m_tree_store     = nullptr;
    GtkWidget*    m_caption_entry  = nullptr;
    GtkWidget*    m_text_view      = nullptr;
    GtkWidget*    m_save_btn       = nullptr;
    GtkWidget*    m_discard_btn    = nullptr;
    GtkWidget*    m_caption_label  = nullptr;
    GtkWidget*    m_text_label     = nullptr;
    GtkWidget*    m_text_scroll    = nullptr;
    GtkWidget*    m_add_snippet_btn= nullptr;
    GtkWidget*    m_delete_btn     = nullptr;
    GtkWidget*    m_rename_btn     = nullptr;

    bool m_populating = false; ///< Unterdrückt selection-changed während populate

    // ── UI-Aufbau ──
    void build_ui();
    void populate_tree();
    void populate_tree_recurse(const std::vector<Group>& groups,
                               GtkTreeIter* parent);

    // ── Editor ──
    void update_editor();
    void clear_editor();

    // ── Hilfsfunktionen ──
    std::vector<int>        get_selected_path();
    std::optional<NodeInfo> get_selected_node();
    void                    restore_selection(const std::vector<int>& path);
    void                    data_changed();  ///< Speichern + UI aktualisieren

    // ── GTK-Callbacks ──
    static void     on_selection_changed(GtkTreeSelection* sel, gpointer data);
    static void     on_save_clicked     (GtkButton* btn, gpointer data);
    static void     on_discard_clicked  (GtkButton* btn, gpointer data);
    static void     on_add_group_clicked(GtkButton* btn, gpointer data);
    static void     on_add_snippet_clicked(GtkButton* btn, gpointer data);
    static void     on_delete_clicked   (GtkButton* btn, gpointer data);
    static void     on_rename_clicked   (GtkButton* btn, gpointer data);
    static gboolean on_window_delete    (GtkWidget* w, GdkEvent* e, gpointer d);
};

#endif // MANAGER_DIALOG_H
