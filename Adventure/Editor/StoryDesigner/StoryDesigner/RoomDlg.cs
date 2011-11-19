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
            Objects = 0,
            Walkmap,
            Deepmap,
            Inventory,
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
            this.DoubleClick += new EventHandler(RoomDlg_DoubleClick);
            this.KeyPress += new KeyPressEventHandler(RoomDlg_KeyPress);
            this.FormClosed += new FormClosedEventHandler(RoomDlg_FormClosed);
            this.DragOver += new DragEventHandler(RoomDlg_DragOver);
            this.DragDrop += new DragEventHandler(RoomDlg_DragDrop);
            mControl = new RoomCtrlDlg(room, data);
            mControl.Location = new Point(Screen.GetWorkingArea(this).Width-mControl.Width, 0);
            mControl.StartPosition = FormStartPosition.Manual;
            mControl.Show(this);
            mControl.RedrawRoom += new RoomCtrlDlg.RedrawEventHandler(mControl_RedrawRoom);
        }

        void RoomDlg_KeyPress(object sender, KeyPressEventArgs e)
        {
            MainForm form = (MainForm)this.Owner;
            if (e.KeyChar >= '1' && e.KeyChar <= '5')
            {
                int num = Convert.ToInt32(e.KeyChar+"");
                form.setRoomViewMode((ViewMode)(num - 1));
            }
        }

        void RoomDlg_DragDrop(object sender, DragEventArgs e)
        {
            AdvObject obj = (AdvObject)e.Data.GetData(typeof(AdvObject));
            if (obj != null)
            {
                ObjectInstance inst = new ObjectInstance(obj, mData);
                Point p = PointToClient(new Point(e.X, e.Y));
                p.X += mRoom.ScrollOffset.x;
                p.Y += mRoom.ScrollOffset.y;
                inst.Name = obj.Name;
                int count = 0;
                foreach (ObjectInstance objiter in mRoom.Objects)
                {
                    if (objiter.Object == obj)
                        ++count;
                }
                if (count != 0)
                    inst.Name += count;
                inst.Depth = p.Y / mData.WalkGridSize + 1;
                inst.Layer = 2;
                inst.Object = obj;
                inst.State = 1;
                inst.Locked = false;
                inst.setPosition(new Vec2i(p.X, p.Y));
                mRoom.Objects.Add(inst);
                return;
            }
            AdvCharacter chr = (AdvCharacter)e.Data.GetData(typeof(AdvCharacter));
            if (chr != null)
            {
                CharacterInstance inst = new CharacterInstance(chr, mData);
                Point p = PointToClient(new Point(e.X, e.Y));
                inst.Name = chr.Name;
                int count = 0;
                foreach (KeyValuePair<string, System.Collections.ArrayList> kvp in mData.CharacterInstances)
                {
                    foreach (CharacterInstance ci in kvp.Value)
                    {
                        if (ci.Character == chr)
                            ++count;
                    }
                }
                if (count > 0)
                    inst.Name += count;
                inst.Character = chr;
                inst.Locked = false;
                inst.Unmovable = false;
                inst.LookDir = 1;
                inst.Room = mRoom.Name;
                inst.RawPosition = new Vec2i(p.X, p.Y)+mRoom.ScrollOffset;
                if (!mData.CharacterInstances.ContainsKey(mRoom.Name.ToLower()))
                    mData.CharacterInstances.Add(mRoom.Name.ToLower(), new System.Collections.ArrayList());
                mData.CharacterInstances[mRoom.Name.ToLower()].Add(inst);
                mRoom.Characters.Add(inst);
                return;
            }
            string image = (string)e.Data.GetData(DataFormats.StringFormat);
            if (image != null)
            {
                mPendingImage = image;
                menuAddBackground.Show(e.X, e.Y);
                return;
            }
        }

        void RoomDlg_DragOver(object sender, DragEventArgs e)
        {
            if (mMode != ViewMode.Objects)
                e.Effect = DragDropEffects.None;
            else
            {
                e.Effect = DragDropEffects.Copy;
            }
        }

        void RoomDlg_DoubleClick(object sender, EventArgs e)
        {
            if (mMode != ViewMode.Walkmap)
                return;
            Vec2i mp = mMousePos + mRoom.ScrollOffset;
            int wmscale = mRoom.DoubleWalkmap ? 2 : 1;
            int wmx = wmscale * mp.x / mData.WalkGridSize;
            int wmy = wmscale * mp.y / mData.WalkGridSize;
            mRoom.Walkmap[wmx, wmy].hasScript = true;
            MainForm form = (MainForm)this.Owner;
            string scrname = Script.toScriptName(wmx, wmy, mRoom.Name);
            form.showScript(Script.Type.WALKMAP, scrname.ToLower());
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
            mDragDepth = false;
        }

        void RoomDlg_MouseMove(object sender, MouseEventArgs e)
        {
            mMousePos.x = e.X;
            mMousePos.y = e.Y;
            Invalidate();
            if (mMode == ViewMode.Objects)
            {
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
            }
            else if (mMode == ViewMode.Walkmap)
            {
                if (!mDragDepth)
                    return;
                modifyWalkmap(e.X, e.Y);
            }
        }

        void RoomDlg_MouseDown(object sender, MouseEventArgs e)
        {
            Point click = new Point(e.X, e.Y);
            Vec2i pos = new Vec2i(e.X, e.Y);
            mMousePos = pos;
            if (mMode == ViewMode.Objects)
            {
                if (e.Button == MouseButtons.Right)
                {
                    menuRemoveBackground.Show(this, click);
                }
                foreach (ObjectInstance obj in mRoom.Objects)
                {
                    if (obj.Layer != 1)
                        continue;
                    Vec2i depthcenter = new Vec2i(obj.Position.x, obj.Depth * mData.WalkGridSize - mData.WalkGridSize / 2);
                    if ((depthcenter - pos - mRoom.ScrollOffset).length() <= 5)
                    {
                        mDragObject = obj;
                        mDragOffset = depthcenter - pos;
                        mDragDepth = true;
                        return;
                    }
                }
                mDragDepth = false;
                mDragObject = getObjectAt(pos + mRoom.ScrollOffset);
                mControl.SelectedObject = mDragObject;
                if (mDragObject != null)
                {
                    mDragOffset = mDragObject.getPosition() - pos;
                }
            }
            else if (mMode == ViewMode.Walkmap)
            {
                if (e.Button == MouseButtons.Right)
                {
                    menuWalkmap.Show(this, click);
                    return;
                }
                modifyWalkmap(pos.x, pos.y);
                mDragDepth = true;
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

            //draw view specific stuff
            Vec2i mp = mMousePos + mRoom.ScrollOffset;
            int wmscale = mRoom.DoubleWalkmap ? 2 : 1;
            int wmx = wmscale * mp.x / mData.WalkGridSize;
            int wmy = wmscale * mp.y / mData.WalkGridSize;
            bool wmfree = mRoom.Walkmap[wmx, wmy].isFree;
            if (mMode == ViewMode.Walkmap)
            {
                float scale = mRoom.DoubleWalkmap ? 0.5f : 1;
                SolidBrush b = new SolidBrush(Color.FromArgb(100, Color.Blue));
                for (int x = 0; x <= mRoom.Walkmap.GetUpperBound(0); ++x)
                {
                    for (int y = 0; y <= mRoom.Walkmap.GetUpperBound(1); ++y)
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
                //draw cursor
                if (mWalkmapPaintMode >= 0 && mWalkmapPaintMode < 3)
                {
                    int size = 2 * mWalkmapPaintMode + 1;
                    e.Graphics.DrawEllipse(Pens.Black, ((wmx-mWalkmapPaintMode) * mData.WalkGridSize + 2) * scale,
                                    ((wmy-mWalkmapPaintMode) * mData.WalkGridSize + 2) * scale,
                                    (size * mData.WalkGridSize - 4) * scale, (size * mData.WalkGridSize - 4) * scale);
                    e.Graphics.DrawEllipse(Pens.LightGray, ((wmx-mWalkmapPaintMode) * mData.WalkGridSize + 1) * scale,
                                    ((wmy-mWalkmapPaintMode) * mData.WalkGridSize + 1) * scale,
                                    (size * mData.WalkGridSize - 2) * scale, (size * mData.WalkGridSize - 2) * scale);
                }
            }

            //draw information
            Font f = new Font(Fonts.DefaultFont.FontFamily, 11);
            if (mMode == ViewMode.Objects){
                string s = String.Format("Position: Room({0}/{1}) Walkmap({2}/{3}){4} Mouse({5}/{6})", mRoom.ScrollOffset.x/mData.WalkGridSize, mRoom.ScrollOffset.y/mData.WalkGridSize, wmx+1, wmy+1, wmfree ? 'F' : 'B', mp.x, mp.y);
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
                    s2 = String.Format("Object: {0} (WalkmapPos: {1},{2})", chr.Name, wmscale*chr.Position.x/mData.WalkGridSize+1, wmscale*chr.Position.y/mData.WalkGridSize+1);
                }
                e.Graphics.DrawString(s2, f, Brushes.Gray, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y+f.Height);
            }
            else if (mMode == ViewMode.Walkmap)
            {
                string s = String.Format("{0} ({1},{2})", wmfree ? "Free" : "Blocked", wmx + 1, wmy + 1);
                e.Graphics.DrawString(s, f, Brushes.Gray, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y);
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

        void modifyWalkmap(int x, int y)
        {
            x += mRoom.ScrollOffset.x;
            y += mRoom.ScrollOffset.y;
            int wmscale = mRoom.DoubleWalkmap ? 2 : 1;
            int wmx = wmscale * x / mData.WalkGridSize;
            int wmy = wmscale * y / mData.WalkGridSize;
            ToolStripMenuItem it = menuWalkmap.Items[0] as ToolStripMenuItem;
            mRoom.Walkmap[wmx, wmy].isFree = it.Checked;
            if (mWalkmapPaintMode > 0)
            {
                setWalkmapPoint(wmx + 1, wmy, it.Checked);
                setWalkmapPoint(wmx - 1, wmy, it.Checked);
                setWalkmapPoint(wmx, wmy + 1, it.Checked);
                setWalkmapPoint(wmx, wmy - 1, it.Checked);
            }
            if (mWalkmapPaintMode > 1)
            {
                setWalkmapPoint(wmx + 1, wmy - 1, it.Checked);
                setWalkmapPoint(wmx - 1, wmy + 1, it.Checked);
                setWalkmapPoint(wmx - 1, wmy - 1, it.Checked);
                setWalkmapPoint(wmx + 1, wmy + 1, it.Checked);
                setWalkmapPoint(wmx + 2, wmy, it.Checked);
                setWalkmapPoint(wmx - 2, wmy, it.Checked);
                setWalkmapPoint(wmx, wmy + 2, it.Checked);
                setWalkmapPoint(wmx, wmy - 2, it.Checked);
            }
        }

        void setWalkmapPoint(int x, int y, bool value)
        {
            if (x < mRoom.Walkmap.GetLowerBound(0) || x > mRoom.Walkmap.GetUpperBound(0))
                return;
            if (y < mRoom.Walkmap.GetLowerBound(1) || y > mRoom.Walkmap.GetUpperBound(1))
                return;
            mRoom.Walkmap[x, y].isFree = value;
        }

        private Room mRoom;
        private AdvData mData;
        private DrawableObject mDragObject;
        private Vec2i mDragOffset;
        private bool mDragDepth;
        private RoomCtrlDlg mControl;
        private Vec2i mMousePos;
        private ViewMode mMode;
        private string mPendingImage;
        private int mWalkmapPaintMode = 0;

        private void addAsBackgroundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mRoom.Background = mPendingImage;
        }

        private void addAsPaToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mRoom.ParallaxBackground = mPendingImage;
        }

        private void removeBackgroundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mRoom.Background = "";
        }

        private void removeParallaxBackgroundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mRoom.ParallaxBackground = "";
        }

        private void freeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem free = menuWalkmap.Items[0] as ToolStripMenuItem;
            ToolStripMenuItem blocked = menuWalkmap.Items[1] as ToolStripMenuItem;
            free.Checked = true;
            blocked.Checked = false;
        }

        private void blockToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem free = menuWalkmap.Items[0] as ToolStripMenuItem;
            ToolStripMenuItem blocked = menuWalkmap.Items[1] as ToolStripMenuItem;
            free.Checked = false;
            blocked.Checked = true;
        }

        private void walkmapMenuSelect(int select)
        {
            for (int i = 0; i < 5; ++i)
            {
                ToolStripMenuItem item = menuWalkmap.Items[i + 3] as ToolStripMenuItem;
                item.Checked = i == select;
            }
            mWalkmapPaintMode = select;
        }

        private void penSize1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            walkmapMenuSelect(0);
        }

        private void penSize2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            walkmapMenuSelect(1);
        }

        private void penSize3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            walkmapMenuSelect(2);
        }

        private void scriptPasterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            walkmapMenuSelect(3);
        }

        private void scriptEraserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            walkmapMenuSelect(4);
        }

        private void editScriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Vec2i pos = new Vec2i(mMousePos.x /mData.WalkGridSize, mMousePos.y / mData.WalkGridSize);
            mRoom.Walkmap[pos.x, pos.y].hasScript = true;
            MainForm form = (MainForm)this.Owner;
            string scrname = Script.toScriptName(pos.x, pos.y, mRoom.Name);
            form.showScript(Script.Type.WALKMAP, scrname.ToLower());
        }
    }
}