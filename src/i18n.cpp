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

// i18n.cpp – Internationalization implementation
#include "i18n.h"

#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sys/stat.h>

// ═══════════════════ Internal state ═══════════════════

static Language s_current = Language::English;
static std::mutex s_mutex;

// ═══════════════════ Translation tables ═══════════════════

using TransMap = std::map<std::string, std::string>;

// Forward declarations
static const TransMap& get_translations(Language lang);

// ── English (default) ──
static const TransMap s_en = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           "Manage Snippets\u2026"},
    {"tray_quit",             "Quit"},
    {"tray_empty",            "(empty)"},
    {"tray_settings",         "Settings\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          "Snippets"},
    {"label_caption",         "Caption:"},
    {"label_text",            "Text:"},
    {"label_group_name",      "Group Name:"},
    {"btn_save",              "Save"},
    {"btn_discard",           "Discard"},
    {"btn_add_group",         "+ Group"},
    {"btn_add_snippet",       "+ Snippet"},
    {"btn_delete",            "Delete"},
    {"btn_rename",            "Rename"},
    {"new_group",             "New Group"},
    {"new_snippet",           "New Snippet"},
    {"confirm_delete",        "Do you really want to delete \"%s\"?"},
    {"dlg_rename_group",      "Rename Group"},
    {"dlg_new_group_name",    "New group name:"},
    {"btn_cancel",            "Cancel"},
    {"btn_ok",                "OK"},
    {"warn_xdotool_title",    "xdotool not found"},
    {"warn_xdotool",          "xdotool was not found.\n\n"
                              "Snippets will only be copied to the clipboard.\n"
                              "For automatic pasting, install xdotool:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          "Wayland session detected.\n\n"
                              "Automatic pasting via xdotool may not work\n"
                              "reliably under Wayland.\n"
                              "Text will be copied to the clipboard instead."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       "Text has been copied to the clipboard."},
    {"unnamed",               "Unnamed"},
    {"settings_title",        "Settings"},
    {"settings_language",     "Language:"},
    {"settings_restart_note", "Language change takes effect after restart."},
    {"default_greetings",     "Greetings"},
    {"default_greeting",      "Greeting"},
    {"default_greeting_text", "Dear Sir or Madam,\n\nThank you for your message."},
    {"default_farewell",      "Farewell"},
    {"default_farewell_text", "Kind regards\nJohn Doe"},
    {"default_signatures",    "Signatures"},
    {"default_private",       "Private"},
    {"default_private_text",  "John Doe\n123 Main Street\n12345 Anytown"},
    {"default_business",      "Business"},
    {"default_business_text", "John Doe\nAcme Corp.\nTel: +1 234 567890"},
    {"default_email",         "E-Mail"},
    {"default_inquiry",       "Inquiry"},
    {"default_inquiry_text",  "Dear Sir or Madam,\n\nI would like to inquire..."},
    {"default_offer",         "Offer"},
    {"default_offer_text",    "Dear Sir or Madam,\n\n"
                              "We are pleased to submit the following offer..."},
    {"default_templates",     "Templates"},
};

