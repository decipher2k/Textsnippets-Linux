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

// i18n.h – Internationalization support
#ifndef I18N_H
#define I18N_H

#include <string>
#include <vector>

enum class Language {
    English = 0,
    German,
    Spanish,
    French,
    Italian,
    Hindi,
    Chinese,
    Russian,
    COUNT
};

/// Initialize i18n – loads saved language from config_dir/settings.json
void i18n_init(const std::string& config_dir);

/// Get a translated string by key. Returns key if not found.
const char* tr(const char* key);

/// Set and persist the current language
void i18n_set_language(Language lang, const std::string& config_dir);

/// Get current language
Language i18n_get_language();

/// Get display name for a language (in its own language)
const char* i18n_language_name(Language lang);

/// Get language code (e.g. "en", "de")
const char* i18n_language_code(Language lang);

/// Get all available languages
std::vector<Language> i18n_available_languages();

/// Parse language from code string
Language i18n_language_from_code(const std::string& code);

#endif // I18N_H
