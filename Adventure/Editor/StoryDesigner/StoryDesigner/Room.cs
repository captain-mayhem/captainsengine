using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Drawing;

namespace StoryDesigner
{
    public class FxShape : ICloneable
    {
        public FxShape() { }
        public FxShape(int num)
        {
            Active = false;
            DependingOnRoomPosition = true;
            Effect = FxEffect.FLOOR_MIRROR;
            Depth = 15;
            MirrorOffset = new Vec2i(-10, -7);
            Strength = 125;
            Positions[0] = new Vec2i(105 * (num + 1), 100);
            Positions[1] = new Vec2i(Positions[0].x + 100, 100);
            Positions[2] = new Vec2i(Positions[1].x, 200);
            Positions[3] = new Vec2i(Positions[0].x, 200);
        }
        public Object Clone()
        {
            FxShape shape = new FxShape();
            shape.Active = Active;
            shape.DependingOnRoomPosition = DependingOnRoomPosition;
            shape.Effect = Effect;
            shape.Room = Room;
            shape.Depth = Depth;
            shape.MirrorOffset = MirrorOffset;
            shape.Strength = Strength;
            shape.Positions = (Vec2i[])Positions.Clone();
            return shape;
        }
        public enum FxEffect
        {
            FLOOR_MIRROR = 0,
            WALL_MIRROR = 1,
            PARTICLE_BARRIER = 2
        };
        public bool Active;
        public bool DependingOnRoomPosition;
        public FxEffect Effect;
        public string Room;
        public int Depth;
        public Vec2i MirrorOffset;
        public int Strength;
        public Vec2i[] Positions = new Vec2i[4];
    }

    public class Room
    {
        public Room()
        {
        }
        public Room(AdvData data)
        {
            mData = data;
            Name = "Room" + (data.NumRooms + 1);
            Size = new Vec2i(data.WindowXRes, data.Settings.Resolution.y);
            Depthmap = new Vec2i(5, 10);
            Zoom = 3;
            Background = "";
            ParallaxBackground = "";
            FXShapes = new ArrayList();
            for (int i = 0; i < 3; ++i)
            {
                FxShape fs = new FxShape(i);
                FXShapes.Add(fs);
            }
            Vec2i wmsize = data.Settings.Resolution / data.WalkGridSize;
            wmsize.x *= 3 * 2;
            wmsize.y *= 2 * 2;
            Walkmap = new WalkMapEntry[wmsize.x, wmsize.y];
            Lighting = Color.White;
            InvScale = new Vec2f(1.0f, 1.0f);
            InvSpacing = 10;
        }
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
        public int InvSpacing;
        public ArrayList Objects = new ArrayList();
        public ArrayList Characters = new ArrayList();
        public struct WalkMapEntry
        {
            public bool isFree;
            public bool hasScript;
        }
        public WalkMapEntry[,] Walkmap = null;
        public Color Lighting;
        private AdvData mData;
        public AdvData Data
        {
            set { mData = value; }
        }

        public void removeObject(ObjectInstance obj)
        {
            mData.removeScript(Script.Type.OBJECT, Script.toScriptName(obj.Name, Name));
            Objects.Remove(obj);
        }

        public void removeObject(AdvObject obj)
        {
            ArrayList removes = new ArrayList();
            foreach (ObjectInstance inst in Objects)
            {
                if (inst.Object == obj)
                    removes.Add(inst);
            }
            foreach (ObjectInstance inst in removes)
            {
                removeObject(inst);
            }
        }

        public void removeCharacter(CharacterInstance chr)
        {
            mData.removeScript(Script.Type.CHARACTER, chr.Name);
            Characters.Remove(chr);
            mData.CharacterInstances[Name.ToLower()].Remove(chr);
        }

        public void removeCharacter(AdvCharacter chr)
        {
            ArrayList removes = new ArrayList();
            foreach (CharacterInstance inst in Characters)
            {
                if (inst.Character == chr)
                    removes.Add(inst);
            }
            foreach (CharacterInstance inst in removes)
            {
                removeCharacter(inst);
            }
        }

        public void removeWalkmapScripts()
        {
            for (int x = 0; x <= Walkmap.GetUpperBound(0); ++x)
            {
                for (int y = 0; y <= Walkmap.GetUpperBound(1); ++y)
                {
                    if (Walkmap[x, y].hasScript)
                    {
                        mData.removeScript(Script.Type.WALKMAP, Script.toScriptName(x, y, Name, mData));
                    }
                }
            }
        }