// ── German ──
static const TransMap s_de = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           "Textbausteine verwalten\u2026"},
    {"tray_quit",             "Beenden"},
    {"tray_empty",            "(leer)"},
    {"tray_settings",         "Einstellungen\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          "Textbausteine"},
    {"label_caption",         "Bezeichnung:"},
    {"label_text",            "Text:"},
    {"label_group_name",      "Gruppenname:"},
    {"btn_save",              "Speichern"},
    {"btn_discard",           "Verwerfen"},
    {"btn_add_group",         "+ Gruppe"},
    {"btn_add_snippet",       "+ Baustein"},
    {"btn_delete",            u8"Löschen"},
    {"btn_rename",            "Umbenennen"},
    {"new_group",             "Neue Gruppe"},
    {"new_snippet",           "Neuer Baustein"},
    {"confirm_delete",        u8"Möchten Sie \"%s\" wirklich löschen?"},
    {"dlg_rename_group",      "Gruppe umbenennen"},
    {"dlg_new_group_name",    "Neuer Gruppenname:"},
    {"btn_cancel",            "Abbrechen"},
    {"btn_ok",                "OK"},
    {"warn_xdotool_title",    "xdotool nicht gefunden"},
    {"warn_xdotool",          "xdotool wurde nicht gefunden.\n\n"
                              "Textbausteine werden nur in die Zwischenablage kopiert.\n"
                              u8"Für automatisches Einfügen installieren Sie xdotool:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          "Wayland-Sitzung erkannt.\n\n"
                              u8"Das automatische Einfügen via xdotool funktioniert unter\n"
                              u8"Wayland möglicherweise nicht zuverlässig.\n"
                              "Der Text wird stattdessen in die Zwischenablage kopiert."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       "Text wurde in die Zwischenablage kopiert."},
    {"unnamed",               "Unbenannt"},
    {"settings_title",        "Einstellungen"},
    {"settings_language",     "Sprache:"},
    {"settings_restart_note", u8"Die Sprachänderung wird nach einem Neustart wirksam."},
    {"default_greetings",     u8"Grüße"},
    {"default_greeting",      u8"Begrüßung"},
    {"default_greeting_text", u8"Sehr geehrte Damen und Herren,\n\nvielen Dank für Ihre Nachricht."},
    {"default_farewell",      "Verabschiedung"},
    {"default_farewell_text", u8"Mit freundlichen Grüßen\nMax Mustermann"},
    {"default_signatures",    "Signaturen"},
    {"default_private",       "Privat"},
    {"default_private_text",  u8"Max Mustermann\nMusterstraße 1\n12345 Musterstadt"},
    {"default_business",      u8"Geschäftlich"},
    {"default_business_text", u8"Max Mustermann\nMuster GmbH\nTel: +49 123 456789"},
    {"default_email",         "E-Mail"},
    {"default_inquiry",       "Anfrage"},
    {"default_inquiry_text",  u8"Sehr geehrte Damen und Herren,\n\nhiermit möchte ich anfragen..."},
    {"default_offer",         "Angebot"},
    {"default_offer_text",    "Sehr geehrte Damen und Herren,\n\n"
                              "gerne unterbreiten wir Ihnen folgendes Angebot..."},
    {"default_templates",     "Vorlagen"},
};

// ── Spanish ──
static const TransMap s_es = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           "Administrar fragmentos\u2026"},
    {"tray_quit",             "Salir"},
    {"tray_empty",            u8"(vacío)"},
    {"tray_settings",         "Configuraci\u00f3n\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          "Fragmentos"},
    {"label_caption",         u8"Título:"},
    {"label_text",            "Texto:"},
    {"label_group_name",      "Nombre del grupo:"},
    {"btn_save",              "Guardar"},
    {"btn_discard",           "Descartar"},
    {"btn_add_group",         "+ Grupo"},
    {"btn_add_snippet",       "+ Fragmento"},
    {"btn_delete",            "Eliminar"},
    {"btn_rename",            "Renombrar"},
    {"new_group",             "Nuevo grupo"},
    {"new_snippet",           "Nuevo fragmento"},
    {"confirm_delete",        u8"¿Desea eliminar \"%s\"?"},
    {"dlg_rename_group",      "Renombrar grupo"},
    {"dlg_new_group_name",    "Nuevo nombre del grupo:"},
    {"btn_cancel",            "Cancelar"},
    {"btn_ok",                "Aceptar"},
    {"warn_xdotool_title",    "xdotool no encontrado"},
    {"warn_xdotool",          "xdotool no fue encontrado.\n\n"
                              "Los fragmentos solo se copiar\u00e1n al portapapeles.\n"
                              "Para el pegado autom\u00e1tico, instale xdotool:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          u8"Sesión Wayland detectada.\n\n"
                              "El pegado autom\u00e1tico con xdotool puede no funcionar\n"
                              u8"de forma fiable en Wayland.\n"
                              "El texto se copiar\u00e1 al portapapeles."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       "El texto se ha copiado al portapapeles."},
    {"unnamed",               "Sin nombre"},
    {"settings_title",        u8"Configuración"},
    {"settings_language",     "Idioma:"},
    {"settings_restart_note", u8"El cambio de idioma se aplicará tras reiniciar."},
    {"default_greetings",     "Saludos"},
    {"default_greeting",      "Saludo"},
    {"default_greeting_text", "Estimado/a,\n\nGracias por su mensaje."},
    {"default_farewell",      "Despedida"},
    {"default_farewell_text", "Atentamente\nJuan P\u00e9rez"},
    {"default_signatures",    "Firmas"},
    {"default_private",       "Privado"},
    {"default_private_text",  "Juan P\u00e9rez\nCalle Principal 1\n12345 Ciudad"},
    {"default_business",      "Empresarial"},
    {"default_business_text", "Juan P\u00e9rez\nEmpresa S.L.\nTel: +34 123 456789"},
    {"default_email",         "Correo"},
    {"default_inquiry",       "Consulta"},
    {"default_inquiry_text",  "Estimado/a,\n\nDeseo consultar..."},
    {"default_offer",         "Oferta"},
    {"default_offer_text",    "Estimado/a,\n\n"
                              "Nos complace presentarle la siguiente oferta..."},
    {"default_templates",     "Plantillas"},
};

