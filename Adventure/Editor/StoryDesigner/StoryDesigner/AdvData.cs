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
        public string LoadingImage;
        public bool TsUseBgImage;
        public string TsBackground;
        public string Startscript;
        public string Mainscript;
        public string AnywhereRoom;
        public ScreenChange ScreenChange;
        public bool TextOnOff;
        public bool DrawDraggedItemIcons;
        public bool ActionText;
        public bool ShowTaskbar;
        public bool TaskbarFromTop;
        public bool NotAntialiased;
        public bool GroupItems;
        public int ActionTextHeight;
        public bool CustomMenu;
        public string CustomMenuRoom;
        public int TaskHeight;
        public string TaskRoom;
        public int TaskPopup;
        public bool SilentDelete;
        public bool InfoLine;
        public bool ProtectGameFile;
        public int MenuFading;
        public int TextSceneFading;
        public bool TaskHideCompletely;
        public int AnywhereTransparency;
        public System.UInt32 TargaColor;
        public System.UInt32 BorderColor;
        public System.UInt32 BackgroundColor;
        public System.UInt32 TextColor;
        public System.UInt32 OffspeechColor;
        public System.UInt32 InfotextColor;
        public int TsStyle;
        public int TsBorderStyle;
        public System.UInt32 TsBorderColor;
        public System.UInt32 TsAreaColor;
        public System.UInt32 TsSelectionColor;
        public System.UInt32 TsTextColor;
        public bool TsUseSymbols;
        public bool MuteMusicWhenSpeech;
        public bool CoinActivated;
        public bool CoinAutoPopup;
        public string CoinRoom;
        public int CoinFading;
        public Vec2i CoinCenter;
        public string LinkText;
        public string GiveLink;
        public string WalkText;
        public Dictionary<string, bool> Booleans;
        public Dictionary<string, string> Commands;
    }

    public struct CursorState
    {
        public System.Collections.ArrayList frames;
        public float fps;
        public int command;
        public Vec2i highlight;
    }

    public class AdvData
    {
        public AdvData(AdvFileReader reader)
        {
            Settings = new ProjectSettings();
            mImages = new Dictionary<string, string>();
            mReader = reader;
            mCursor = new System.Collections.ArrayList();
        }

        public System.Drawing.Bitmap getImage(string filename)
        {
            return mReader.getImage(filename);
        }

        public Dictionary<string, string> Images
        {
            get { return mImages; }
            //set { mImages = value; }
        }

        public System.Collections.ArrayList Cursor
        {
            get { return mCursor; }
            //set { mCursor = value; }
        }

        public ProjectSettings Settings;
        Dictionary<string, string> mImages;
        System.Collections.ArrayList mCursor;
        AdvFileReader mReader;
    }
}
