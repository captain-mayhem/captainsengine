using System;
using System.Collections.Generic;
using System.Text;

namespace StoryDesigner
{
    enum ResourceID
    {
        SCRIPT = 0,
        ITEM = 1,
        MUSIC = 2,
        OBJECT = 3,
        IMAGE = 4,
        ROOM = 5,
        SOUND = 6,
        FOLDER = 11,
        CHARACTER = 15,
    }

    public enum ScreenChange
    {
        SC_DIRECT = 0,
        SC_FADEOUT,
        SC_RECTANGLE,
        SC_CIRCLE,
        SC_SHUTTERS,
        SC_CLOCK,
        SC_BLEND,
        SC_BLEND_SLOW,
    }

    public struct ProjectSettings
    {
        public string Projectname;
        public string Directory;
        public Vec2i Resolution;
        public string GameIcon;
        public string TsBackground;
        public string Startscript;
        public string Mainscript;
        public string AnywhereRoom;
        public ScreenChange ScreenChange;
        public bool TextOnOff;
        public bool DrawDraggedItemIcons;
        public bool ActionText;
        public bool NotAntialiased;
        public bool GroupItems;
        public int TaskHeight;
        public string TaskRoom;
        public int TaskPopup;
        public bool TaskHideCompletely;
        public int TargaColor;
        public int BorderColor;
        public int BackgroundColor;
        public int TextColor;
        public int TsStyle;
        public int TsBorderStyle;
        public int TsBorderColor;
        public int TsAreaColor;
        public int TsSelectionColor;
        public int TsTextColor;
        public bool MuteMusicWhenSpeech;
    }

    public class AdvData
    {
        public AdvData(AdvFileReader reader)
        {
            Settings = new ProjectSettings();
            mImages = new Dictionary<string, string>();
            mReader = reader;
        }

        public System.Drawing.Bitmap getImage(string filename)
        {
            return mReader.getImage(filename);
        }

        public Dictionary<string, string> Images
        {
            get { return mImages; }
            set { mImages = value; }
        }

        public ProjectSettings Settings;
        Dictionary<string, string> mImages;
        AdvFileReader mReader;
    }
}