// ── French ──
static const TransMap s_fr = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           "G\u00e9rer les extraits\u2026"},
    {"tray_quit",             "Quitter"},
    {"tray_empty",            "(vide)"},
    {"tray_settings",         "Param\u00e8tres\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          "Extraits"},
    {"label_caption",         u8"Libellé :"},
    {"label_text",            "Texte :"},
    {"label_group_name",      "Nom du groupe :"},
    {"btn_save",              "Enregistrer"},
    {"btn_discard",           "Annuler"},
    {"btn_add_group",         "+ Groupe"},
    {"btn_add_snippet",       "+ Extrait"},
    {"btn_delete",            "Supprimer"},
    {"btn_rename",            "Renommer"},
    {"new_group",             "Nouveau groupe"},
    {"new_snippet",           "Nouvel extrait"},
    {"confirm_delete",        u8"Voulez-vous vraiment supprimer « %s » ?"},
    {"dlg_rename_group",      "Renommer le groupe"},
    {"dlg_new_group_name",    "Nouveau nom du groupe :"},
    {"btn_cancel",            "Annuler"},
    {"btn_ok",                "OK"},
    {"warn_xdotool_title",    "xdotool introuvable"},
    {"warn_xdotool",          "xdotool n'a pas \u00e9t\u00e9 trouv\u00e9.\n\n"
                              "Les extraits seront uniquement copi\u00e9s dans le press-papiers.\n"
                              "Pour le collage automatique, installez xdotool :\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          "Session Wayland d\u00e9tect\u00e9e.\n\n"
                              "Le collage automatique via xdotool peut ne pas fonctionner\n"
                              "de mani\u00e8re fiable sous Wayland.\n"
                              "Le texte sera copi\u00e9 dans le presse-papiers."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       u8"Le texte a été copié dans le presse-papiers."},
    {"unnamed",               u8"Sans nom"},
    {"settings_title",        u8"Paramètres"},
    {"settings_language",     "Langue :"},
    {"settings_restart_note", u8"Le changement de langue prendra effet après le redémarrage."},
    {"default_greetings",     "Salutations"},
    {"default_greeting",      "Salutation"},
    {"default_greeting_text", "Madame, Monsieur,\n\nMerci pour votre message."},
    {"default_farewell",      u8"Formule de politesse"},
    {"default_farewell_text", "Cordialement\nJean Dupont"},
    {"default_signatures",    "Signatures"},
    {"default_private",       u8"Privé"},
    {"default_private_text",  "Jean Dupont\n1 Rue Principale\n75000 Paris"},
    {"default_business",      "Professionnel"},
    {"default_business_text", "Jean Dupont\nEntreprise SARL\nT\u00e9l : +33 1 23 45 67 89"},
    {"default_email",         "Courriel"},
    {"default_inquiry",       "Demande"},
    {"default_inquiry_text",  "Madame, Monsieur,\n\nJe souhaite me renseigner..."},
    {"default_offer",         "Offre"},
    {"default_offer_text",    "Madame, Monsieur,\n\n"
                              "Nous avons le plaisir de vous soumettre l'offre suivante..."},
    {"default_templates",     u8"Modèles"},
};

