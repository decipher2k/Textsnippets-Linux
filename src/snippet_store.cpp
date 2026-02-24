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

// snippet_store.cpp – Data model, JSON load / save
#include "snippet_store.h"
#include "i18n.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using json = nlohmann::json;

// ═══════════════════ JSON-Serialisierung ═══════════════════

static json snippet_to_json(const Snippet& s) {
    return {{"caption", s.caption}, {"text", s.text}};
}

static json group_to_json(const Group& g) {
    json j;
    j["name"]     = g.name;
    j["children"] = json::array();
    for (const auto& child : g.children)
        j["children"].push_back(group_to_json(child));
    j["snippets"] = json::array();
    for (const auto& s : g.snippets)
        j["snippets"].push_back(snippet_to_json(s));
    return j;
}

static Snippet snippet_from_json(const json& j) {
    return {j.value("caption", ""), j.value("text", "")};
}

static Group group_from_json(const json& j) {
    Group g;
    g.name = j.value("name", std::string(tr("unnamed")));
    if (j.contains("children"))
        for (const auto& c : j["children"])
            g.children.push_back(group_from_json(c));
    if (j.contains("snippets"))
        for (const auto& s : j["snippets"])
            g.snippets.push_back(snippet_from_json(s));
    return g;
}

// ═══════════════════ SnippetStore ═══════════════════

SnippetStore::SnippetStore() {
    const char* home = std::getenv("HOME");
    if (!home) home = "/tmp";
    m_config_dir = std::string(home) + "/.config/textsnippets";
    m_filepath   = m_config_dir + "/snippets.json";
}

void SnippetStore::ensure_config_dir() {
    // Erstelle .config und Unterverzeichnis (ignoriert Fehler wenn existent)
    std::string base = std::string(std::getenv("HOME") ? std::getenv("HOME") : "/tmp")
                       + "/.config";
    mkdir(base.c_str(), 0755);
    mkdir(m_config_dir.c_str(), 0755);
}

// ─────────── Laden ───────────

bool SnippetStore::load() {
    std::ifstream file(m_filepath);
    if (!file.is_open()) {
        init_defaults();
        save();
        return true;
    }

    try {
        json j = json::parse(file);
        m_groups.clear();
        if (j.contains("groups"))
            for (const auto& g : j["groups"])
                m_groups.push_back(group_from_json(g));
        return true;
    } catch (const json::exception& e) {
        std::cerr << "JSON-Parsefehler: " << e.what() << std::endl;
        init_defaults();
        return false;
    }
}

// ─────────── Speichern ───────────

bool SnippetStore::save() {
    ensure_config_dir();

    json j;
    j["groups"] = json::array();
    for (const auto& g : m_groups)
        j["groups"].push_back(group_to_json(g));

    std::ofstream file(m_filepath);
    if (!file.is_open()) {
        std::cerr << "Kann nicht schreiben: " << m_filepath << std::endl;
        return false;
    }
    file << j.dump(2);
    return true;
}

// ─────────── Standarddaten ───────────

void SnippetStore::init_defaults() {
    m_groups.clear();

    Group greetings;
    greetings.name = tr("default_greetings");
    greetings.snippets.push_back(
        {tr("default_greeting"), tr("default_greeting_text")});
    greetings.snippets.push_back(
        {tr("default_farewell"), tr("default_farewell_text")});

    Group signatures;
    signatures.name = tr("default_signatures");
    signatures.snippets.push_back(
        {tr("default_private"), tr("default_private_text")});
    signatures.snippets.push_back(
        {tr("default_business"), tr("default_business_text")});

    Group email;
    email.name = tr("default_email");
    email.snippets.push_back(
        {tr("default_inquiry"), tr("default_inquiry_text")});
    email.snippets.push_back(
        {tr("default_offer"), tr("default_offer_text")});

    Group templates;
    templates.name = tr("default_templates");
    templates.children.push_back(email);

    m_groups.push_back(greetings);
    m_groups.push_back(signatures);
    m_groups.push_back(templates);
}

// ═══════════════════ Pfad-Auflösung ═══════════════════

std::optional<NodeInfo> SnippetStore::resolve(const int* indices, int depth) {
    if (depth <= 0 || indices[0] < 0 ||
        indices[0] >= static_cast<int>(m_groups.size()))
        return std::nullopt;

    Group*  current = &m_groups[indices[0]];
    Group*  parent  = nullptr;

    for (int i = 1; i < depth; ++i) {
        int idx        = indices[i];
        int n_children = static_cast<int>(current->children.size());

        if (idx < n_children) {
            // Untergruppe
            parent  = current;
            current = &current->children[idx];
        } else {
            // Textbaustein
            int s_idx = idx - n_children;
            if (s_idx < 0 ||
                s_idx >= static_cast<int>(current->snippets.size()))
                return std::nullopt;
            if (i != depth - 1)
                return std::nullopt; // Snippets haben keine Kinder
            return NodeInfo{false, nullptr, &current->snippets[s_idx], current};
        }
    }

    return NodeInfo{true, current, nullptr, parent};
}

// ═══════════════════ Mutation ═══════════════════

void SnippetStore::add_root_group(const std::string& name) {
    m_groups.push_back(Group{name, {}, {}});
}

void SnippetStore::add_group_at(const int* parent_indices, int parent_depth,
                                const std::string& name) {
    auto info = resolve(parent_indices, parent_depth);
    if (!info || !info->is_group) return;
    info->group->children.push_back(Group{name, {}, {}});
}

void SnippetStore::add_snippet_at(const int* group_indices, int group_depth,
                                  const std::string& caption,
                                  const std::string& text) {
    auto info = resolve(group_indices, group_depth);
    if (!info || !info->is_group) return;
    info->group->snippets.push_back(Snippet{caption, text});
}

void SnippetStore::delete_at(const int* indices, int depth) {
    if (depth <= 0) return;

    // Root-Gruppe
    if (depth == 1) {
        int idx = indices[0];
        if (idx >= 0 && idx < static_cast<int>(m_groups.size()))
            m_groups.erase(m_groups.begin() + idx);
        return;
    }

    // Eltern-Knoten auflösen (ein Level weniger)
    auto parent_info = resolve(indices, depth - 1);
    if (!parent_info || !parent_info->is_group) return;

    Group*     parent     = parent_info->group;
    int        last_idx   = indices[depth - 1];
    int        n_children = static_cast<int>(parent->children.size());

    if (last_idx < n_children) {
        parent->children.erase(parent->children.begin() + last_idx);
    } else {
        int s_idx = last_idx - n_children;
        if (s_idx >= 0 && s_idx < static_cast<int>(parent->snippets.size()))
            parent->snippets.erase(parent->snippets.begin() + s_idx);
    }
}
