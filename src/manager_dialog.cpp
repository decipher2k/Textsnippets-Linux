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

// manager_dialog.cpp – Manager dialog (TreeView + Editor)
#include "manager_dialog.h"
#include "i18n.h"

#include <iostream>

// ═══════════════════════════════════════════════════════════════
//  Konstruktor / Destruktor
// ═══════════════════════════════════════════════════════════════

ManagerDialog::ManagerDialog(SnippetStore& store,
                             std::function<void()> on_data_changed)
    : m_store(store), m_on_data_changed(std::move(on_data_changed))
{
    build_ui();
}

ManagerDialog::~ManagerDialog() {
    if (m_window)
        gtk_widget_destroy(m_window);
}

// ═══════════════════════════════════════════════════════════════
//  UI-Aufbau
// ═══════════════════════════════════════════════════════════════

void ManagerDialog::build_ui() {
    // ── Main window ──
    m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(m_window), tr("window_title"));
    gtk_window_set_default_size(GTK_WINDOW(m_window), 850, 520);
    g_signal_connect(m_window, "delete-event",
                     G_CALLBACK(on_window_delete), this);

    // ── Horizontaler Paned (links: Tree, rechts: Editor) ──
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_set_position(GTK_PANED(paned), 280);

    // ────────── Links: TreeView ──────────
    GtkWidget* scroll_tree = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_tree),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll_tree, 250, -1);

    m_tree_store = gtk_tree_store_new(NUM_COLS,
                                      G_TYPE_STRING,   // COL_ICON_NAME
                                      G_TYPE_STRING,   // COL_DISPLAY_NAME
                                      G_TYPE_BOOLEAN); // COL_IS_GROUP
    m_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(m_tree_store));
    g_object_unref(m_tree_store); // TreeView hält eigene Referenz

    // Column: Icon + Name
    GtkTreeViewColumn* col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, tr("col_snippets"));

    GtkCellRenderer* icon_r = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(col, icon_r, FALSE);
    gtk_tree_view_column_add_attribute(col, icon_r, "icon-name", COL_ICON_NAME);

    GtkCellRenderer* text_r = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, text_r, TRUE);
    gtk_tree_view_column_add_attribute(col, text_r, "text", COL_DISPLAY_NAME);

    gtk_tree_view_append_column(GTK_TREE_VIEW(m_tree_view), col);

    // Selektion
    GtkTreeSelection* sel =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(m_tree_view));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect(sel, "changed",
                     G_CALLBACK(on_selection_changed), this);

    gtk_container_add(GTK_CONTAINER(scroll_tree), m_tree_view);
    gtk_paned_pack1(GTK_PANED(paned), scroll_tree, FALSE, FALSE);

    // ────────── Rechts: Editor ──────────
    GtkWidget* right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(right_box), 10);

    // Caption
    m_caption_label = gtk_label_new(tr("label_caption"));
    gtk_widget_set_halign(m_caption_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), m_caption_label, FALSE, FALSE, 0);

    m_caption_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(right_box), m_caption_entry, FALSE, FALSE, 0);

    // Text
    m_text_label = gtk_label_new(tr("label_text"));
    gtk_widget_set_halign(m_text_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(right_box), m_text_label, FALSE, FALSE, 0);

    m_text_scroll = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_text_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    m_text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_text_view), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(m_text_scroll), m_text_view);
    gtk_box_pack_start(GTK_BOX(right_box), m_text_scroll, TRUE, TRUE, 0);

    // Speichern / Verwerfen
    GtkWidget* btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(btn_box, GTK_ALIGN_END);
    m_save_btn    = gtk_button_new_with_label(tr("btn_save"));
    m_discard_btn = gtk_button_new_with_label(tr("btn_discard"));
    g_signal_connect(m_save_btn,    "clicked", G_CALLBACK(on_save_clicked),    this);
    g_signal_connect(m_discard_btn, "clicked", G_CALLBACK(on_discard_clicked), this);
    gtk_box_pack_start(GTK_BOX(btn_box), m_save_btn,    FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), m_discard_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(right_box), btn_box, FALSE, FALSE, 0);

    gtk_paned_pack2(GTK_PANED(paned), right_box, TRUE, FALSE);

    // ────────── Gesamtlayout ──────────
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), paned, TRUE, TRUE, 0);

    // Separator
    gtk_box_pack_start(GTK_BOX(main_box),
                       gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),
                       FALSE, FALSE, 0);

    // Toolbar unten
    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(toolbar), 8);

    GtkWidget* add_group_btn = gtk_button_new_with_label(tr("btn_add_group"));
    m_add_snippet_btn        = gtk_button_new_with_label(tr("btn_add_snippet"));
    m_delete_btn             = gtk_button_new_with_label(tr("btn_delete"));
    m_rename_btn             = gtk_button_new_with_label(tr("btn_rename"));

    g_signal_connect(add_group_btn,    "clicked", G_CALLBACK(on_add_group_clicked),   this);
    g_signal_connect(m_add_snippet_btn,"clicked", G_CALLBACK(on_add_snippet_clicked), this);
    g_signal_connect(m_delete_btn,     "clicked", G_CALLBACK(on_delete_clicked),      this);
    g_signal_connect(m_rename_btn,     "clicked", G_CALLBACK(on_rename_clicked),      this);

    gtk_box_pack_start(GTK_BOX(toolbar), add_group_btn,     FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), m_add_snippet_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), m_delete_btn,      FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), m_rename_btn,      FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(m_window), main_box);

    clear_editor();
}