// ── Italian ──
static const TransMap s_it = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           "Gestisci frammenti\u2026"},
    {"tray_quit",             "Esci"},
    {"tray_empty",            "(vuoto)"},
    {"tray_settings",         "Impostazioni\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          "Frammenti"},
    {"label_caption",         "Didascalia:"},
    {"label_text",            "Testo:"},
    {"label_group_name",      "Nome del gruppo:"},
    {"btn_save",              "Salva"},
    {"btn_discard",           "Annulla"},
    {"btn_add_group",         "+ Gruppo"},
    {"btn_add_snippet",       "+ Frammento"},
    {"btn_delete",            "Elimina"},
    {"btn_rename",            "Rinomina"},
    {"new_group",             "Nuovo gruppo"},
    {"new_snippet",           "Nuovo frammento"},
    {"confirm_delete",        "Vuoi davvero eliminare \"%s\"?"},
    {"dlg_rename_group",      "Rinomina gruppo"},
    {"dlg_new_group_name",    "Nuovo nome del gruppo:"},
    {"btn_cancel",            "Annulla"},
    {"btn_ok",                "OK"},
    {"warn_xdotool_title",    "xdotool non trovato"},
    {"warn_xdotool",          "xdotool non \u00e8 stato trovato.\n\n"
                              "I frammenti verranno solo copiati negli appunti.\n"
                              "Per incollare automaticamente, installa xdotool:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          "Sessione Wayland rilevata.\n\n"
                              "L'incollaggio automatico tramite xdotool potrebbe\n"
                              "non funzionare in modo affidabile sotto Wayland.\n"
                              "Il testo verr\u00e0 copiato negli appunti."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       u8"Il testo è stato copiato negli appunti."},
    {"unnamed",               "Senza nome"},
    {"settings_title",        "Impostazioni"},
    {"settings_language",     "Lingua:"},
    {"settings_restart_note", u8"La modifica della lingua avrà effetto dopo il riavvio."},
    {"default_greetings",     "Saluti"},
    {"default_greeting",      "Saluto"},
    {"default_greeting_text", "Gentile Signore/a,\n\nGrazie per il Suo messaggio."},
    {"default_farewell",      "Commiato"},
    {"default_farewell_text", "Cordiali saluti\nMario Rossi"},
    {"default_signatures",    "Firme"},
    {"default_private",       "Privato"},
    {"default_private_text",  "Mario Rossi\nVia Principale 1\n00100 Roma"},
    {"default_business",      "Aziendale"},
    {"default_business_text", "Mario Rossi\nAzienda S.r.l.\nTel: +39 012 3456789"},
    {"default_email",         "E-Mail"},
    {"default_inquiry",       "Richiesta"},
    {"default_inquiry_text",  "Gentile Signore/a,\n\nDesidero chiedere informazioni..."},
    {"default_offer",         "Offerta"},
    {"default_offer_text",    "Gentile Signore/a,\n\n"
                              u8"Siamo lieti di presentarLe la seguente offerta..."},
    {"default_templates",     "Modelli"},
};