        public void rename(string name)
        {
            Script scr = mData.getScript(Script.Type.ROOM, Name);
            if (scr != null)
            {
                scr.Name = name;
                mData.addScript(scr);
            }
            //rename object scripts
            foreach (ObjectInstance obj in Objects)
            {
                Script objscr = mData.removeScript(Script.Type.OBJECT, Script.toScriptName(obj.Name, Name));
                if (objscr != null)
                {
                    objscr.Name = Script.toScriptName(obj.Name, name);
                    mData.addScript(objscr);
                }
            }
            //rename character instances
            mData.CharacterInstances[name.ToLower()] = new ArrayList();
            if (mData.CharacterInstances.ContainsKey(Name.ToLower()))
            {
                foreach (CharacterInstance chr in mData.CharacterInstances[Name.ToLower()])
                {
                    chr.Room = name;
                    mData.CharacterInstances[name.ToLower()].Add(chr);
                }
                mData.CharacterInstances.Remove(Name.ToLower());
            }
            //rename walkmap scripts
            for (int x = 0; x <= Walkmap.GetUpperBound(0); ++x)
            {
                for (int y = 0; y <= Walkmap.GetUpperBound(1); ++y)
                {
                    if (Walkmap[x, y].hasScript)
                    {
                        Script wmscr = mData.removeScript(Script.Type.WALKMAP, Script.toScriptName(x, y, Name, mData));
                        wmscr.Name = Script.toScriptName(x, y, name, mData);
                        mData.addScript(wmscr);
                    }
                }
            }

            mData.removeScript(Script.Type.ROOM, Name);

            mData.Rooms.Remove(Name.ToLower());
            Name = name;
            mData.addRoom(this);
        }

        public Room duplicate(string name)
        {
            Room rm = new Room();
            rm.mData = mData;
            rm.Name = name;
            rm.Size = Size;
            rm.ScrollOffset = ScrollOffset;
            rm.Depthmap = Depthmap;
            rm.Zoom = Zoom;
            rm.Background = Background;
            rm.ParallaxBackground = ParallaxBackground;
            rm.DoubleWalkmap = DoubleWalkmap;
            rm.FXShapes = new ArrayList(FXShapes.Count);
            for (int i = 0; i < FXShapes.Count; ++i)
            {
                FxShape shape = (FxShape)FXShapes[i];
                rm.FXShapes.Add(shape.Clone());
            }
            rm.HasInventory = HasInventory;
            rm.InvPos = InvPos;
            rm.InvScale = InvScale;
            rm.InvSize = InvSize;
            rm.InvSpacing = InvSpacing;
            rm.Walkmap = (WalkMapEntry[,])Walkmap.Clone();
            rm.Lighting = Lighting;
            foreach (ObjectInstance inst in Objects)
            {
                ObjectInstance newinst = inst.duplicate(Name, rm.Name);
                rm.Objects.Add(newinst);
            }
            foreach (CharacterInstance inst in Characters)
            {
                CharacterInstance newinst = inst.duplicate(rm.Name);
                rm.Characters.Add(newinst);
            }
            Script roomscr = mData.getScript(Script.Type.ROOM, Name);
            mData.addScript(roomscr.duplicate(rm.Name));
            for (int x = 0; x <= Walkmap.GetUpperBound(0); ++x)
            {
                for (int y = 0; y <= Walkmap.GetUpperBound(1); ++y)
                {
                    if (Walkmap[x, y].hasScript)
                    {
                        Script wmscr = mData.getScript(Script.Type.WALKMAP, Script.toScriptName(x, y, Name, mData));
                        mData.addScript(wmscr.duplicate(Script.toScriptName(x, y, rm.Name, mData)));
                    }
                }
            }
            return rm;
        }

        public float getScale(Vec2i pos)
        {
            float scaleStart = Depthmap.y*mData.WalkGridSize;
            float scaleStop = Depthmap.x*mData.WalkGridSize;
            float minVal = 1.0f-(scaleStart-scaleStop)/mData.Settings.Resolution.y*Zoom*0.3f;
            float factor = (pos.y - scaleStart) / (scaleStop - scaleStart);
            factor = factor < 0 ? 0 : factor;
            factor = factor > 1.0f ? 1.0f : factor;
            float scale = (1 - factor) * 1 + (factor) * minVal;
            return scale;
        }
    }

    public abstract class DrawableObject : Drawable
    {
        public DrawableObject(IStateFrameData data) : base(data) { }
        public abstract void draw(Graphics g, bool boundary, Color bordercolor, float scale);
        public abstract bool isHit(Vec2i pos);
        public abstract Vec2i getPosition();
        public abstract void setPosition(Vec2i pos);
        public abstract Vec2i getSize();
        public abstract bool isLocked();
    }