// ═══════════════════════════════════════════════════════════════
//  Zeigen / Verstecken
// ═══════════════════════════════════════════════════════════════

void ManagerDialog::show() {
    populate_tree();
    clear_editor();
    gtk_widget_show_all(m_window);
    gtk_window_present(GTK_WINDOW(m_window));
}

void ManagerDialog::hide() {
    gtk_widget_hide(m_window);
}

bool ManagerDialog::is_visible() const {
    return gtk_widget_get_visible(m_window);
}

// ═══════════════════════════════════════════════════════════════
//  TreeView befüllen
// ═══════════════════════════════════════════════════════════════

void ManagerDialog::populate_tree() {
    m_populating = true;
    gtk_tree_store_clear(m_tree_store);
    populate_tree_recurse(m_store.groups(), nullptr);
    gtk_tree_view_expand_all(GTK_TREE_VIEW(m_tree_view));
    m_populating = false;
}

void ManagerDialog::populate_tree_recurse(const std::vector<Group>& groups,
                                          GtkTreeIter* parent) {
    for (const auto& group : groups) {
        GtkTreeIter iter;
        gtk_tree_store_append(m_tree_store, &iter, parent);
        gtk_tree_store_set(m_tree_store, &iter,
                           COL_ICON_NAME,    "folder",
                           COL_DISPLAY_NAME, group.name.c_str(),
                           COL_IS_GROUP,     TRUE,
                           -1);

        // Untergruppen (rekursiv)
        populate_tree_recurse(group.children, &iter);

        // Textbausteine
        for (const auto& sn : group.snippets) {
            GtkTreeIter s_iter;
            gtk_tree_store_append(m_tree_store, &s_iter, &iter);
            gtk_tree_store_set(m_tree_store, &s_iter,
                               COL_ICON_NAME,    "text-x-generic",
                               COL_DISPLAY_NAME, sn.caption.c_str(),
                               COL_IS_GROUP,     FALSE,
                               -1);
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  Selektions-Hilfsfunktionen
// ═══════════════════════════════════════════════════════════════

std::vector<int> ManagerDialog::get_selected_path() {
    std::vector<int> result;
    GtkTreeSelection* sel =
        gtk_tree_view_get_selection(GTK_TREE_VIEW(m_tree_view));
    GtkTreeIter  iter;
    GtkTreeModel* model = nullptr;

    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
        int  depth   = gtk_tree_path_get_depth(path);
        int* indices = gtk_tree_path_get_indices(path);
        for (int i = 0; i < depth; ++i)
            result.push_back(indices[i]);
        gtk_tree_path_free(path);
    }
    return result;
}

std::optional<NodeInfo> ManagerDialog::get_selected_node() {
    auto p = get_selected_path();
    if (p.empty()) return std::nullopt;
    return m_store.resolve(p.data(), static_cast<int>(p.size()));
}

void ManagerDialog::restore_selection(const std::vector<int>& path_vec) {
    if (path_vec.empty()) return;

    GtkTreePath* path = gtk_tree_path_new();
    for (int idx : path_vec)
        gtk_tree_path_append_index(path, idx);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(m_tree_store), &iter, path)) {
        gtk_tree_view_expand_to_path(GTK_TREE_VIEW(m_tree_view), path);
        gtk_tree_selection_select_path(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(m_tree_view)), path);
    }
    gtk_tree_path_free(path);
}

