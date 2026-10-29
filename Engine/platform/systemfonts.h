#ifndef CGE_SYSTEMFONTS_H
#define CGE_SYSTEMFONTS_H

#include <cge_export.h>

#include <system/types.h>

#include <string>
#include <vector>

namespace CGE{

//! The fonts installed on the system, found by their family name
/*! Where the fonts come from:

    | Platform    | Fonts                                                    |
    |-------------|----------------------------------------------------------|
    | Windows     | GDI's fonts                                              |
    | Linux (X11) | fontconfig's fonts                                       |
    | Android     | /system/fonts, and the asset folder fonts/               |
    | Web         | the asset folder fonts/ (the page has no system fonts)   |

    On Windows and Linux the system's font matching finds a font for every
    name, as it does for other applications: a missing family gives a
    substitute (FontData::family says which). On Android and the web the
    fonts are indexed with FreeType when they are first asked for; a name
    that isn't among them goes through a small alias table, so the generic
    names ("sans-serif", "serif", "monospace") and the common Windows fonts
    ("Arial", "Times New Roman", "Courier New", ...) find a similar font,
    e.g. a metric-compatible Liberation font when the application ships one.
    Family names are UTF-8 and compared without regard to case where the
    platform does the matching itself. */
class CGE_API SystemFonts{
public:
  //! a font file's contents and the face it holds
  struct FontData{
    //! the whole file, for FreeType (Font::load)
    std::vector<uint8> data;
    //! the face in a font collection (.ttc), 0 otherwise
    int faceIndex = 0;
    //! the family that was found, which can be a substitute
    std::string family;
  };
  //! the kind of letters a family has, for sorting families in a font picker
  enum Style{
    STYLE_UNKNOWN,
    STYLE_SERIF,
    STYLE_SANS_SERIF,
    STYLE_MONOSPACE,
    STYLE_SCRIPT,
    STYLE_DECORATIVE,
  };
  //! an installed family and how it looks
  struct Family{
    std::string name;
    //! from the font's own description (Windows: GDI's; elsewhere the PANOSE or class of the OS/2 table)
    Style style = STYLE_UNKNOWN;
    //! all characters are equally wide
    bool fixedPitch = false;
    //! the characters are symbols (e.g. Wingdings), not letters
    bool symbol = false;
  };
  //! the font for a family, weight (400 normal, 700 bold) and slant
  static bool find(const std::string& family, FontData& font, int weight = 400, bool italic = false);
  //! is a family of this name installed (without substitutes)
  static bool exists(const std::string& family);
  //! the installed families, sorted, each once
  static std::vector<std::string> getFamilies();
  //! the installed families with their look, sorted, each once
  /*! Slower than getFamilies() on Linux, where a font file of every family is opened. */
  static std::vector<Family> getFamilyDetails();
};

}

#endif
