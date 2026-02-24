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

// clipboard.h – Clipboard logic and paste mechanism
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <string>

/// Call once at startup (detects session type, checks available tools).
void clipboard_init();

/// Insert a snippet: copy to clipboard, then simulate Ctrl+V if possible.
/// Returns true if the paste action was triggered.
bool insert_snippet(const std::string& text);

/// Check if a paste simulation tool is available (xdotool, wtype, or ydotool).
bool is_paste_tool_available();

/// Check if a Wayland session is active.
bool is_wayland_session();

#endif // CLIPBOARD_H
