#include "i18n.h"

#include <cstring>
#include <string_view>
#include <unordered_map>

#include <platform/services.h>

namespace sd{

namespace{

//! the German texts, mostly the C# StoryDesigner's (its *.de.resx files and
//! Strings.de-DE.resx), and in its words for what it doesn't have
const std::pair<const char*, const char*> kGerman[] = {
  //the main menu
  {"File", "Datei"},
  {"New", "Neu"},
  {"Open...", "Öffnen..."},
  {"Save", "Speichern"},
  {"Save as...", "Speichern unter..."},
  {"Import project (.adz)...", "Projekt importieren (.adz)..."},
  {"Export project (.adz)...", "Projekt exportieren (.adz)..."},
  {"Import character (.adc)...", "Charakter importieren (.adc)..."},
  {"Import room (.adr)...", "Raum importieren (.adr)..."},
  {"Create game...", "Spiel erstellen..."},
  {"Run game", "Spiel starten"},
  {"Quit", "Beenden"},
  {"Insert", "Einfügen"},
  {"Image...", "Bild..."},
  {"Soundeffect...", "Soundeffekt..."},
  {"Musicfile...", "Musikstück..."},
  {"Video...", "Video..."},
  {"Room", "Raum"},
  {"Object", "Objekt"},
  {"Character", "Charakter"},
  {"Item", "Gegenstand"},
  {"Script", "Skript"},
  {"Project", "Projekt"},
  {"Project settings", "Projekteinstellungen"},
  {"Settings", "Grundeinstellungen"},
  {"Commands and bools", "Kommandos und Bools"},
  {"Mouse icons", "Mauszeiger"},
  {"Fonts", "Schriftarten"},
  {"Languages", "Sprach- und Soundtools"},
  {"View", "Ansicht"},
  {"Game pool", "Spiel-Pool"},
  {"Media pool", "Medien-Pool"},
  {"Walkmap on", "Walkmap ein"},
  {"Deepmap on", "Tiefenmap ein"},
  {"Inventoryfield on", "Gegenstandsfeld ein"},
  {"FX Shapes on", "FX Shapes ein"},
  {"Plugins", "Plugins"},
  {"Remote control", "Fernsteuerung"},
  {"Editor settings", "Editor-Einstellungen"},
  {"Language", "Sprache"},
  {"Help", "Hilfe"},
  {"About...", "Über..."},
  {"About StoryDesigner", "Über StoryDesigner"},
  {"Version %s", "Version %s"},
  {"A tool to create Point&Click-Adventures", "Ein Werkzeug, um Point&Click-Adventures zu erstellen"},
  {"Part of Captain's Game Engine, under the LGPL 2.1.", "Teil der Captain's Game Engine, unter der LGPL 2.1."},
  {"IP address / hostname", "IP-Adresse / Hostname"},
  {"Port", "Port"},
  {"Connect", "Verbinden"},
  {"Disconnect", "Trennen"},
  {"Could not connect to CGE", "Keine Verbindung zur CGE"},
  {"Start the game first (Run game).", "Zuerst das Spiel starten (Spiel starten)."},
  {"Script window", "Skriptfenster"},
  {"send", "senden"},
  //the pools
  {"CHARACTERS", "CHARAKTERE"},
  {"SCRIPTS", "SKRIPTE"},
  {"ITEMS", "GEGENSTÄNDE"},
  {"OBJECTS", "OBJEKTE"},
  {"ROOMS", "RÄUME"},
  {"IMAGES", "BILDER"},
  {"MUSIC", "MUSIK"},
  {"New character", "Neuer Charakter"},
  {"New script", "Neues Skript"},
  {"New item", "Neuer Gegenstand"},
  {"New object", "Neues Objekt"},
  {"New room", "Neuer Raum"},
  {"Add images...", "Bilder hinzufügen..."},
  {"Add music...", "Musik hinzufügen..."},
  {"Add sounds...", "Sounds hinzufügen..."},
  {"Add videos...", "Videos hinzufügen..."},
  {"New folder", "Neuer Unterordner"},
  {"Rename...", "Umbenennen..."},
  {"Duplicate", "Duplizieren"},
  {"Delete", "Löschen"},
  {"Open", "Öffnen"},
  //the project settings (the C# Settings dialog)
  {"General", "Generell"},
  {"Project name", "Projektname"},
  {"Aspect ratio:", "Seitenverhältnis:"},
  {"Game resolution", "Auflösung des Spiels"},
  {"No antialiasing", "Kein Antialiasing"},
  {"Mute music when speech", "Musik leise bei Sprache"},
  {"Text on / off", "Text ausschaltbar"},
  {"Draw dragged icon items", "Gegenstandsicons mitziehen"},
  {"Group items", "Gegenstände gruppieren"},
  {"Action text", "Aktionstext"},
  {"Action text height", "Aktionstexthöhe"},
  {"Silent delete", "Stilles Löschen"},
  {"Info line", "Infozeile"},
  {"Protect game files", "Spieldateien schützen"},
  {"\"Loading\" image", "\"Laden\" Bild"},
  {"\"Settings\" picture", "\"Einstellungen\" Bild"},
  {"Script and taskbar", "Skript und Bedienfeld"},
  {"Start cutscene", "Start-Cutscene"},
  {"Main script", "Hauptskript"},
  {"Screen change", "Bildwechsel"},
  {"Direct", "Direkt"},
  {"Fade out to black", "Schwarz überblenden"},
  {"Rectangle", "Rechteck"},
  {"Circle", "Kreis"},
  {"Shutters", "Rolläden"},
  {"Clock", "Uhr"},
  {"Blend", "Überblenden"},
  {"Blend slow", "Langsam Überblenden"},
  {"Taskbar", "Bedienfläche"},
  {"Height", "Höhe"},
  {"Room with the taskbar", "Raum mit Bedienfläche"},
  {"When down, hide completely", "Wenn unten, ganz verdecken"},
  {"Appears from:", "Erscheint von:"},
  {"Top", "Oben"},
  {"Bottom", "Unten"},
  {"Taskbar style:", "Style der Bedienfläche:"},
  {"Always there", "Immer da"},
  {"Direct popup", "Direktes Auftauchen"},
  {"Scrolling popup", "Scrollendes Auftauchen"},
  {"Room for \"anywhere\" objects", "Raum für \"Überall\" Objekte"},
  {"Transparency", "Transparenz"},
  {"Menus and colors", "Menü und Farben"},
  {"Menu colors", "Menüfarben"},
  {"Border", "Rand"},
  {"Background", "Hintergrundfarbe"},
  {"Text", "Text"},
  {"Use custom menu", "Eigenes Menü verwenden"},
  {"Room with custom menu", "Raum mit dem eigenen Menü"},
  {"Fading", "Fading"},
  {"Offspeech color", "Offspeech Farbe"},
  {"Infotext color", "Infotext Farbe"},
  {"Targa transparent color", "Targa Transparenzfarbe"},
  {"Textscene", "Textszene"},
  {"Solid", "Fest"},
  {"Additive", "Additiv"},
  {"None", "Kein"},
  {"Rectangles", "Rechtecke"},
  {"Lines", "Linien"},
  {"Area color", "Flächenfarbe"},
  {"Border color", "Randfarbe"},
  {"Text color", "Textfarbe"},
  {"Selection color", "Auswahlfarbe"},
  {"Appearance", "Erscheinungsbild"},
  {"Use background image", "Hintergrundbild benutzen"},
  {"Coin interface", "Coin-Interface"},
  {"Activate coin interface", "Coin-Interface einschalten"},
  {"Room that contains the interface", "Raum der das Interface enthält"},
  {"Auto-popup with right mouse click", "automatischer Popup bei Rechtsklick"},
  {"Interface center (drag the cross):", "Coin-Interface Mittelpunkt (das Kreuz ziehen):"},
  {"Advanced", "Erweitert"},
  {"Png to Jpeg conversion", "Png nach Jpeg konvertieren"},
  {"Scripting language", "Skriptsprache"},
  {"<none>", "<kein>"},
  //the editor's settings (in the C# Settings dialog too)
  {"Create backups", "Backups erstellen"},
  {"Fit images", "Bilder einpassen"},
  {"Crop transparent image parts", "Transparente Bildteile abschneiden"},
  //messages
  {"Information", "Information"},
  {"Error", "Fehler"},
  {"Done.", "Erledigt."},
  {"Start it now?", "Jetzt starten?"},
  {"The game was created under %s.", "Spiel unter %s erstellt."},
  {"\nLeft out:", "\nWeggelassen:"},
  {"Preparing the game...", "Spiel wird vorbereitet..."},
  {"Save the changes to %s?", "Änderungen an %s speichern?"},
  {"Creating the game...", "Spiel wird erstellt..."},
  {"Yes", "Ja"},
  {"No", "Nein"},
  {"OK", "OK"},
  {"Cancel", "Abbrechen"},
  {"Don't save", "Nicht speichern"},
  {"Unsaved changes", "Ungespeicherte Änderungen"},
  {"Not all media files were found. Project cannot work correctly.",
    "Nicht alle Mediendateien wurden gefunden. Das Projekt kann nicht richtig funktionieren."},
  {"Confirmation", "Bestätigung"},
  {"Preview", "Vorschau"},
  {"(none)", "(kein)"},
  //the editors, which the C# StoryDesigner had in English only; translated
  //so that they don't mix the languages with the texts above
  {"Objects", "Objekte"},
  {"Walkmap", "Walkmap"},
  {"Deepmap", "Tiefenmap"},
  {"Inventory field", "Gegenstandsfeld"},
  {"FX shapes", "FX Shapes"},
  {"(keys 1 to 5)", "(Tasten 1 bis 5)"},
  {"Width", "Breite"},
  {"Zoom factor", "Zoomfaktor"},
  {"Double walkmap", "Doppelte Walkmap"},
  {"Room light", "Raumlicht"},
  {"Edit room script", "Raumskript bearbeiten"},
  {"Edit script", "Skript bearbeiten"},
  {"Edit Script", "Skript bearbeiten"},
  {"Create script", "Skript erstellen"},
  {"Free", "Frei"},
  {"Block", "Blockiert"},
  {"Pen Size 1", "Stiftgröße 1"},
  {"Pen Size 2", "Stiftgröße 2"},
  {"Pen Size 3", "Stiftgröße 3"},
  {"Script Paster", "Skript einfügen"},
  {"Script Eraser", "Skript löschen"},
  {"Copy Walkmap Script", "Walkmap-Skript kopieren"},
  {"Paste Walkmap Script", "Walkmap-Skript einfügen"},
  {"Back", "Hinten"},
  {"Middle", "Mitte"},
  {"Front", "Vorne"},
  {"Set to front", "Nach vorne"},
  {"Set to back", "Nach hinten"},
  {"Locked", "Gesperrt"},
  {"Lighten object", "Objekt aufhellen"},
  {"Unmovable", "Unbeweglich"},
  {"Add as Background", "Als Hintergrund hinzufügen"},
  {"Add as Parallax Background", "Als Parallax-Hintergrund hinzufügen"},
  {"Remove Background", "Hintergrund entfernen"},
  {"Remove Parallax Background", "Parallax-Hintergrund entfernen"},
  {"Remove Field", "Feld entfernen"},
  {"Background image", "Hintergrundbild"},
  {"Background colour", "Hintergrundfarbe"},
  {"Depending on room position", "Abhängig von der Raumposition"},
  {"Do NOT zoom", "NICHT zoomen"},
  {"Ghost mode", "Geistmodus"},
  {"Memory resistant", "Speicherresistent"},
  {"Real left animations", "Echte Links-Animationen"},
  {"Walk text", "Lauftext"},
  {"Link text", "Verbindungstext"},
  {"Give link", "Gib-Verbindung"},
  {"Use mouse wheel", "Mausrad benutzen"},
  {"Font", "Schriftart"},
  {"Size", "Größe"},
  {"Style", "Stil"},
  {"Bold", "Fett"},
  {"Italic", "Kursiv"},
  {"Outline", "Umriss"},
  {"Shadow", "Schatten"},
  {"Fill", "Füllung"},
  {"Spacing", "Abstand"},
  {"Letter spacing", "Zeichenabstand"},
  {"Charset", "Zeichensatz"},
  {"Drawstyle", "Zeichenstil"},
  {"Colors", "Farben"},
  {"State", "Zustand"},
  {"Frame script", "Frame-Skript"},
  {"Copy", "Kopieren"},
  {"Move", "Verschieben"},
  {"Copy action", "Aktion kopieren"},
  {"Paste action", "Aktion einfügen"},
  {"Rename action", "Aktion umbenennen"},
  {"Delete action", "Aktion löschen"},
  {"Equalize all actions (like selected)", "Alle Aktionen angleichen (wie ausgewählt)"},
  {"Equalize all sizes and footprints", "Alle Größen und Fußpunkte angleichen"},
  {"Commands", "Kommandos"},
  {"Command", "Kommando"},
  {"Bools", "Bools"},
  {"Add command", "Kommando hinzufügen"},
  {"Add bool", "Bool hinzufügen"},
  {"Add", "Hinzufügen"},
  {"Remove", "Entfernen"},
  {"Activate", "Aktivieren"},
  {"Customize", "Anpassen"},
  {"Use effect", "Effekt benutzen"},
  {"Loop", "Wiederholen"},
  {"Position:", "Position:"},
  {"Symbol (.ico file)", "Symbol (.ico-Datei)"},
  {"Texts", "Texte"},
  {"Translation", "Übersetzung"},
  {"Gather texts", "Texte sammeln"},
  {"Add language...", "Sprache hinzufügen..."},
  {"Delete language", "Sprache löschen"},
  {"The new language's name:", "Der Name der neuen Sprache:"},
  {"The folder's name:", "Der Name des Ordners:"},
  {"Right click...", "Rechtsklick..."},
  {"up", "hoch"},
  {"down", "runter"},
};

UiLanguage gLanguage = UiLanguage::English;
//! the translated texts by their English text with its ImGui id, kept so
//! that the pointers handed out stay valid
std::unordered_map<std::string, std::string> gCache;

const char* german(std::string_view text){
  static const std::unordered_map<std::string_view, const char*> table = []{
    std::unordered_map<std::string_view, const char*> t;
    for (const auto& entry : kGerman)
      t.emplace(entry.first, entry.second);
    return t;
  }();
  auto it = table.find(text);
  return it == table.end() ? nullptr : it->second;
}

}

void setUiLanguage(UiLanguage language){
  if (language == gLanguage)
    return;
  gLanguage = language;
  gCache.clear();
}

UiLanguage uiLanguage(){
  return gLanguage;
}

UiLanguage systemUiLanguage(){
  for (const std::string& lang : CGE::Platform::getLanguages()){
    if (lang.compare(0, 2, "de") == 0)
      return UiLanguage::German;
    if (lang.compare(0, 2, "en") == 0)
      return UiLanguage::English;
  }
  return UiLanguage::English;
}

const char* tr(const char* text){
  if (gLanguage == UiLanguage::English)
    return text;
  auto cached = gCache.find(text);
  if (cached != gCache.end())
    return cached->second.c_str();
  const char* id = strstr(text, "##");
  std::string_view key = id ? std::string_view(text, id - text) : std::string_view(text);
  const char* translated = german(key);
  std::string result = translated ? std::string(translated) + (id ? id : "") : std::string(text);
  return gCache.emplace(text, std::move(result)).first->second.c_str();
}

const char* windowTitle(const char* title){
  //the same id in every language
  std::string key = std::string("###title") + title;
  auto cached = gCache.find(key);
  if (cached != gCache.end())
    return cached->second.c_str();
  std::string result = std::string(tr(title)) + "###" + title;
  return gCache.emplace(key, std::move(result)).first->second.c_str();
}

}
