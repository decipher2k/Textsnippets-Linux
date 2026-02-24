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

// main.cpp – Entry point: GTK init, tray icon setup, main loop
#include "clipboard.h"
#include "i18n.h"
#include "manager_dialog.h"
#include "snippet_store.h"
#include "tray.h"

#include <gtk/gtk.h>
#include <memory>

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // ── Initialize i18n ──
    SnippetStore tmp_store;
    i18n_init(tmp_store.config_dir());

    // ── Initialize clipboard module ──
    clipboard_init();

    // ── Load data model ──
    SnippetStore store;
    store.load();

    // ── Manager dialog ──
    std::unique_ptr<TrayIcon>      tray;
    std::unique_ptr<ManagerDialog> dialog;

    dialog = std::make_unique<ManagerDialog>(
        store,
        [&tray]() {
            // Callback: data changed → rebuild tray menu
            if (tray) tray->rebuild_menu();
        });

    // ── System tray icon ──
    tray = std::make_unique<TrayIcon>(
        store,
        [&dialog]() {
            // "Manage Snippets…" clicked
            if (dialog) dialog->show();
        },
        []() {
            // "Quit" clicked
            gtk_main_quit();
        });

    // ── Settings dialog callback ──
    tray->set_on_settings([&store]() {
        GtkWidget* dlg = gtk_dialog_new_with_buttons(
            tr("settings_title"),
            nullptr,
            static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL),
            tr("btn_ok"), GTK_RESPONSE_OK,
            nullptr);
        gtk_window_set_default_size(GTK_WINDOW(dlg), 350, 150);

        GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
        gtk_container_set_border_width(GTK_CONTAINER(content), 16);
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(content), box);

        // Language selector
        GtkWidget* lang_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        GtkWidget* lang_label = gtk_label_new(tr("settings_language"));
        gtk_box_pack_start(GTK_BOX(lang_box), lang_label, FALSE, FALSE, 0);

        GtkWidget* combo = gtk_combo_box_text_new();
        auto langs = i18n_available_languages();
        int active_idx = 0;
        for (size_t i = 0; i < langs.size(); ++i) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                            i18n_language_name(langs[i]));
            if (langs[i] == i18n_get_language())
                active_idx = static_cast<int>(i);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active_idx);
        gtk_box_pack_start(GTK_BOX(lang_box), combo, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(box), lang_box, FALSE, FALSE, 0);

        // Restart note
        GtkWidget* note = gtk_label_new(tr("settings_restart_note"));
        gtk_widget_set_halign(note, GTK_ALIGN_START);
        PangoAttrList* attrs = pango_attr_list_new();
        pango_attr_list_insert(attrs, pango_attr_style_new(PANGO_STYLE_ITALIC));
        pango_attr_list_insert(attrs, pango_attr_scale_new(0.9));
        gtk_label_set_attributes(GTK_LABEL(note), attrs);
        pango_attr_list_unref(attrs);
        gtk_box_pack_start(GTK_BOX(box), note, FALSE, FALSE, 0);

        gtk_widget_show_all(dlg);

        if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
            int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
            if (idx >= 0 && idx < static_cast<int>(langs.size())) {
                i18n_set_language(langs[idx], store.config_dir());
            }
        }

        gtk_widget_destroy(dlg);
    });

    // ── Warnings (one-time) ──
    if (!is_paste_tool_available()) {
        GtkWidget* msg = gtk_message_dialog_new(
            nullptr,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "%s", tr("warn_no_paste_tool"));
        gtk_dialog_run(GTK_DIALOG(msg));
        gtk_widget_destroy(msg);
    }

    // ── GTK main loop ──
    gtk_main();

    return 0;
}
