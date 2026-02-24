cmake_minimum_required(VERSION 3.14)
project(textsnippets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- GTK 3 ----------
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# ---------- AppIndicator (ayatana bevorzugt, Fallback: legacy) ----------
pkg_check_modules(AYATANA ayatana-appindicator3-0.1)
if(AYATANA_FOUND)
    set(INDICATOR_INCLUDE_DIRS  ${AYATANA_INCLUDE_DIRS})
    set(INDICATOR_LIBRARY_DIRS  ${AYATANA_LIBRARY_DIRS})
    set(INDICATOR_LIBRARIES     ${AYATANA_LIBRARIES})
    set(INDICATOR_CFLAGS        ${AYATANA_CFLAGS_OTHER})
    set(USE_AYATANA TRUE)
else()
    pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)
    set(INDICATOR_INCLUDE_DIRS  ${APPINDICATOR_INCLUDE_DIRS})
    set(INDICATOR_LIBRARY_DIRS  ${APPINDICATOR_LIBRARY_DIRS})
    set(INDICATOR_LIBRARIES     ${APPINDICATOR_LIBRARIES})
    set(INDICATOR_CFLAGS        ${APPINDICATOR_CFLAGS_OTHER})
    set(USE_AYATANA FALSE)
endif()

# ---------- nlohmann/json ----------
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json nicht gefunden – wird per FetchContent heruntergeladen …")
    include(FetchContent)
    FetchContent_Declare(json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# ---------- Executable ----------
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/tray.cpp
    src/manager_dialog.cpp
    src/snippet_store.cpp
    src/clipboard.cpp
    src/i18n.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GTK3_INCLUDE_DIRS}
    ${INDICATOR_INCLUDE_DIRS}
    src
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${GTK3_LIBRARY_DIRS}
    ${INDICATOR_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GTK3_LIBRARIES}
    ${INDICATOR_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${GTK3_CFLAGS_OTHER}
    ${INDICATOR_CFLAGS}
    -Wall -Wextra -Wpedantic
)

if(USE_AYATANA)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_AYATANA_APPINDICATOR)
endif()

# ---------- Install ----------
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES data/textsnippets.desktop
        DESTINATION share/applications)
install(FILES data/textsnippets-autostart.desktop
        DESTINATION /etc/xdg/autostart)

# ---------- CPack / DEB ----------
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "textsnippets")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Linux text snippet manager with system tray")
set(CPACK_PACKAGE_DESCRIPTION "A Linux text snippet manager running as a system tray application. Snippets can be organized hierarchically in groups and inserted into the active window via the tray menu. Supports English, German, Spanish, French, Italian, Hindi, Chinese, and Russian.")
set(CPACK_PACKAGE_CONTACT "decipher2k20")
set(CPACK_PACKAGE_MAINTAINER "decipher2k20")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "xdotool, xclip, wtype, wl-clipboard, ydotool")

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)
