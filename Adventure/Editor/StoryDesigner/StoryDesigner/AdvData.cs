using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Drawing;

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

    public class FontInfo
    {
        public string name;
        public int charset;
        public bool bold;
        public bool italic;
        public int size;
        public int outline;
        public int shadow;
        public int fill;
        public int fading;
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
        public ArrayList Fonts;
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
        Vec2i getSize(int state);
        void setSize(int state, Vec2i size);
        Vec2i getFramePartOffset(int state, int frame, int part);
        void setFramePartOffset(int state, int frame, int part, Vec2i offset);
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
        public Vec2i getSize(int state)
        {
            return new Vec2i(32, 32);
        }
        public void setSize(int state, Vec2i size)
        {
        }
        public Vec2i getFramePartOffset(int state, int frame, int part)
        {
            return new Vec2i(0, 0);
        }
        public void setFramePartOffset(int state, int frame, int part, Vec2i offset)
        {
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
        public Vec2i getSize(int state)
        {
            return new Vec2i(50, 50);
        }
        public void setSize(int state, Vec2i size)
        {
        }
        public Vec2i getFramePartOffset(int state, int frame, int part)
        {
            return new Vec2i(0, 0);
        }
        public void setFramePartOffset(int state, int frame, int part, Vec2i offset)
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

    class ExtendedFrame
    {
        public ExtendedFrame()
        {
            names = new ArrayList();
            offsets = new ArrayList();
        }
        public ArrayList names;
        public ArrayList offsets;
    }

    public class ObjectState
    {
        public ObjectState()
        {
            frames = new ArrayList();
            fpsDivider = 20;
        }
        public System.Collections.ArrayList frames;
        public int fpsDivider;
    }

    public class AdvObject : IStateFrameData
    {
        public AdvObject(AdvData data)
        {
            mData = data;
        }
        public int Add(ObjectState os)
        {
            return mStates.Add(os);
        }
        public bool frameExists(int state, int frame)
        {
            ObjectState os = (ObjectState)mStates[state];
            return frame < os.frames.Count;
        }
        public string[] getFrame(int state, int frame)
        {
            ObjectState os = (ObjectState)mStates[state];
            if (frame >= os.frames.Count)
                return null;
            ExtendedFrame extfrm = (ExtendedFrame)os.frames[frame];
            string[] ret = new string[extfrm.names.Count];
            for (int i = 0; i < extfrm.names.Count; ++i)
            {
                ret[i] = (string)extfrm.names[i];
            }
            //string[] ret = (string[])extfrm.names.ToArray(typeof(string[]));
            return ret;
        }
        public System.Drawing.Bitmap getImage(string framepart)
        {
            return mData.getImage(framepart);
        }
        public void setFramePart(int state, int frame, int part, string name)
        {
            ObjectState os = (ObjectState)mStates[state];
            while (frame >= os.frames.Count)
                os.frames.Add(new ExtendedFrame());
            ExtendedFrame extfrm = (ExtendedFrame)os.frames[frame];
            while (part >= extfrm.names.Count)
                extfrm.names.Add("");
            extfrm.names[part] = name;
        }
        public int getFPSDivider(int state)
        {
            ObjectState os = (ObjectState)mStates[state];
            return os.fpsDivider;
        }
        public void setFPSDivider(int state, int fpsdivider)
        {
            ObjectState os = (ObjectState)mStates[state];
            os.fpsDivider = fpsdivider;
            mStates[state] = os;
        }
        public Vec2i getHotspot(int state)
        {
            return new Vec2i();
        }
        public void setHotspot(int state, Vec2i hotspot)
        {
        }
        public Vec2i getSize(int state)
        {
            return mSize;
        }
        public void setSize(int state, Vec2i size)
        {
            mSize = size;
        }
        public Vec2i getFramePartOffset(int state, int frame, int part)
        {
            ObjectState os = (ObjectState)mStates[state];
            if (frame >= os.frames.Count)
                return new Vec2i(0,0);
            ExtendedFrame extfrm = (ExtendedFrame)os.frames[frame];
            if (part >= extfrm.offsets.Count)
                return new Vec2i(0,0);
            return (Vec2i)extfrm.offsets[part];
        }
        public void setFramePartOffset(int state, int frame, int part, Vec2i offset)
        {
            ObjectState os = (ObjectState)mStates[state];
            ExtendedFrame extfrm = (ExtendedFrame)os.frames[frame];
            extfrm.offsets[part] = offset;
        }

        public string Name
        {
            get { return mName; }
            set { mName = value; }
        }
        public bool Lighten
        {
            get { return mLighten; }
            set { mLighten = value; }
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        string mName;
        Vec2i mSize;
        bool mLighten;
        AdvData mData;
    }

    public class CharacterState
    {
        public CharacterState()
        {
            frames = new ArrayList();
            fpsDivider = 20;
            size = new Vec2i(120, 200);
            basepoint = new Vec2i(60, 199);
        }
        public Vec2i size;
        public Vec2i basepoint;
        public System.Collections.ArrayList frames;
        public int fpsDivider;
    }

    public class AdvCharacter : IStateFrameData
    {
        public AdvCharacter(AdvData data)
        {
            mData = data;
        }
        public int Add(CharacterState os)
        {
            return mStates.Add(os);
        }
        public bool frameExists(int state, int frame)
        {
            CharacterState cs = (CharacterState)mStates[state];
            return frame < cs.frames.Count;
        }
        public string[] getFrame(int state, int frame)
        {
            CharacterState cs = (CharacterState)mStates[state];
            if (frame >= cs.frames.Count)
                return null;
            ExtendedFrame extfrm = (ExtendedFrame)cs.frames[frame];
            string[] ret = new string[extfrm.names.Count];
            for (int i = 0; i < extfrm.names.Count; ++i)
            {
                ret[i] = (string)extfrm.names[i];
            }
            //string[] ret = (string[])extfrm.names.ToArray(typeof(string[]));
            return ret;
        }
        public System.Drawing.Bitmap getImage(string framepart)
        {
            return mData.getImage(framepart);
        }
        public void setFramePart(int state, int frame, int part, string name)
        {
            CharacterState cs = (CharacterState)mStates[state];
            while (frame >= cs.frames.Count)
                cs.frames.Add(new ExtendedFrame());
            ExtendedFrame extfrm = (ExtendedFrame)cs.frames[frame];
            while (part >= extfrm.names.Count)
                extfrm.names.Add("");
            extfrm.names[part] = name;
        }
        public int getFPSDivider(int state)
        {
            CharacterState cs = (CharacterState)mStates[state];
            return cs.fpsDivider;
        }
        public void setFPSDivider(int state, int fpsdivider)
        {
            CharacterState cs = (CharacterState)mStates[state];
            cs.fpsDivider = fpsdivider;
            mStates[state] = cs;
        }
        public Vec2i getHotspot(int state)
        {
            CharacterState cs = (CharacterState)mStates[state];
            return cs.basepoint;
        }
        public void setHotspot(int state, Vec2i hotspot)
        {
            CharacterState cs = (CharacterState)mStates[state];
            cs.basepoint = hotspot;
            mStates[state] = cs;
        }
        public Vec2i getSize(int state)
        {
            CharacterState cs = (CharacterState)mStates[state];
            return cs.size;
        }
        public void setSize(int state, Vec2i size)
        {
            CharacterState cs = (CharacterState)mStates[state];
            cs.size = size;
            mStates[state] = cs;
        }
        public Vec2i getFramePartOffset(int state, int frame, int part)
        {
            CharacterState cs = (CharacterState)mStates[state];
            if (frame >= cs.frames.Count)
                return new Vec2i(0,0);
            ExtendedFrame extfrm = (ExtendedFrame)cs.frames[frame];
            if (part >= extfrm.offsets.Count)
                return new Vec2i(0,0);
            return (Vec2i)extfrm.offsets[part];
        }
        public void setFramePartOffset(int state, int frame, int part, Vec2i offset)
        {
            CharacterState cs = (CharacterState)mStates[state];
            ExtendedFrame extfrm = (ExtendedFrame)cs.frames[frame];
            extfrm.offsets[part] = offset;
        }

        public void setStateName(int state, string name)
        {
            while (mExtraStateNames.Count <= state - 16)
            {
                mExtraStateNames.Add("");
            }
            mExtraStateNames[state-16] = name;
        }
        public string getStateName(int state)
        {
            string name = (string)mExtraStateNames[state - 16];
            if (name.Length == 0)
            {
                name = "Extra " + Convert.ToString(state - 15);
                if (state == 32)
                    name += " (Stand left)";
                else if (state == 33)
                    name += " (Walk left)";
                else if (state == 34)
                    name += " (Stand, Talk left)";
                else if (state == 35)
                    name += " (Walk, Talk left)";
            }
            return name;
        }

        public string Name
        {
            get { return mName; }
            set { mName = value; }
        }

        public System.UInt32 TextColor
        {
            get { return mTextColor; }
            set { mTextColor = value; }
        }

        public int WalkSpeed
        {
            get { return mWalkSpeed; }
            set { mWalkSpeed = value; }
        }

        public bool NoZoom
        {
            get { return mNotZoom; }
            set { mNotZoom = value; }
        }

        public bool RealLeftAnimations
        {
            get { return mRealLeft; }
            set { mRealLeft = value; }
        }

        public bool MemoryReistent
        {
            get { return mMemResistent; }
            set { mMemResistent = value; }
        }

        public bool Ghost
        {
            get { return mGhost; }
            set { mGhost = value; }
        }

        public string Walksound
        {
            get { return mWalkSound; }
            set { mWalkSound = value; }
        }

        public int Font
        {
            get { return mFontID; }
            set { mFontID = value; }
        }

        public int Zoom
        {
            get { return mZoom; }
            set { mZoom = value; }
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        string mName;
        System.UInt32 mTextColor;
        int mWalkSpeed;
        bool mNotZoom;
        bool mRealLeft;
        bool mMemResistent;
        bool mGhost;
        string mWalkSound;
        System.Collections.ArrayList mExtraStateNames = new ArrayList();
        int mFontID;
        int mZoom;
        AdvData mData;
    }

    public class Script
    {
        public enum Type
        {
            CUTSCENE,
            ITEM,
            CHARACTER,
            ROOM,
            OBJECT,
            WALKMAP
        }
        public Script(Type t)
        {
            mType = t;
        }
        public string Text
        {
            get {return mText;}
            set { mText = value; }
        }
        public string Name
        {
            get { return mName; }
            set { mName = value; }
        }
        public Type ScriptType
        {
            get { return mType; }
        }
        Type mType;
        string mName;
        string mText;
    }

    public class Room
    {
        public string Name;
        public Vec2i Size;
        public Vec2i ScrollOffset;
        public Vec2i Depthmap;
        public int Zoom;
        public string Background;
        public string ParallaxBackground;
        public bool DoubleWalkmap;
        public ArrayList FXShapes;
        public bool HasInventory;
        public Vec2i InvPos;
        public Vec2i InvSize;
        public Vec2f InvScale;
        public ArrayList Walkmap = new ArrayList();
        public ArrayList Objects = new ArrayList();
        public ArrayList Characters = new ArrayList();
    }

    public class Drawable
    {
        public Drawable(IStateFrameData obj)
        {
            mData = obj;
        }
        public void draw(Graphics g, int state, Vec2i position)
        {
            if (state == 0)
            {
                return;
            }
            string[] parts = mData.getFrame(state - 1, 0);
            if (parts == null)
            {
                return;
            }
            for (int i = 0; i < parts.Length; ++i)
            {
                System.Drawing.Bitmap bmp = mData.getImage(parts[i]);
                Vec2i offset = mData.getFramePartOffset(state - 1, 0, i);
                g.DrawImage(bmp, position.x + offset.x, position.y + offset.y, bmp.Width, bmp.Height);
            }
        }
        protected IStateFrameData mData;
    }

    public abstract class DrawableObject : Drawable{
        public DrawableObject(IStateFrameData data) : base(data) { }
        public abstract void draw(Graphics g);
    }

    public class CharacterInstance : DrawableObject
    {
        public CharacterInstance(AdvCharacter chr) : base(chr){
            Character = chr;
        }
        public override void draw(Graphics g)
        {
            draw(g, LookDir, RawPosition);
        }
        public string Name;
        public AdvCharacter Character;
        public string Room;
        public Vec2i RawPosition
        {
            set { position = value; }
            get { return position; }
        }
        public Vec2i Position
        {
            set { position = value - mData.getHotspot(LookDir); ; }
            get { return position+mData.getHotspot(LookDir); }
        }
        public int LookDir;
        public bool Unmovable;
        public bool Locked;

        private Vec2i position;
    }

    public class ObjectInstance : DrawableObject
    {
        public ObjectInstance(AdvObject obj) : base(obj) {
            Object = obj;
        }
        public override void draw(Graphics g)
        {
            draw(g, State, Position);
        }
        public string Name;
        public AdvObject Object;
        public Vec2i Position;
        public int State;
        public int Layer;
        public int Depth;
        public bool Locked;
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
            Settings.Fonts = new ArrayList();

            mImages = new Dictionary<string, string>();
            mImageCache = new Dictionary<string, System.Drawing.Bitmap>();
            mReader = null;

            mCursor = new Cursor(this);
            mCursor.init();

            mItems = new Dictionary<string, Item>();
            mObjects = new Dictionary<string, AdvObject>();
            mCharacters = new Dictionary<string, AdvCharacter>();
            mRooms = new Dictionary<string, Room>();
            mScripts = new Dictionary<KeyValuePair<Script.Type, string>, Script>();
            mWMScripts = new Dictionary<string, ArrayList>();
        }

        public AdvData(AdvFileReader reader) : this()
        {
            //Settings = new ProjectSettings();
            //mImages = new Dictionary<string, string>();
            mCursor = new Cursor(this);
            mReader = reader;
            //mCursor = new Cursor(this);
            //mItems = new Dictionary<string, Item>();
            //mObjects = new Dictionary<string, AdvObject>();
            //mScripts = new Dictionary<KeyValuePair<Script.Type, string>, Script>();
            //mWMScripts = new Dictionary<string, ArrayList>();
        }

        public System.Drawing.Bitmap getImage(string name)
        {
            string filename = mImages[name.ToLower()];
            if (mImageCache.ContainsKey(filename))
                return mImageCache[filename];
            System.Drawing.Bitmap bmp = mReader.getImage(filename);
            mImageCache[filename] = bmp;
            return bmp;
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

        public AdvObject getObject(string name)
        {
            return mObjects[name.ToLower()];
        }
        public void addObject(AdvObject obj)
        {
            mObjects.Add(obj.Name.ToLower(), obj);
        }

        public AdvCharacter getCharacter(string name)
        {
            return mCharacters[name];
        }
        public void addCharacter(AdvCharacter chr)
        {
            mCharacters.Add(chr.Name.ToLower(), chr);
        }

        public Room getRoom(string name)
        {
            return mRooms[name];
        }
        public void addRoom(Room room)
        {
            mRooms.Add(room.Name.ToLower(), room);
        }

        public void addScript(Script scr)
        {
            mScripts.Add(new KeyValuePair<Script.Type, string>(scr.ScriptType, scr.Name.ToLower()), scr);
        }
        public Script getScript(Script.Type type, string name)
        {
            return mScripts[new KeyValuePair<Script.Type, string>(type, name)];
        }

        public void addWalkmapScript(Script scr, Vec2i pos, string roomname)
        {
        }

        public int WalkGridSize
        {
            get { return Settings.Resolution.x / 32; }
        }

        public ProjectSettings Settings;
        Dictionary<string, string> mImages;
        Dictionary<string, System.Drawing.Bitmap> mImageCache;
        Cursor mCursor;
        Dictionary<string, Item> mItems;
        Dictionary<string, AdvObject> mObjects;
        Dictionary<string, AdvCharacter> mCharacters;
        Dictionary<string, Room> mRooms;
        Dictionary<KeyValuePair<Script.Type, string>, Script> mScripts;
        Dictionary<string, ArrayList> mWMScripts;
        AdvFileReader mReader;
    }
}