// ── Hindi ──
static const TransMap s_hi = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           u8"स्निपेट प्रबंधित करें\u2026"},
    {"tray_quit",             u8"बाहर निकलें"},
    {"tray_empty",            u8"(खाली)"},
    {"tray_settings",         u8"सेटिंग्स\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          u8"स्निपेट"},
    {"label_caption",         u8"शीर्षक:"},
    {"label_text",            u8"पाठ:"},
    {"label_group_name",      u8"समूह का नाम:"},
    {"btn_save",              u8"सहेजें"},
    {"btn_discard",           u8"रद्द करें"},
    {"btn_add_group",         u8"+ समूह"},
    {"btn_add_snippet",       u8"+ स्निपेट"},
    {"btn_delete",            u8"हटाएं"},
    {"btn_rename",            u8"नाम बदलें"},
    {"new_group",             u8"नया समूह"},
    {"new_snippet",           u8"नया स्निपेट"},
    {"confirm_delete",        u8"क्या आप वाकई \"%s\" को हटाना चाहते हैं?"},
    {"dlg_rename_group",      u8"समूह का नाम बदलें"},
    {"dlg_new_group_name",    u8"नया समूह नाम:"},
    {"btn_cancel",            u8"रद्द करें"},
    {"btn_ok",                u8"ठीक है"},
    {"warn_xdotool_title",    u8"xdotool नहीं मिला"},
    {"warn_xdotool",          u8"xdotool नहीं मिला।\n\n"
                              u8"स्निपेट केवल क्लिपबोर्ड पर कॉपी किए जाएंगे।\n"
                              u8"स्वचालित पेस्टिंग के लिए xdotool इंस्टॉल करें:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          u8"Wayland सत्र का पता चला।\n\n"
                              u8"xdotool के माध्यम से स्वचालित पेस्टिंग Wayland\n"
                              u8"के तहत विश्वसनीय रूप से काम नहीं कर सकती।\n"
                              u8"पाठ इसके बजाय क्लिपबोर्ड पर कॉपी किया जाएगा।"},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       u8"पाठ क्लिपबोर्ड पर कॉपी किया गया है।"},
    {"unnamed",               u8"अनाम"},
    {"settings_title",        u8"सेटिंग्स"},
    {"settings_language",     u8"भाषा:"},
    {"settings_restart_note", u8"भाषा परिवर्तन पुनरारंभ के बाद प्रभावी होगा।"},
    {"default_greetings",     u8"अभिवादन"},
    {"default_greeting",      u8"नमस्कार"},
    {"default_greeting_text", u8"प्रिय महोदय/महोदया,\n\nआपके संदेश के लिए धन्यवाद।"},
    {"default_farewell",      u8"विदाई"},
    {"default_farewell_text", u8"सादर\nराजेश कुमार"},
    {"default_signatures",    u8"हस्ताक्षर"},
    {"default_private",       u8"निजी"},
    {"default_private_text",  u8"राजेश कुमार\nमुख्य सड़क 1\n110001 नई दिल्ली"},
    {"default_business",      u8"व्यावसायिक"},
    {"default_business_text", u8"राजेश कुमार\nकंपनी प्रा. लि.\nदूरभाष: +91 11 23456789"},
    {"default_email",         u8"ई-मेल"},
    {"default_inquiry",       u8"पूछताछ"},
    {"default_inquiry_text",  u8"प्रिय महोदय/महोदया,\n\nमैं पूछताछ करना चाहता/चाहती हूं..."},
    {"default_offer",         u8"प्रस्ताव"},
    {"default_offer_text",    u8"प्रिय महोदय/महोदया,\n\n"
                              u8"हमें आपको निम्नलिखित प्रस्ताव प्रस्तुत करते हुए प्रसन्नता हो रही है..."},
    {"default_templates",     u8"टेम्पलेट"},
};

// ── Chinese (Simplified) ──
static const TransMap s_zh = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           u8"管理文本片段\u2026"},
    {"tray_quit",             u8"退出"},
    {"tray_empty",            u8"(空)"},
    {"tray_settings",         u8"设置\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          u8"文本片段"},
    {"label_caption",         u8"标题："},
    {"label_text",            u8"文本："},
    {"label_group_name",      u8"组名称："},
    {"btn_save",              u8"保存"},
    {"btn_discard",           u8"放弃"},
    {"btn_add_group",         u8"+ 组"},
    {"btn_add_snippet",       u8"+ 片段"},
    {"btn_delete",            u8"删除"},
    {"btn_rename",            u8"重命名"},
    {"new_group",             u8"新建组"},
    {"new_snippet",           u8"新建片段"},
    {"confirm_delete",        u8"您确定要删除「%s」吗？"},
    {"dlg_rename_group",      u8"重命名组"},
    {"dlg_new_group_name",    u8"新组名称："},
    {"btn_cancel",            u8"取消"},
    {"btn_ok",                u8"确定"},
    {"warn_xdotool_title",    u8"未找到 xdotool"},
    {"warn_xdotool",          u8"未找到 xdotool。\n\n"
                              u8"文本片段将仅复制到剪贴板。\n"
                              u8"如需自动粘贴，请安装 xdotool：\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          u8"检测到 Wayland 会话。\n\n"
                              u8"通过 xdotool 自动粘贴在 Wayland 下\n"
                              u8"可能不太可靠。\n"
                              u8"文本将被复制到剪贴板。"},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       u8"文本已复制到剪贴板。"},
    {"unnamed",               u8"未命名"},
    {"settings_title",        u8"设置"},
    {"settings_language",     u8"语言："},
    {"settings_restart_note", u8"语言更改将在重启后生效。"},
    {"default_greetings",     u8"问候"},
    {"default_greeting",      u8"问候语"},
    {"default_greeting_text", u8"尊敬的先生/女士：\n\n感谢您的来信。"},
    {"default_farewell",      u8"告别"},
    {"default_farewell_text", u8"此致敬礼\n张三"},
    {"default_signatures",    u8"签名"},
    {"default_private",       u8"私人"},
    {"default_private_text",  u8"张三\n主大街1号\n100000 北京"},
    {"default_business",      u8"商务"},
    {"default_business_text", u8"张三\n示例有限公司\n电话：+86 10 12345678"},
    {"default_email",         u8"电子邮件"},
    {"default_inquiry",       u8"咨询"},
    {"default_inquiry_text",  u8"尊敬的先生/女士：\n\n我想咨询…"},
    {"default_offer",         u8"报价"},
    {"default_offer_text",    u8"尊敬的先生/女士：\n\n"
                              u8"我们很高兴向您提交以下报价…"},
    {"default_templates",     u8"模板"},
};