// ═══════════════════════════════════════════════════════════════
//  Editor aktualisieren / leeren
// ═══════════════════════════════════════════════════════════════

void ManagerDialog::update_editor() {
    auto node = get_selected_node();
    if (!node) {
        clear_editor();
        return;
    }

    if (node->is_group) {
        // Gruppe ausgewählt → Name bearbeiten
        gtk_label_set_text(GTK_LABEL(m_caption_label), tr("label_group_name"));
        gtk_entry_set_text(GTK_ENTRY(m_caption_entry), node->group->name.c_str());

        gtk_widget_hide(m_text_label);
        gtk_widget_hide(m_text_scroll);

        gtk_widget_set_sensitive(m_save_btn,        TRUE);
        gtk_widget_set_sensitive(m_discard_btn,     TRUE);
        gtk_widget_set_sensitive(m_add_snippet_btn, TRUE);
        gtk_widget_set_sensitive(m_delete_btn,      TRUE);
        gtk_widget_set_sensitive(m_rename_btn,      TRUE);
    } else {
        // Snippet selected
        gtk_label_set_text(GTK_LABEL(m_caption_label), tr("label_caption"));
        gtk_entry_set_text(GTK_ENTRY(m_caption_entry),
                           node->snippet->caption.c_str());

        gtk_widget_show(m_text_label);
        gtk_widget_show(m_text_scroll);

        GtkTextBuffer* buf =
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text_view));
        gtk_text_buffer_set_text(buf, node->snippet->text.c_str(), -1);

        gtk_widget_set_sensitive(m_save_btn,        TRUE);
        gtk_widget_set_sensitive(m_discard_btn,     TRUE);
        gtk_widget_set_sensitive(m_add_snippet_btn, TRUE); // Sibling-Snippet
        gtk_widget_set_sensitive(m_delete_btn,      TRUE);
        gtk_widget_set_sensitive(m_rename_btn,      FALSE);
    }
}

void ManagerDialog::clear_editor() {
    gtk_label_set_text(GTK_LABEL(m_caption_label), tr("label_caption"));
    gtk_entry_set_text(GTK_ENTRY(m_caption_entry), "");

    GtkTextBuffer* buf =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_text_view));
    gtk_text_buffer_set_text(buf, "", -1);

    gtk_widget_show(m_text_label);
    gtk_widget_show(m_text_scroll);

    gtk_widget_set_sensitive(m_save_btn,        FALSE);
    gtk_widget_set_sensitive(m_discard_btn,     FALSE);
    gtk_widget_set_sensitive(m_add_snippet_btn, FALSE);
    gtk_widget_set_sensitive(m_delete_btn,      FALSE);
    gtk_widget_set_sensitive(m_rename_btn,      FALSE);
}

// Gemeinsamer Helfer: Speichern + Tree neu aufbauen + Tray-Menü aktualisieren
void ManagerDialog::data_changed() {
    m_store.save();
    populate_tree();
    if (m_on_data_changed)
        m_on_data_changed();
}

// ═══════════════════════════════════════════════════════════════
//  GTK-Callbacks
// ═══════════════════════════════════════════════════════════════

void ManagerDialog::on_selection_changed(GtkTreeSelection* /*sel*/,
                                         gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    if (self->m_populating) return;
    self->update_editor();
}

// ─────────── Speichern ───────────

void ManagerDialog::on_save_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    auto  path = self->get_selected_path();
    auto  node = self->get_selected_node();
    if (!node) return;

    const char* caption = gtk_entry_get_text(GTK_ENTRY(self->m_caption_entry));

    if (node->is_group) {
        node->group->name = caption;
    } else {
        node->snippet->caption = caption;

        GtkTextBuffer* buf =
            gtk_text_view_get_buffer(GTK_TEXT_VIEW(self->m_text_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buf, &start, &end);
        gchar* text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
        node->snippet->text = text;
        g_free(text);
    }

    self->data_changed();
    self->restore_selection(path);
}

// ─────────── Verwerfen ───────────