    public class CharacterInstance : DrawableObject
    {
        public CharacterInstance(AdvCharacter chr, AdvData data)
            : base(chr)
        {
            Character = chr;
            mAdvData = data;
            setName(chr.Name);
        }
        public override void draw(Graphics g, bool boundary, Color bordercolor, float scale)
        {
            int state = LookDir;
            if (LookDir == 4)
                state = 3;
            if (Character.NoZoom)
                scale = 1.0f;
            scale *= Character.Zoom / 100.0f;
            Vec2i size = mData.getSize(state-1);
            Bitmap bmp = new Bitmap(size.x, size.y);
            Graphics gbmp = Graphics.FromImage(bmp);
            draw(gbmp, state, new Vec2i(0, 0));
            if (LookDir == 4)
                bmp.RotateFlip(RotateFlipType.RotateNoneFlipX);
            Vec2i renderPos = RawPosition + mData.getHotspot(state - 1) - mData.getHotspot(state - 1) * scale;
            if (LookDir == 4)
            {
                renderPos.x += (int)((-mData.getSize(state - 1).x + 2 * mData.getHotspot(state - 1).x) * scale);
            }
            g.DrawImage(bmp, renderPos.x, renderPos.y, size.x*scale, size.y*scale);
            if (boundary)
                drawBoundary(g, state, RawPosition, bordercolor);
        }
        public override bool isHit(Vec2i pos)
        {
            return isHit(LookDir, pos - RawPosition);
        }
        public override Vec2i getPosition()
        {
            return Position;
        }
        public override void setPosition(Vec2i pos)
        {
            Position = pos;
        }
        public override Vec2i getSize()
        {
            int state = LookDir;
            if (LookDir == 4)
                state = 3;
            return mData.getSize(state);
        }
        public override bool isLocked()
        {
            return Locked;
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
            set { position = value - mData.getHotspot(LookDir == 4 ? 3 : LookDir); ; }
            get { return position + mData.getHotspot(LookDir == 4 ? 3 : LookDir); }
        }

        private void setName(string basename)
        {
            Name = basename;
            int count = 0;
            foreach (KeyValuePair<string, System.Collections.ArrayList> kvp in mAdvData.CharacterInstances)
            {
                foreach (CharacterInstance ci in kvp.Value)
                {
                    if (ci.Character == Character)
                        ++count;
                }
            }
            if (count > 0)
                Name += count;
        }

        public CharacterInstance duplicate(string roomName)
        {
            CharacterInstance inst = new CharacterInstance(Character, mAdvData);
            inst.Room = roomName;
            inst.position = position;
            inst.LookDir = LookDir;
            inst.Unmovable = Unmovable;
            inst.Locked = Locked;
            if (!mAdvData.CharacterInstances.ContainsKey(roomName.ToLower()))
                mAdvData.CharacterInstances.Add(roomName.ToLower(), new System.Collections.ArrayList());
            mAdvData.CharacterInstances[roomName.ToLower()].Add(inst);
            Script scr = mAdvData.getScript(Script.Type.CHARACTER, Name);
            if (scr != null)
            {
                Script newscr = scr.duplicate(inst.Name);
                mAdvData.addScript(newscr);
            }
            return inst;
        }
        public void setLookDir(int lookDir)
        {
            int oldstate = LookDir == 4 ? 3 : LookDir;
            int newstate = lookDir == 4 ? 3 : lookDir;
            Vec2i oldoffset = mData.getHotspot(oldstate-1);
            Vec2i newoffset = mData.getHotspot(newstate-1);
            position = position + oldoffset - newoffset;
            LookDir = lookDir;
        }
        public int LookDir;
        public bool Unmovable;
        public bool Locked;

        private Vec2i position;
        private AdvData mAdvData;
    }

    public class ObjectInstance : DrawableObject
    {
        public ObjectInstance(AdvObject obj, AdvData data)
            : base(obj)
        {
            Object = obj;
            mAdvData = data;
        }
        public override void draw(Graphics g, bool boundary, Color bordercolor, float scale)
        {
            Vec2i size = mData.getSize(State - 1);
            Bitmap bmp = new Bitmap(size.x, size.y);
            Graphics gbmp = Graphics.FromImage(bmp);
            draw(gbmp, State, new Vec2i());
            g.DrawImage(bmp, Position.x, Position.y, size.x, size.y);
            if (boundary)
            {
                drawBoundary(g, State, Position, bordercolor);
                if (Layer == 1)
                {
                    Utilities.drawDepthHandle(g, mAdvData, Position, Depth, Color.Blue);
                }
            }
        }
        public override bool isHit(Vec2i pos)
        {
            return isHit(State, pos - Position);
        }
        public override Vec2i getPosition()
        {
            return Position;
        }
        public override void setPosition(Vec2i pos)
        {
            Position = pos;
        }
        public override Vec2i getSize()
        {
            return mData.getSize(State);
        }
        public override bool isLocked()
        {
            return Locked;
        }
        public ObjectInstance duplicate(string oldRoomName, string roomName)
        {
            ObjectInstance inst = new ObjectInstance(Object, mAdvData);
            inst.Name = Name;
            inst.Position = Position;
            inst.State = State;
            inst.Layer = Layer;
            inst.Depth = Depth;
            inst.Locked = Locked;
            Script scr = mAdvData.getScript(Script.Type.OBJECT, Script.toScriptName(Name, oldRoomName));
            if (scr != null)
            {
                Script newscr = scr.duplicate(Script.toScriptName(Name, roomName));
                mAdvData.addScript(newscr);
            }
            return inst;
        }
        public string Name;
        public AdvObject Object;
        public Vec2i Position;
        public int State;
        public int Layer;
        public int Depth;
        public bool Locked;
        private AdvData mAdvData;
    }
}
