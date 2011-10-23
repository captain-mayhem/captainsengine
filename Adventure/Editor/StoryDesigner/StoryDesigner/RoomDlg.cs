using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class RoomDlg : Form
    {
        public enum ViewMode
        {
            Objects,
            Walkmap,
            Deepmap,
            Specialfx
        };

        public RoomDlg(Room room, AdvData data, ViewMode mode)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
            mMode = mode;
            this.Text = "Room (" + room.Name + ")";
            this.ClientSize = new Size(data.Settings.Resolution.x, data.Settings.Resolution.y);
            this.Paint += new PaintEventHandler(RoomDlg_Paint);
            this.MouseDown += new MouseEventHandler(RoomDlg_MouseDown);
            this.MouseMove += new MouseEventHandler(RoomDlg_MouseMove);
            this.MouseUp += new MouseEventHandler(RoomDlg_MouseUp);
            this.FormClosed += new FormClosedEventHandler(RoomDlg_FormClosed);
            mControl = new RoomCtrlDlg(room, data);
            mControl.Location = new Point(Screen.GetWorkingArea(this).Width-mControl.Width, 0);
            mControl.StartPosition = FormStartPosition.Manual;
            mControl.Show(this);
            mControl.RedrawRoom += new RoomCtrlDlg.RedrawEventHandler(mControl_RedrawRoom);
        }

        void mControl_RedrawRoom(object sender, RoomCtrlDlg.RedrawEventArgs e)
        {
            this.Invalidate();
        }

        void RoomDlg_FormClosed(object sender, FormClosedEventArgs e)
        {
            mControl.Dispose();
        }

        void RoomDlg_MouseUp(object sender, MouseEventArgs e)
        {
            mDragObject = null;
        }

        void RoomDlg_MouseMove(object sender, MouseEventArgs e)
        {
            mMousePos.x = e.X;
            mMousePos.y = e.Y;
            Invalidate();
            if (mDragObject == null)
                return;
            if (mDragObject.isLocked() && !mDragDepth)
                return;
            Vec2i pos = new Vec2i(e.X, e.Y);
            if (mDragDepth)
            {
                ObjectInstance obj = (ObjectInstance)mDragObject;
                obj.Depth = (e.Y + 5 + mDragOffset.y + mData.WalkGridSize / 2) / mData.WalkGridSize;
                if (obj.Depth < 1)
                    obj.Depth = 1;
                if (obj.Depth > mRoom.Size.y / mData.WalkGridSize)
                    obj.Depth = mRoom.Size.y / mData.WalkGridSize;
            }
            else
            {
                mDragObject.setPosition(pos + mDragOffset);
            }
            //this.Invalidate();
        }

        void RoomDlg_MouseDown(object sender, MouseEventArgs e)
        {
            Vec2i pos = new Vec2i(e.X, e.Y);
            foreach (ObjectInstance obj in mRoom.Objects)
            {
                if (obj.Layer != 1)
                    continue;
                Vec2i depthcenter = new Vec2i(obj.Position.x, obj.Depth * mData.WalkGridSize - mData.WalkGridSize / 2);
                if ((depthcenter - pos-mRoom.ScrollOffset).length() <= 5)
                {
                    mDragObject = obj;
                    mDragOffset = depthcenter - pos;
                    mDragDepth = true;
                    return;
                }
            }
            mDragDepth = false;
            mDragObject = getObjectAt(pos+mRoom.ScrollOffset);
            mControl.SelectedObject = mDragObject;
            if (mDragObject != null)
            {
                mDragOffset = mDragObject.getPosition() - pos;
            }
        }

        void RoomDlg_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.TranslateTransform(-mRoom.ScrollOffset.x, -mRoom.ScrollOffset.y);
            if (mRoom.ParallaxBackground.Length > 0)
            {
                Bitmap bmp = mData.getImage(mRoom.ParallaxBackground);
                e.Graphics.DrawImage(bmp, 0, 0, bmp.Width, bmp.Height);
            }
            if (mRoom.Background.Length > 0)
            {
                Bitmap bmp = mData.getImage(mRoom.Background);
                e.Graphics.DrawImage(bmp, 0, 0, bmp.Width, bmp.Height);
            }

            System.Collections.Generic.LinkedList<System.Collections.Generic.KeyValuePair<int, DrawableObject> > blitqueue = new LinkedList<KeyValuePair<int,DrawableObject>>();
            foreach (ObjectInstance obj in mRoom.Objects)
            {
                int depth = getDepth(obj);
                KeyValuePair<int, DrawableObject> pair = new KeyValuePair<int, DrawableObject>(depth, obj);
                insetQueue(blitqueue, pair);
            }
            foreach (CharacterInstance chr in mRoom.Characters)
            {
                int depth = getDepth(chr);
                KeyValuePair<int, DrawableObject> pair = new KeyValuePair<int, DrawableObject>(depth, chr);
                insetQueue(blitqueue, pair);
            }

            foreach (System.Collections.Generic.KeyValuePair<int,DrawableObject> pair in blitqueue)
            {
                pair.Value.draw(e.Graphics, mMode == ViewMode.Objects);
            }

            if (mMode == ViewMode.Walkmap)
            {
                float scale = mRoom.DoubleWalkmap ? 0.5f : 1;
                SolidBrush b = new SolidBrush(Color.FromArgb(100, Color.Blue));
                for (int x = 0; x < mRoom.Walkmap.GetLength(0); ++x)
                {
                    for (int y = 0; y < mRoom.Walkmap.GetLength(1); ++y)
                    {
                        if (!mRoom.Walkmap[x, y].isFree)
                        {
                            e.Graphics.FillRectangle(b, x * mData.WalkGridSize * scale, y * mData.WalkGridSize * scale,
                                mData.WalkGridSize * scale, mData.WalkGridSize * scale);
                        }
                        if (mRoom.Walkmap[x, y].hasScript)
                        {
                            e.Graphics.DrawRectangle(Pens.Black, (x * mData.WalkGridSize + 4) * scale,
                                (y * mData.WalkGridSize + 4) * scale,
                                (mData.WalkGridSize - 8) * scale, (mData.WalkGridSize - 8) * scale);
                            e.Graphics.DrawRectangle(Pens.Yellow, (x * mData.WalkGridSize + 4) * scale + 1,
                                (y * mData.WalkGridSize + 4) * scale + 1,
                                (mData.WalkGridSize - 8) * scale, (mData.WalkGridSize - 8) * scale);
                        }
                    }
                }
                //draw grid
                for (float i = mData.WalkGridSize*scale; i < mRoom.Size.x; i += mData.WalkGridSize*scale)
                {
                    e.Graphics.DrawLine(Pens.Brown, i, 0, i, mRoom.Size.y);
                }
                for (float i = mData.WalkGridSize*scale; i < mRoom.Size.y; i += mData.WalkGridSize*scale)
                {
                    e.Graphics.DrawLine(Pens.Brown, 0, i, mRoom.Size.x, i);
                }
            }

            //draw information

            if (mMode == ViewMode.Objects){
                Vec2i mp = mMousePos + mRoom.ScrollOffset;
                int scale = mRoom.DoubleWalkmap ? 2 : 1;
                int wmx = scale*mp.x/mData.WalkGridSize;
                int wmy = scale*mp.y/mData.WalkGridSize;
                bool wmfree = mRoom.Walkmap[wmx, wmy].isFree;
                string s = String.Format("Position: Room({0}/{1}) Walkmap({2}/{3}){4} Mouse({5}/{6})", mRoom.ScrollOffset.x/mData.WalkGridSize, mRoom.ScrollOffset.y/mData.WalkGridSize, wmx+1, wmy+1, wmfree ? 'F' : 'B', mp.x, mp.y);
                Font f = new Font(Fonts.DefaultFont.FontFamily, 11);
                e.Graphics.DrawString(s, f, Brushes.Gray, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y);
                string s2 = "Object:";
                DrawableObject drob = getObjectAt(mMousePos+mRoom.ScrollOffset);
                if (drob is ObjectInstance)
                {
                    ObjectInstance obj = (ObjectInstance)drob;
                    s2 = String.Format("Object: {0} (PixelPos: {1},{2})", obj.Name, obj.Position.x, obj.Position.y);
                }
                else if (drob is CharacterInstance)
                {
                    CharacterInstance chr = (CharacterInstance)drob;
                    s2 = String.Format("Object: {0} (WalkmapPos: {1},{2})", chr.Name, scale*chr.Position.x/mData.WalkGridSize+1, scale*chr.Position.y/mData.WalkGridSize+1);
                }
                e.Graphics.DrawString(s2, f, Brushes.Gray, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y+f.Height);
            }
        }

        public Room Room
        {
            get { return mRoom; }
        }

        private static void insetQueue(System.Collections.Generic.LinkedList<System.Collections.Generic.KeyValuePair<int, DrawableObject>> blitqueue, KeyValuePair<int, DrawableObject> pair)
        {
            LinkedListNode<System.Collections.Generic.KeyValuePair<int, DrawableObject>> iter = blitqueue.First;
            bool found = false;
            while (iter != null)
            {
                if (pair.Key <= iter.Value.Key)
                {
                    blitqueue.AddBefore(iter, pair);
                    found = true;
                    break;
                }
                iter = iter.Next;
            }
            if (!found)
                blitqueue.AddLast(pair);
        }

        private DrawableObject getObjectAt(Vec2i pos)
        {
            DrawableObject ret = null;
            int retdepth = -1;
            foreach (ObjectInstance obj in mRoom.Objects)
            {
                if (obj.isHit(pos))
                {
                    int depth = getDepth(obj);
                    if (depth > retdepth)
                    {
                        retdepth = depth;
                        ret = obj;
                    }
                }
            }
            foreach (CharacterInstance chr in mRoom.Characters)
            {
                if (chr.isHit(pos))
                {
                    int depth = getDepth(chr);
                    if (depth > retdepth)
                    {
                        retdepth = depth;
                        ret = chr;
                    }
                }
            }
            return ret;
        }

        private int getDepth(ObjectInstance obj)
        {
            int depth;
            if (obj.Layer == 0) //back
                depth = 0;
            else if (obj.Layer == 1)
                depth = obj.Depth;
            else
                depth = 999;
            return depth;
        }

        private int getDepth(CharacterInstance chr)
        {
            return chr.Position.y / mData.WalkGridSize;
        }

        public ViewMode Viewmode
        {
            set { mMode = value; Invalidate(); }
        }

        private Room mRoom;
        private AdvData mData;
        private DrawableObject mDragObject;
        private Vec2i mDragOffset;
        private bool mDragDepth;
        private RoomCtrlDlg mControl;
        private Vec2i mMousePos;
        private ViewMode mMode;
    }
}