// ── Russian ──
static const TransMap s_ru = {
    {"app_title",             "Textsnippets"},
    {"tray_manage",           u8"Управление фрагментами\u2026"},
    {"tray_quit",             u8"Выход"},
    {"tray_empty",            u8"(пусто)"},
    {"tray_settings",         u8"Настройки\u2026"},
    {"window_title",          "Textsnippets"},
    {"col_snippets",          u8"Фрагменты"},
    {"label_caption",         u8"Заголовок:"},
    {"label_text",            u8"Текст:"},
    {"label_group_name",      u8"Имя группы:"},
    {"btn_save",              u8"Сохранить"},
    {"btn_discard",           u8"Отменить"},
    {"btn_add_group",         u8"+ Группа"},
    {"btn_add_snippet",       u8"+ Фрагмент"},
    {"btn_delete",            u8"Удалить"},
    {"btn_rename",            u8"Переименовать"},
    {"new_group",             u8"Новая группа"},
    {"new_snippet",           u8"Новый фрагмент"},
    {"confirm_delete",        u8"Вы действительно хотите удалить «%s»?"},
    {"dlg_rename_group",      u8"Переименовать группу"},
    {"dlg_new_group_name",    u8"Новое имя группы:"},
    {"btn_cancel",            u8"Отмена"},
    {"btn_ok",                "OK"},
    {"warn_xdotool_title",    u8"xdotool не найден"},
    {"warn_xdotool",          u8"xdotool не найден.\n\n"
                              u8"Фрагменты будут только скопированы в буфер обмена.\n"
                              u8"Для автоматической вставки установите xdotool:\n\n"
                              "  sudo apt install xdotool      (Debian/Ubuntu)\n"
                              "  sudo dnf install xdotool      (Fedora)\n"
                              "  sudo pacman -S xdotool        (Arch)"},
    {"warn_wayland",          u8"Обнаружен сеанс Wayland.\n\n"
                              u8"Автоматическая вставка через xdotool может\n"
                              u8"работать ненадёжно в Wayland.\n"
                              u8"Текст будет скопирован в буфер обмена."},
    {"notif_title",           "Textsnippets"},
    {"notif_clipboard",       u8"Текст скопирован в буфер обмена."},
    {"unnamed",               u8"Без имени"},
    {"settings_title",        u8"Настройки"},
    {"settings_language",     u8"Язык:"},
    {"settings_restart_note", u8"Изменение языка вступит в силу после перезапуска."},
    {"default_greetings",     u8"Приветствия"},
    {"default_greeting",      u8"Приветствие"},
    {"default_greeting_text", u8"Уважаемые дамы и господа,\n\nБлагодарим вас за ваше сообщение."},
    {"default_farewell",      u8"Прощание"},
    {"default_farewell_text", u8"С уважением\nИван Иванов"},
    {"default_signatures",    u8"Подписи"},
    {"default_private",       u8"Личная"},
    {"default_private_text",  u8"Иван Иванов\nГлавная ул. 1\n101000 Москва"},
    {"default_business",      u8"Рабочая"},
    {"default_business_text", u8"Иван Иванов\nООО «Компания»\nТел: +7 495 1234567"},
    {"default_email",         u8"Эл. почта"},
    {"default_inquiry",       u8"Запрос"},
    {"default_inquiry_text",  u8"Уважаемые дамы и господа,\n\nЯ хотел бы узнать..."},
    {"default_offer",         u8"Предложение"},
    {"default_offer_text",    u8"Уважаемые дамы и господа,\n\n"
                              u8"Мы рады представить вам следующее предложение..."},
    {"default_templates",     u8"Шаблоны"},
};

