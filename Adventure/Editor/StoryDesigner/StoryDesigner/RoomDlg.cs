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
                int depth;
                if (obj.Layer == 0) //back
                    depth = 0;
                else if (obj.Layer == 1)
                    depth = obj.Depth;
                else
                    depth = 999;
                KeyValuePair<int, DrawableObject> pair = new KeyValuePair<int, DrawableObject>(depth, obj);
                insetQueue(blitqueue, pair);
            }
            foreach (CharacterInstance chr in mRoom.Characters)
            {
                int depth = chr.Position.y / mData.WalkGridSize;
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

        private Room mRoom;
        private AdvData mData;
    }
}