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
        public int RightClick;
        public bool UseMouseWheel;
    }

    public interface IStateFrameData{
        bool frameExists(int state, int frame);
        string[] getFrame(int state, int frame);
        System.Drawing.Bitmap getImage(string framepart);
        int getFPSDivider(int state);
        void setFPSDivider(int state, int fpsdivider);
        void setFramePart(int state, int frame, int part, string name);
        Vec2i getHotspot(int state);
        void setHotspot(int state, Vec2i hotspot);
    };

    public struct CursorState
    {
        public System.Collections.ArrayList frames;
        public int fpsDivider;
        public int command;
        public Vec2i highlight;
    }

    public class Cursor : IStateFrameData
    {
        public Cursor(AdvData data)
        {
            mData = data;
        }
        public int Add(CursorState cs)
        {
            return mStates.Add(cs);
        }
        public void init()
        {
            mStates.Clear();
            for (int i = 0; i < 10; ++i)
            {
                CursorState cs;
                cs.command = i == 9 ? 2 : 1;
                cs.fpsDivider = 20;
                cs.frames = new System.Collections.ArrayList();
                cs.highlight = new Vec2i(1, 1);
                mStates.Add(cs);
            }
        }
        public bool frameExists(int state, int frame)
        {
            CursorState cs = (CursorState)mStates[state];
            return frame < cs.frames.Count;
        }
        public string[] getFrame(int state, int frame)
        {
            CursorState cs = (CursorState)mStates[state];
            if (frame >= cs.frames.Count)
                return null;
            string[] ret = new string[1];
            ret[0] = (string)cs.frames[frame];
            return ret;
        }
        public System.Drawing.Bitmap getImage(string framepart)
        {
            return mData.getImage(framepart);
        }
        public void setFramePart(int state, int frame, int part, string name)
        {
            CursorState cs = (CursorState)mStates[state];
            while (frame >= cs.frames.Count)
                cs.frames.Add("");
            cs.frames[frame] = name;
            mStates[state] = cs;
        }
        public int getFPSDivider(int state)
        {
            CursorState cs = (CursorState)mStates[state];
            return cs.fpsDivider;
        }
        public void setFPSDivider(int state, int fpsdivider)
        {
            CursorState cs = (CursorState)mStates[state];
            cs.fpsDivider = fpsdivider;
            mStates[state] = cs;
        }
        public Vec2i getHotspot(int state)
        {
            CursorState cs = (CursorState)mStates[state];
            return cs.highlight;
        }
        public void setHotspot(int state, Vec2i hotspot)
        {
            CursorState cs = (CursorState)mStates[state];
            cs.highlight = hotspot;
            mStates[state] = cs;
        }

        public void setCommand(int state, int command)
        {
            CursorState cs = (CursorState)mStates[state];
            cs.command = command+1;
            mStates[state] = cs;
        }
        public int getCommand(int state)
        {
            CursorState cs = (CursorState)mStates[state];
            return cs.command-1;
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        AdvData mData;
    }

    public struct ItemState
    {
        public System.Collections.ArrayList frames;
        public int fpsDivider;
    }

    public class Item : IStateFrameData
    {
        public Item(AdvData data)
        {
            mData = data;
        }
        public int Add(ItemState ist)
        {
            return mStates.Add(ist);
        }
        public bool frameExists(int state, int frame)
        {
            ItemState ist = (ItemState)mStates[state];
            return frame < ist.frames.Count;
        }
        public string[] getFrame(int state, int frame)
        {
            ItemState ist = (ItemState)mStates[state];
            if (frame >= ist.frames.Count)
                return null;
            string[] ret = new string[1];
            ret[0] = (string)ist.frames[frame];
            return ret;
        }
        public System.Drawing.Bitmap getImage(string framepart)
        {
            return mData.getImage(framepart);
        }
        public void setFramePart(int state, int frame, int part, string name)
        {
            ItemState ist = (ItemState)mStates[state];
            while (frame >= ist.frames.Count)
                ist.frames.Add("");
            ist.frames[frame] = name;
            mStates[state] = ist;
        }
        public int getFPSDivider(int state)
        {
            ItemState ist = (ItemState)mStates[state];
            return ist.fpsDivider;
        }
        public void setFPSDivider(int state, int fpsdivider)
        {
            ItemState ist = (ItemState)mStates[state];
            ist.fpsDivider = fpsdivider;
            mStates[state] = ist;
        }
        public Vec2i getHotspot(int state)
        {
            return new Vec2i();
        }
        public void setHotspot(int state, Vec2i hotspot)
        {
        }

        public string Name
        {
            get { return mName; }
            set { mName = value; }
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        string mName;
        AdvData mData;
    }

    public class AdvData
    {
        public AdvData()
        {
            Settings = new ProjectSettings();
            Settings.ActionText = true;
            Settings.ActionTextHeight = 0;
            Settings.AnywhereRoom = "";
            Settings.AnywhereTransparency = 0;
            Settings.BackgroundColor = 32768;
            Settings.Booleans = new Dictionary<string, bool>();
            Settings.BorderColor = 8421376;
            Settings.CoinActivated = false;
            Settings.CoinAutoPopup = true;
            Settings.CoinCenter = new Vec2i(65, 90);
            Settings.CoinFading = 0;
            Settings.CoinRoom = "";
            Settings.Commands = new Dictionary<string, string>();
            Settings.CustomMenu = true;
            Settings.CustomMenuRoom = "";
            Settings.Directory = "";
            Settings.DrawDraggedItemIcons = false;
            Settings.GameIcon = "";
            Settings.GiveLink = "to";
            Settings.GroupItems = false;
            Settings.InfoLine = true;
            Settings.InfotextColor = 16777215;
            Settings.LinkText = "with";
            Settings.LoadingImage = "";
            Settings.Mainscript = "";
            Settings.MenuFading = 0;
            Settings.MuteMusicWhenSpeech = true;
            Settings.NotAntialiased = false;
            Settings.OffspeechColor = 16777215;
            Settings.Projectname = "New Project";
            Settings.ProtectGameFile = false;
            Settings.Resolution = new Vec2i(640, 480);
            Settings.ScreenChange = ScreenChange.SC_DIRECT;
            Settings.ShowTaskbar = true;
            Settings.SilentDelete = false;
            Settings.Startscript = "";
            Settings.TargaColor = 16711935;
            Settings.TaskbarFromTop = false;
            Settings.TaskHeight = 140;
            Settings.TaskHideCompletely = true;
            Settings.TaskPopup = 0;
            Settings.TaskRoom = "";
            Settings.TextColor = 65280;
            Settings.TextOnOff = false;
            Settings.TextSceneFading = 0;
            Settings.TsAreaColor = 8421504;
            Settings.TsBackground = "";
            Settings.TsBorderColor = 0;
            Settings.TsBorderStyle = 0;
            Settings.TsSelectionColor = 12632256;
            Settings.TsStyle = 0;
            Settings.TsTextColor = 15793151;
            Settings.TsUseBgImage = false;
            Settings.TsUseSymbols = false;
            Settings.WalkText = "Walk to";
            Settings.RightClick = 0;
            Settings.UseMouseWheel = false;

            mImages = new Dictionary<string, string>();
            mReader = null;

            mCursor = new Cursor(this);
            mCursor.init();

            mItems = new Dictionary<string, Item>();
        }

        public AdvData(AdvFileReader reader)
        {
            Settings = new ProjectSettings();
            mImages = new Dictionary<string, string>();
            mReader = reader;
            mCursor = new Cursor(this);
            mItems = new Dictionary<string, Item>();
        }

        public System.Drawing.Bitmap getImage(string name)
        {
            string filename = mImages[name.ToLower()];
            return mReader.getImage(filename);
        }

        public Dictionary<string, string> Images
        {
            get { return mImages; }
            //set { mImages = value; }
        }

        public Cursor Cursor
        {
            get { return mCursor; }
            //set { mCursor = value; }
        }

        public Item getItem(string name)
        {
            return mItems[name];
        }
        public void addItem(Item item)
        {
            mItems.Add(item.Name.ToLower(), item);
        }

        public ProjectSettings Settings;
        Dictionary<string, string> mImages;
        Cursor mCursor;
        Dictionary<string, Item> mItems;
        AdvFileReader mReader;
    }
}
