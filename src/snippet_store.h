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

// snippet_store.h – Datenmodell und JSON-Persistenz
#ifndef SNIPPET_STORE_H
#define SNIPPET_STORE_H

#include <optional>
#include <string>
#include <vector>

// ───────────── Datenstrukturen ─────────────

/// Ein einzelner Textbaustein.
struct Snippet {
    std::string caption; ///< Kurzbezeichnung (im Menü sichtbar)
    std::string text;    ///< Der eigentliche Textinhalt
};

/// Eine Gruppe, die Untergruppen und Textbausteine enthalten kann.
struct Group {
    std::string name;
    std::vector<Group>   children; ///< Verschachtelte Untergruppen
    std::vector<Snippet> snippets; ///< Textbausteine in dieser Gruppe
};

// ───────────── Navigation ─────────────

/// Ergebnis der Pfadauflösung im Datenmodell.
struct NodeInfo {
    bool     is_group;
    Group*   group   = nullptr; ///< != nullptr wenn is_group
    Snippet* snippet = nullptr; ///< != nullptr wenn !is_group
    Group*   parent  = nullptr; ///< Eltern-Gruppe (nullptr bei Root-Gruppen)
};

// ───────────── Store ─────────────

/**
 * Verwaltet die Textbausteine im Speicher und auf der Festplatte
 * (~/.config/textbaustein-manager/snippets.json).
 *
 * Der Baum wird im TreeView mit folgender Konvention abgebildet:
 *   children[0 .. n-1]  →  TreeView-Index  0 .. n-1   (Gruppen)
 *   snippets[0 .. m-1]  →  TreeView-Index  n .. n+m-1 (Bausteine)
 *
 * resolve() wandelt einen solchen TreeView-Pfad (Array von int) in
 * ein NodeInfo um, mit dem direkt auf die Daten zugegriffen werden kann.
 */
class SnippetStore {
public:
    SnippetStore();

    /// JSON laden.  Erstellt Beispieldaten falls Datei nicht existiert.
    bool load();
    /// JSON schreiben.
    bool save();

    std::vector<Group>&       groups()       { return m_groups; }
    const std::vector<Group>& groups() const { return m_groups; }

    // ── Pfad-Navigation ──

    /// Löst einen TreeView-Pfad (indices[0..depth-1]) in ein NodeInfo auf.
    std::optional<NodeInfo> resolve(const int* indices, int depth);

    // ── Mutation ──

    void add_root_group(const std::string& name);
    void add_group_at  (const int* parent_indices, int parent_depth,
                        const std::string& name);
    void add_snippet_at(const int* group_indices, int group_depth,
                        const std::string& caption, const std::string& text);
    void delete_at     (const int* indices, int depth);

    // ── Pfade ──

    std::string config_dir() const { return m_config_dir; }
    std::string filepath()   const { return m_filepath;   }

private:
    std::vector<Group> m_groups;
    std::string m_config_dir;
    std::string m_filepath;

    void ensure_config_dir();
    void init_defaults();
};

#endif // SNIPPET_STORE_H
