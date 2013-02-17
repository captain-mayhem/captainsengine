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
        VIDEO = 20,
        INVALID = 100,
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

        public bool NoPngToJpeg;
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
        void setStateName(int state, string name);
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
            if (frame < cs.frames.Count)
            {
                return cs.frames[frame] != null;
            }
            return false;
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

        public void setStateName(int state, string name) { }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        AdvData mData;
    }

    public struct ItemState : ICloneable
    {
        public System.Collections.ArrayList frames;
        public int fpsDivider;

        public Object Clone()
        {
            ItemState state = new ItemState();
            state.fpsDivider = fpsDivider;
            state.frames = (ArrayList)frames.Clone();
            return state;
        }
    }

    public class Item : IStateFrameData
    {
        public Item(AdvData data)
        {
            mData = data;
        }
        public Item(AdvData data, int numStates)
        {
            mData = data;
            Name = "Item" + (mData.NumItems+1);
            for (int state = 0; state < numStates; ++state)
            {
                ItemState ist;
                ist.frames = new System.Collections.ArrayList();
                ist.fpsDivider = 20;
                Add(ist);
            }
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
        public void setStateName(int state, string name) { }

        public Item duplicate(string name)
        {
            Item ret = new Item(mData);
            ret.mName = name;
            for (int i = 0; i < mStates.Count; ++i)
            {
                ItemState state = (ItemState)mStates[i];
                ret.mStates.Add(state.Clone());
            }
            //duplicate script
            Script scr = mData.getScript(Script.Type.ITEM, mName);
            Script newscr = scr.duplicate(name);
            mData.addScript(newscr);
            return ret;
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        string mName;
        AdvData mData;
    }

    class ExtendedFrame : ICloneable
    {
        public ExtendedFrame()
        {
            names = new ArrayList();
            offsets = new ArrayList();
        }
        public Object Clone(){
            ExtendedFrame frm = new ExtendedFrame();
            frm.names = (ArrayList)names.Clone();
            frm.offsets = (ArrayList)offsets.Clone();
            return frm;
        }
        public ArrayList names;
        public ArrayList offsets;
    }

    public class ObjectState : ICloneable
    {
        public ObjectState()
        {
            frames = new ArrayList();
            fpsDivider = 20;
        }
        public Object Clone()
        {
            ObjectState state = new ObjectState();
            state.fpsDivider = fpsDivider;
            state.frames = new ArrayList(frames.Count);
            for (int i = 0; i < frames.Count; ++i)
            {
                ExtendedFrame frm = (ExtendedFrame)frames[i];
                state.frames.Add(frm.Clone());
            }
            return state;
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
        public AdvObject(AdvData data, int numStates)
        {
            mData = data;
            Name = "Object" + (mData.NumObjects + 1);
            mSize = new Vec2i(100, 100);
            for (int state = 0; state < numStates; ++state)
            {
                ObjectState ost = new ObjectState();
                ost.frames = new System.Collections.ArrayList();
                ost.fpsDivider = 20;
                Add(ost);
            }
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
            {
                extfrm.names.Add("");
                extfrm.offsets.Add(new Vec2i());
            }
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

        public void setStateName(int state, string name) { }

        public AdvObject duplicate(string name)
        {
            AdvObject ret = new AdvObject(mData);
            ret.mName = name;
            ret.mSize = mSize;
            ret.mLighten = mLighten;
            for (int i = 0; i < mStates.Count; ++i)
            {
                ObjectState state = (ObjectState)mStates[i];
                ret.mStates.Add(state.Clone());
            }
            return ret;
        }

        System.Collections.ArrayList mStates = new System.Collections.ArrayList();
        string mName;
        Vec2i mSize;
        bool mLighten;
        AdvData mData;
    }

    public class CharacterState : ICloneable
    {
        public CharacterState()
        {
            frames = new ArrayList();
            fpsDivider = 20;
            size = new Vec2i(120, 200);
            basepoint = new Vec2i(60, 199);
        }

        public Object Clone()
        {
            CharacterState cs = new CharacterState();
            cs.size = size;
            cs.basepoint = basepoint;
            cs.frames = new ArrayList(frames.Count);
            for (int i = 0; i < frames.Count; ++i)
            {
                ExtendedFrame frm = (ExtendedFrame)frames[i];
                cs.frames.Add(frm.Clone());
            }
            cs.fpsDivider = fpsDivider;
            return cs;
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
        public AdvCharacter(AdvData data, int numStates)
        {
            mData = data;
            Name = "Character" + (mData.NumCharacters+1);
            mWalkSpeed = 5;
            mZoom = 100;
            mTextColor = (UInt32)(mRand.Next(255) << 16 | mRand.Next(255) << 8 | mRand.Next(255));
            for (int i = 1; i <= numStates - 16; ++i)
            {
                mExtraStateNames.Add("");
            }
            for (int state = 0; state < numStates; ++state)
            {
                CharacterState cst = new CharacterState();
                cst.frames = new System.Collections.ArrayList();
                cst.size = new Vec2i(120, 200);
                cst.basepoint = new Vec2i(60, 198);
                cst.fpsDivider = 20;
                Add(cst);
            }
        }
        public int Add(CharacterState os)
        {
            return mStates.Add(os);
        }
        public bool frameExists(int state, int frame)
        {
            CharacterState cs = (CharacterState)mStates[state];
            if (frame < cs.frames.Count)
            {
                ExtendedFrame frm = (ExtendedFrame)cs.frames[frame];
                for (int i = 0; i < frm.names.Count; ++i)
                {
                    if (frm.names[i] != null)
                        return true;
                }
            }
            return false;
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
            {
                extfrm.names.Add("");
                extfrm.offsets.Add(new Vec2i());
            }
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
            return getStateName(state, true);
        }
        public string getStateName(int state, bool defaultNames)
        {
            string name = (string)mExtraStateNames[state - 16];
            if (name.Length == 0 && defaultNames)
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

        public AdvCharacter duplicate(string name)
        {
            AdvCharacter ret = new AdvCharacter(mData);
            ret.mStates = new ArrayList(mStates.Count);
            for (int i = 0; i < mStates.Count; ++i)
            {
                CharacterState cs = (CharacterState)mStates[i];
                ret.mStates.Add(cs.Clone());
            }
            ret.mName = name;
            ret.mTextColor = mTextColor;
            ret.mWalkSpeed = mWalkSpeed;
            ret.mNotZoom = mNotZoom;
            ret.mRealLeft = mRealLeft;
            ret.mMemResistent = mMemResistent;
            ret.mGhost = mGhost;
            ret.mWalkSound = mWalkSound;
            ret.mExtraStateNames = (ArrayList)mExtraStateNames.Clone();
            ret.mFontID = mFontID;
            ret.mZoom = mZoom;
            return ret;
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
        static Random mRand = new Random();
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
            WALKMAP,
            MAX
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
        static public string toScriptName(string obj, string room)
        {
            return obj + ";" + room;
        }
        static public string toScriptName(int x, int y, string room, AdvData data)
        {
            if (data != null && data.Reader.MajorVersion < 3)
                return String.Format("{0:D2}{1:D2}{2}", x + 1, y + 1, room);
            else
                return String.Format("#{0:D3}{1:D3}{2}", x + 1, y + 1, room);
        }
        public Script duplicate(string name)
        {
            Script ret = new Script(mType);
            ret.mName = name;
            ret.mText = mText;
            return ret;
        }
        Type mType;
        string mName;
        string mText;
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
                if (parts[i] == null)
                    continue;
                System.Drawing.Bitmap bmp = mData.getImage(parts[i]);
                Vec2i offset = mData.getFramePartOffset(state - 1, 0, i);
                g.DrawImage(bmp, position.x + offset.x, position.y + offset.y, bmp.Width, bmp.Height);
            }
        }
        public bool isHit(int state, Vec2i position)
        {
            if (position.x < 0 || position.y < 0)
                return false;
            Vec2i size = mData.getSize(state);
            if (position.x > size.x || position.y > size.y)
                return false;
            return true;
        }
        public void drawBoundary(Graphics g, int state, Vec2i position, Color bordercolor)
        {
            Pen p = new Pen(bordercolor);
            const int span = 4;
            g.DrawLine(p, position.x - 1, position.y - 1, position.x + span, position.y - 1);
            g.DrawLine(p, position.x - 1, position.y - 1, position.x - 1, position.y + span);
            Vec2i size = mData.getSize(state);
            g.DrawLine(p, position.x + size.x, position.y - 1, position.x + size.x - span -1 , position.y - 1);
            g.DrawLine(p, position.x + size.x, position.y - 1, position.x + size.x, position.y + span);

            g.DrawLine(p, position.x - 1, position.y + size.y, position.x + span, position.y + size.y);
            g.DrawLine(p, position.x - 1, position.y + size.y, position.x - 1, position.y + size.y - span-1);

            g.DrawLine(p, position.x +size.x, position.y + size.y, position.x + size.x -span - 1, position.y + size.y);
            g.DrawLine(p, position.x +size.x, position.y + size.y, position.x + size.x, position.y + size.y - span - 1);
        }
        protected IStateFrameData mData;
    }

    public class AdvData
    {
        public AdvData(Persistence persist)
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

            Settings.NoPngToJpeg = true;

            mImages = new Dictionary<string, string>();
            mSounds = new Dictionary<string, string>();
            mMusic = new Dictionary<string, string>();
            mVideos = new Dictionary<string, string>();
            mImageCache = new Dictionary<string, System.Drawing.Bitmap>();
            mReader = new AdvFileReader(persist);

            mCursor = new Cursor(this);
            mCursor.init();

            mItems = new Dictionary<string, Item>();
            mObjects = new Dictionary<string, AdvObject>();
            mCharacters = new Dictionary<string, AdvCharacter>();
            mCharacterInstances = new Dictionary<string, ArrayList>();
            mRooms = new Dictionary<string, Room>();
            mScripts = new Dictionary<string, Script>[(int)Script.Type.MAX];
            mLanguages = new Dictionary<string, Language>();
            for (int i = 0; i < (int)Script.Type.MAX; ++i)
            {
                mScripts[i] = new Dictionary<string, Script>();
            }
            Persistence = persist;   
        }

        public AdvData(AdvFileReader reader, Persistence persist)
            : this(persist)
        {
            mCursor = new Cursor(this);
            mReader = reader;
            Settings.NoPngToJpeg = false;
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

        public Dictionary<string, string> Sounds
        {
            get { return mSounds; }
        }

        public Dictionary<string, string> Music
        {
            get { return mMusic; }
        }

        public Dictionary<string, string> Videos
        {
            get { return mVideos; }
        }

        public Cursor Cursor
        {
            get { return mCursor; }
            //set { mCursor = value; }
        }

        public Item getItem(string name)
        {
            return mItems[name.ToLower()];
        }
        public void addItem(Item item)
        {
            mItems.Add(item.Name.ToLower(), item);
        }
        public Item removeItem(string name)
        {
            Item it = getItem(name);
            removeScript(Script.Type.ITEM, it.Name);
            mItems.Remove(name.ToLower());
            return it;
        }
        public int NumItems
        {
            get { return mItems.Count; }
        }
        public Dictionary<string, Item> Items
        {
            get { return mItems; }
        }

        public AdvObject getObject(string name)
        {
            return mObjects[name.ToLower()];
        }
        public void addObject(AdvObject obj)
        {
            mObjects.Add(obj.Name.ToLower(), obj);
        }
        public AdvObject removeObject(string name)
        {
            AdvObject obj = getObject(name);
            foreach (KeyValuePair<string,Room> pair in Rooms)
            {
                pair.Value.removeObject(obj);
            }
            mObjects.Remove(name.ToLower());
            return obj;
        }
        public int NumObjects
        {
            get { return mObjects.Count; }
        }
        public Dictionary<string, AdvObject> Objects
        {
            get { return mObjects; }
        }

        public AdvCharacter getCharacter(string name)
        {
            return mCharacters[name.ToLower()];
        }
        public void addCharacter(AdvCharacter chr)
        {
            mCharacters.Add(chr.Name.ToLower(), chr);
        }
        public AdvCharacter removeCharacter(string name)
        {
            AdvCharacter chr = getCharacter(name);
            foreach (KeyValuePair<string, Room> pair in Rooms)
            {
                pair.Value.removeCharacter(chr);
            }
            mCharacters.Remove(name.ToLower());
            return chr;
        }
        public int NumCharacters
        {
            get { return mCharacters.Count; }
        }
        public Dictionary<string, AdvCharacter> Characters
        {
            get { return mCharacters; }
        }
        public Dictionary<string, ArrayList> CharacterInstances
        {
            get { return mCharacterInstances; }
        }

        public Room getRoom(string name)
        {
            return mRooms[name.ToLower()];
        }
        public void addRoom(Room room)
        {
            mRooms.Add(room.Name.ToLower(), room);
        }
        public Room removeRoom(string name)
        {
            Room rm = getRoom(name);
            while (rm.Objects.Count > 0)
            {
                rm.removeObject((ObjectInstance)rm.Objects[0]);
            }
            while (rm.Characters.Count > 0)
            {
                rm.removeCharacter((CharacterInstance)rm.Characters[0]);
            }
            CharacterInstances.Remove(rm.Name.ToLower());
            rm.removeWalkmapScripts();
            removeScript(Script.Type.ROOM, rm.Name);
            mRooms.Remove(name.ToLower());
            return rm;
        }
        public int NumRooms
        {
            get { return mRooms.Count; }
        }
        public Dictionary<string, Room> Rooms
        {
            get { return mRooms; }
        }

        public void addScript(Script scr)
        {
            getScripts(scr.ScriptType).Add(scr.Name.ToLower(), scr);
        }
        public Script getScript(Script.Type type, string name)
        {
            if (getScripts(type).ContainsKey(name.ToLower()))
                return getScripts(type)[name.ToLower()];
            return null;
        }
        public Script removeScript(Script.Type type, string name)
        {
            Script scr = getScript(type, name);
            getScripts(type).Remove(name.ToLower());
            return scr;
        }
        public Dictionary<string, Script> getScripts(Script.Type type)
        {
            return mScripts[(int)type];
        }

        public void addWalkmapScript(Script scr, Vec2i pos, string roomname)
        {
            addScript(scr);
        }
        public int NumCutscenes
        {
            get {
                return mScripts[(int)Script.Type.CUTSCENE].Count;
            }
        }

        public int WalkGridSize
        {
            get { return Settings.Resolution.x / 32; }
        }

        public void addLanguage(string language, Language.Section section, string text)
        {
            Language lang;
            if (mLanguages.ContainsKey(language.ToLower()))
                lang = mLanguages[language.ToLower()];
            else
            {
                lang = new Language();
                lang.Name = language;
                mLanguages.Add(language.ToLower(), lang);
            }
            lang.addWordWithoutIndex(section, text);
        }

        public void buildLanguageIndex()
        {
            if (mLanguages.Count == 0)
            {
                mLanguages["origin"] = new Language();
                mLanguages["origin"].Name = "origin";
            }
            mLanguages["origin"].buildIndex();
        }

        public Dictionary<string, Language> Languages
        {
            get { return mLanguages; }
        }

        public AdvFileReader Reader
        {
            get { return mReader; }
        }

        public void updateLanguageList()
        {
            PcdkParser parser = new PcdkParser();
            parser.initSyntax();
            parser.Function += new PcdkParser.functionCB(parser_Function);

            Language.beginRefresh();
            foreach (Dictionary<string, Script> scripts in mScripts)
            {
                foreach (KeyValuePair<string, Script> pair in scripts)
                {
                    parser.parseText(pair.Value.Text);
                }
            }
            mLanguages["origin"].addWord(Language.Section.Commands, Settings.WalkText, null);
            mLanguages["origin"].addWord(Language.Section.Commands, Settings.LinkText, null);
            mLanguages["origin"].addWord(Language.Section.Commands, Settings.GiveLink, null);
            foreach (KeyValuePair<string,string> pair in Settings.Commands){
                mLanguages["origin"].addWord(Language.Section.Commands, pair.Value, null);
            }
            Language.endRefresh(this);
        }

        void parser_Function(PcdkParser.Argument funcname, object[] args, int startidx)
        {
            if (funcname.Text == "speech")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[1];
                string voice = "";
                if (args.Length > 2)
                {
                    PcdkParser.Argument voicearg = (PcdkParser.Argument)args[2];
                    if (voicearg.Text != "dontwait")
                        voice = voicearg.Text;
                }
                mLanguages["origin"].addWord(Language.Section.Speech, text.Text, voice);
                return;
            }
            else if (funcname.Text == "offspeech")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[2];
                string voice = "";
                if (args.Length > 3)
                {
                    PcdkParser.Argument voicearg = (PcdkParser.Argument)args[3];
                    if (voicearg.Text != "dontwait")
                        voice = voicearg.Text;
                }
                mLanguages["origin"].addWord(Language.Section.Offspeech, text.Text, voice);
                return;
            }
            else if (funcname.Text == "showinfo")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[0];
                mLanguages["origin"].addWord(Language.Section.Showinfo, text.Text, null);
            }
            else if (funcname.Text == "textout")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[1];
                mLanguages["origin"].addWord(Language.Section.Textout, text.Text, null);
            }
            else if (funcname.Text == "setstring")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[1];
                mLanguages["origin"].addWord(Language.Section.Setstring, text.Text, null);
            }
            else if (funcname.Text == "row")
            {
                PcdkParser.Argument text = (PcdkParser.Argument)args[1];
                mLanguages["origin"].addWord(Language.Section.Textscenes, text.Text, null);
            }
        }

        public ProjectSettings Settings;
        public Persistence Persistence;
        Dictionary<string, string> mImages;
        Dictionary<string, string> mSounds;
        Dictionary<string, string> mMusic;
        Dictionary<string, string> mVideos;
        Dictionary<string, System.Drawing.Bitmap> mImageCache;
        Cursor mCursor;
        Dictionary<string, Item> mItems;
        Dictionary<string, AdvObject> mObjects;
        Dictionary<string, AdvCharacter> mCharacters;
        Dictionary<string, ArrayList> mCharacterInstances;
        Dictionary<string, Room> mRooms;
        Dictionary<string, Script>[] mScripts;
        Dictionary<string, Language> mLanguages;
        AdvFileReader mReader;
    }
}