void ManagerDialog::on_discard_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    self->update_editor(); // Setzt Felder auf gespeicherten Stand zurück
}

// ─────────── + Gruppe ───────────

void ManagerDialog::on_add_group_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    auto  path = self->get_selected_path();

    if (path.empty()) {
        // Kein Element ausgewählt → Root-Gruppe
        self->m_store.add_root_group(tr("new_group"));
    } else {
        auto node = self->m_store.resolve(path.data(),
                                          static_cast<int>(path.size()));
        if (node && node->is_group) {
            self->m_store.add_group_at(path.data(),
                                       static_cast<int>(path.size()),
                                       tr("new_group"));
        } else {
            if (path.size() > 1) {
                path.pop_back();
                self->m_store.add_group_at(path.data(),
                                           static_cast<int>(path.size()),
                                           tr("new_group"));
            } else {
                self->m_store.add_root_group(tr("new_group"));
            }
        }
    }

    self->data_changed();
}

// ─────────── + Baustein ───────────

void ManagerDialog::on_add_snippet_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    auto  path = self->get_selected_path();
    if (path.empty()) return;

    auto node = self->m_store.resolve(path.data(),
                                      static_cast<int>(path.size()));
    if (!node) return;

    if (node->is_group) {
        // Snippet in selected group
        self->m_store.add_snippet_at(path.data(),
                                     static_cast<int>(path.size()),
                                     tr("new_snippet"), "");
    } else {
        // Snippet selected → sibling snippet in parent group
        if (path.size() > 1) {
            auto parent_path = path;
            parent_path.pop_back();
            self->m_store.add_snippet_at(parent_path.data(),
                                         static_cast<int>(parent_path.size()),
                                         tr("new_snippet"), "");
        }
    }

    self->data_changed();
}

// ─────────── Löschen ───────────

void ManagerDialog::on_delete_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    auto  path = self->get_selected_path();
    if (path.empty()) return;

    auto node = self->m_store.resolve(path.data(),
                                      static_cast<int>(path.size()));
    if (!node) return;

    const char* name = node->is_group ? node->group->name.c_str()
                                      : node->snippet->caption.c_str();

    // Confirmation dialog
    GtkWidget* dlg = gtk_message_dialog_new(
        GTK_WINDOW(self->m_window),
        static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        tr("confirm_delete"), name);

    int resp = gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);

    if (resp == GTK_RESPONSE_YES) {
        self->m_store.delete_at(path.data(), static_cast<int>(path.size()));
        self->data_changed();
        self->clear_editor();
    }
}

// ─────────── Umbenennen ───────────

void ManagerDialog::on_rename_clicked(GtkButton* /*btn*/, gpointer data) {
    auto* self = static_cast<ManagerDialog*>(data);
    auto  path = self->get_selected_path();
    auto  node = self->get_selected_node();
    if (!node || !node->is_group) return;

    GtkWidget* dlg = gtk_dialog_new_with_buttons(
        tr("dlg_rename_group"),
        GTK_WINDOW(self->m_window),
        static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        tr("btn_cancel"), GTK_RESPONSE_CANCEL,
        tr("btn_ok"),     GTK_RESPONSE_OK,
        nullptr);
    gtk_dialog_set_default_response(GTK_DIALOG(dlg), GTK_RESPONSE_OK);

    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    gtk_container_set_border_width(GTK_CONTAINER(content), 12);

    GtkWidget* label = gtk_label_new(tr("dlg_new_group_name"));
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 4);

    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), node->group->name.c_str());
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
    gtk_box_pack_start(GTK_BOX(content), entry, FALSE, FALSE, 4);

    gtk_widget_show_all(dlg);

    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK) {
        // Knoten erneut auflösen (Sicherheit)
        auto n2 = self->m_store.resolve(path.data(),
                                        static_cast<int>(path.size()));
        if (n2 && n2->is_group) {
            n2->group->name = gtk_entry_get_text(GTK_ENTRY(entry));
            self->data_changed();
            self->restore_selection(path);
        }
    }

    gtk_widget_destroy(dlg);
}

// ─────────── Fenster schließen (nur verstecken) ───────────

gboolean ManagerDialog::on_window_delete(GtkWidget* widget,
                                         GdkEvent* /*event*/,
                                         gpointer /*data*/) {
    gtk_widget_hide(widget);
    return TRUE; // Zerstörung verhindern
}
