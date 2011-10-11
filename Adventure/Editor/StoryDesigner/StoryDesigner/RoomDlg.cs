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
        public RoomDlg(Room room, AdvData data)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
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
            if (mDragObject == null)
                return;
            Vec2i pos = new Vec2i(e.X, e.Y);
            mDragObject.setPosition(pos + mDragOffset);
            this.Invalidate();
        }

        void RoomDlg_MouseDown(object sender, MouseEventArgs e)
        {
            Vec2i pos = new Vec2i(e.X, e.Y);
            mDragObject = getObjectAt(pos);
            mControl.SelectedObject = mDragObject;
            if (mDragObject != null)
            {
                mDragOffset = mDragObject.getPosition() - pos;
            }
        }

        void RoomDlg_Paint(object sender, PaintEventArgs e)
        {
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
                pair.Value.draw(e.Graphics);
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

        private Room mRoom;
        private AdvData mData;
        private DrawableObject mDragObject;
        private Vec2i mDragOffset;
        private RoomCtrlDlg mControl;
    }
}