// ═══════════════════ Translation lookup ═══════════════════

static const TransMap& get_translations(Language lang) {
    switch (lang) {
        case Language::German:  return s_de;
        case Language::Spanish: return s_es;
        case Language::French:  return s_fr;
        case Language::Italian: return s_it;
        case Language::Hindi:   return s_hi;
        case Language::Chinese: return s_zh;
        case Language::Russian: return s_ru;
        default:                return s_en;
    }
}

// ═══════════════════ Public API ═══════════════════

void i18n_init(const std::string& config_dir) {
    std::string settings_path = config_dir + "/settings.json";
    std::ifstream file(settings_path);
    if (file.is_open()) {
        std::string line;
        // Simple JSON parsing – look for "language": "xx"
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        auto pos = content.find("\"language\"");
        if (pos != std::string::npos) {
            auto colon = content.find(':', pos);
            if (colon != std::string::npos) {
                auto q1 = content.find('"', colon + 1);
                if (q1 != std::string::npos) {
                    auto q2 = content.find('"', q1 + 1);
                    if (q2 != std::string::npos) {
                        std::string code = content.substr(q1 + 1, q2 - q1 - 1);
                        s_current = i18n_language_from_code(code);
                    }
                }
            }
        }
    }
}

const char* tr(const char* key) {
    const auto& map = get_translations(s_current);
    auto it = map.find(key);
    if (it != map.end())
        return it->second.c_str();
    // Fallback to English
    if (s_current != Language::English) {
        auto it2 = s_en.find(key);
        if (it2 != s_en.end())
            return it2->second.c_str();
    }
    return key;
}

void i18n_set_language(Language lang, const std::string& config_dir) {
    s_current = lang;

    // Persist to settings.json
    std::string settings_path = config_dir + "/settings.json";

    // Ensure config dir exists
    mkdir(config_dir.c_str(), 0755);

    std::ofstream file(settings_path);
    if (file.is_open()) {
        file << "{\n  \"language\": \"" << i18n_language_code(lang) << "\"\n}\n";
    }
}

Language i18n_get_language() {
    return s_current;
}

const char* i18n_language_name(Language lang) {
    switch (lang) {
        case Language::English: return "English";
        case Language::German:  return "Deutsch";
        case Language::Spanish: return u8"Español";
        case Language::French:  return u8"Français";
        case Language::Italian: return "Italiano";
        case Language::Hindi:   return u8"हिन्दी";
        case Language::Chinese: return u8"中文";
        case Language::Russian: return u8"Русский";
        default:                return "English";
    }
}

const char* i18n_language_code(Language lang) {
    switch (lang) {
        case Language::English: return "en";
        case Language::German:  return "de";
        case Language::Spanish: return "es";
        case Language::French:  return "fr";
        case Language::Italian: return "it";
        case Language::Hindi:   return "hi";
        case Language::Chinese: return "zh";
        case Language::Russian: return "ru";
        default:                return "en";
    }
}

std::vector<Language> i18n_available_languages() {
    return {
        Language::English,
        Language::German,
        Language::Spanish,
        Language::French,
        Language::Italian,
        Language::Hindi,
        Language::Chinese,
        Language::Russian
    };
}

Language i18n_language_from_code(const std::string& code) {
    if (code == "de") return Language::German;
    if (code == "es") return Language::Spanish;
    if (code == "fr") return Language::French;
    if (code == "it") return Language::Italian;
    if (code == "hi") return Language::Hindi;
    if (code == "zh") return Language::Chinese;
    if (code == "ru") return Language::Russian;
    return Language::English;
}
