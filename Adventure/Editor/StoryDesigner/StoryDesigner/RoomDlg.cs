using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;

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

        enum DragMode
        {
            DragNone,
            DragBottom,
            DragRight,
            DragCorner,
            DragPosition
        };

        public RoomDlg(Room room, AdvData data, ViewMode mode, MainForm parent, Form mdiParent)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
            mMode = mode;
            updateRoom();
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
            mControl.MdiParent = mdiParent;
            mControl.Location = new Point(Screen.GetWorkingArea(this).Width-mControl.Width, 0);
            mControl.StartPosition = FormStartPosition.Manual;
            mControl.Show();
            mControl.RedrawRoom += new RoomCtrlDlg.RedrawEventHandler(mControl_RedrawRoom);
            mTimer = new Timer();
            mTimer.Tick += new EventHandler(mTimer_Tick);
            mTimer.Interval = 50;
            mTimer.Start();
            mOwner = parent;
        }

        void mTimer_Tick(object sender, EventArgs e)
        {
            ++mFramecount;
            if (mFramecount >= 10)
                mFramecount = 0;
            this.Invalidate();
        }

        internal void RoomDlg_KeyPress(object sender, KeyPressEventArgs e)
        {
            MainForm form = mOwner;
            if (e.KeyChar >= '1' && e.KeyChar <= '5')
            {
                int num = Convert.ToInt32(e.KeyChar+"");
                form.setRoomViewMode((ViewMode)(num - 1));
            }
        }

        void RoomDlg_DragDrop(object sender, DragEventArgs e)
        {
            Resource res = (Resource)e.Data.GetData(typeof(Resource));
            if (res.ID == ResourceID.OBJECT)
            {
                AdvObject obj = mData.getObject(res.Name);
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
                mControl.SelectedObject = inst;
                return;
            }
            else if (res.ID == ResourceID.CHARACTER)
            {
                AdvCharacter chr = mData.getCharacter(res.Name);
                CharacterInstance inst = new CharacterInstance(chr, mData);
                Point p = PointToClient(new Point(e.X, e.Y));
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
                mControl.SelectedObject = inst;
                return;
            }
            else if (res.ID == ResourceID.IMAGE)
            {
                string image = res.Name;
                mPendingImage = image;
                menuAddBackground.Show(e.X, e.Y);
                return;
            }
        }

        void RoomDlg_DragOver(object sender, DragEventArgs e)
        {
            if (mMode != ViewMode.Objects || !e.Data.GetDataPresent(typeof(Resource)))
                e.Effect = DragDropEffects.None;
            else
            {
                Resource res = (Resource)e.Data.GetData(typeof(Resource));
                if (res.ID == ResourceID.IMAGE || res.ID == ResourceID.CHARACTER || res.ID == ResourceID.OBJECT)
                    e.Effect = DragDropEffects.Copy;
                else
                    e.Effect = DragDropEffects.None;
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
            MainForm form = mOwner;
            string scrname = Script.toScriptName(wmx, wmy, mRoom.Name, mData);
            form.showScript(Script.Type.WALKMAP, scrname);
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
            if (mMode == ViewMode.Deepmap)
                mDragDepthBottom = false;
            if (mMode == ViewMode.Inventory)
            {
                mDragMode = DragMode.DragNone;
                this.Cursor = Cursors.Default;
            }
            if (mMode == ViewMode.Specialfx)
            {
                mDragMode = DragMode.DragNone;
            }
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
            else if (mMode == ViewMode.Deepmap)
            {
                int depth = (e.Y + mData.WalkGridSize / 2 + mRoom.ScrollOffset.y) / mData.WalkGridSize;
                if (mDragDepth)
                {
                    if (depth < 1)
                        depth = 1;
                    if (depth >= mRoom.Depthmap.y)
                        depth = mRoom.Depthmap.y - 1;
                    mRoom.Depthmap.x = depth;
                }
                if (mDragDepthBottom)
                {
                    if (depth <= mRoom.Depthmap.x)
                        depth = mRoom.Depthmap.x + 1;
                    if (depth >= mRoom.Size.y/mData.WalkGridSize)
                        depth = mRoom.Size.y/mData.WalkGridSize-1;
                    mRoom.Depthmap.y = depth;
                }
            }
            else if (mMode == ViewMode.Inventory)
            {
                if (mDragMode == DragMode.DragNone)
                {
                    int sizex = (int)((mRoom.InvSize.x * 61) * mRoom.InvScale.x) + 30;
                    int sizey = (int)((mRoom.InvSize.y * 59) * mRoom.InvScale.y) + 5;
                    int ydown = -mRoom.ScrollOffset.y + mRoom.InvPos.y + sizey;
                    int xmiddle = -mRoom.ScrollOffset.x + mRoom.InvPos.x + sizex / 2;
                    //e.Graphics.DrawLine(tbp, xmiddle - 15, ydown - 3, xmiddle + 15, ydown - 3);
                    int ymiddle = -mRoom.ScrollOffset.y + mRoom.InvPos.y + sizey / 2;
                    int xright = -mRoom.ScrollOffset.x + mRoom.InvPos.x + sizex;
                    //bottom
                    if (e.X >= xmiddle - 15 && e.X <= xmiddle + 15)
                    {
                        if (e.Y <= ydown + 3 && e.Y >= ydown - 5 - 3)
                            this.Cursor = Cursors.SizeNS;
                        else
                            this.Cursor = Cursors.Default;
                    }
                    //right
                    else if (e.Y >= ymiddle - 15 && e.Y <= ymiddle + 15)
                    {
                        if (e.X <= xright + 3 && e.X >= xright - 5 - 3)
                            this.Cursor = Cursors.SizeWE;
                        else
                            this.Cursor = Cursors.Default;
                    }
                    //corner
                    else if (e.X >= xright - 11 - 3 && e.X <= xright + 3)
                    {
                        if (e.Y >= ydown - 11 - 3 && e.Y <= ydown + 3)
                            this.Cursor = Cursors.SizeNWSE;
                        else
                            this.Cursor = Cursors.Default;
                    }
                    else
                        this.Cursor = Cursors.Default;
                }
                else if (mDragMode == DragMode.DragCorner)
                {
                    int maxX = (int)((mData.Settings.Resolution.x - 30) / 60);
                    int maxY = (int)((mData.Settings.Resolution.y - 5) / 60);
                    mRoom.InvSize.x = (int)Math.Round((e.X + mRoom.ScrollOffset.x - mRoom.InvPos.x - 30) / (mRoom.InvScale.x * 61));
                    if (mRoom.InvSize.x < 1)
                        mRoom.InvSize.x = 1;
                    if (mRoom.InvSize.x > maxX)
                        mRoom.InvSize.x = maxX;
                    mRoom.InvSize.y = (int)Math.Round((e.Y + mRoom.ScrollOffset.y - mRoom.InvPos.y - 5) / (mRoom.InvScale.y * 59));
                    if (mRoom.InvSize.y < 1)
                        mRoom.InvSize.y = 1;
                    if (mRoom.InvSize.y > maxY)
                        mRoom.InvSize.y = maxY;
                }
                else if (mDragMode == DragMode.DragPosition)
                {
                    mRoom.InvPos = new Vec2i(e.X, e.Y) - mDragOffset;
                    if (mRoom.InvPos.x < 0)
                        mRoom.InvPos.x = 0;
                    if (mRoom.InvPos.y < 0)
                        mRoom.InvPos.y = 0;
                }
                else if (mDragMode == DragMode.DragBottom)
                {
                    float newsize = (e.Y - mDragOffset.y - mRoom.InvPos.y + mRoom.ScrollOffset.y);
                    float oldsize = (mRoom.InvSize.y * 59) + 5;
                    mRoom.InvScale.y = newsize / oldsize;
                    if (mRoom.InvScale.y > 1.25)
                        mRoom.InvScale.y = 1.25f;
                    if (mRoom.InvScale.y < 0.65)
                        mRoom.InvScale.y = 0.65f;
                }
                else if (mDragMode == DragMode.DragRight)
                {
                    float newsize = (e.X - mDragOffset.x - mRoom.InvPos.x + mRoom.ScrollOffset.x);
                    float oldsize = (mRoom.InvSize.x * 61) + 30;
                    mRoom.InvScale.x = newsize / oldsize;
                    if (mRoom.InvScale.x > 1.25)
                        mRoom.InvScale.x = 1.25f;
                    if (mRoom.InvScale.x < 0.65)
                        mRoom.InvScale.x = 0.65f;
                }
            }
            else if (mMode == ViewMode.Specialfx)
            {
                if (mDragMode == DragMode.DragCorner)
                {
                    mDraggingShape.Positions[mShapeIndex] = mMousePos + mRoom.ScrollOffset;
                    if (mDraggingShape.Positions[mShapeIndex].x < 0)
                        mDraggingShape.Positions[mShapeIndex].x = 0;
                    if (mDraggingShape.Positions[mShapeIndex].x > mRoom.Size.x)
                        mDraggingShape.Positions[mShapeIndex].x = mRoom.Size.x;
                    if (mDraggingShape.Positions[mShapeIndex].y < 0)
                        mDraggingShape.Positions[mShapeIndex].y = 0;
                    if (mDraggingShape.Positions[mShapeIndex].y > mRoom.Size.y)
                        mDraggingShape.Positions[mShapeIndex].y = mRoom.Size.y;
                }
                else if (mDragMode == DragMode.DragPosition)
                {
                    Vec2i diff = mMousePos - mDragOffset;
                    for (int i = 0; i < mDraggingShape.Positions.Length; ++i)
                    {
                        mDraggingShape.Positions[i] += diff;
                        if (mDraggingShape.Positions[i].x < 0)
                            correctShapeBoundary(i, new Vec2i(-mDraggingShape.Positions[i].x, 0), ref diff);
                        if (mDraggingShape.Positions[i].x > mRoom.Size.x)
                            correctShapeBoundary(i, new Vec2i(mRoom.Size.x - mDraggingShape.Positions[i].x, 0), ref diff);
                        if (mDraggingShape.Positions[i].y < 0)
                            correctShapeBoundary(i, new Vec2i(0, -mDraggingShape.Positions[i].y), ref diff);
                        if (mDraggingShape.Positions[i].y > mRoom.Size.y)
                            correctShapeBoundary(i, new Vec2i(0, mRoom.Size.y - mDraggingShape.Positions[i].y), ref diff);
                    }
                    mDragOffset = mMousePos;
                }
                else if (mDragMode == DragMode.DragBottom)
                {
                    mDraggingShape.Depth = (e.Y + 5 + mDragOffset.y + mData.WalkGridSize / 2) / mData.WalkGridSize;
                    if (mDraggingShape.Depth < 1)
                        mDraggingShape.Depth = 1;
                    if (mDraggingShape.Depth > mRoom.Size.y / mData.WalkGridSize)
                        mDraggingShape.Depth = mRoom.Size.y / mData.WalkGridSize;
                }
            }
        }

        void RoomDlg_MouseDown(object sender, MouseEventArgs e)
        {
            Point click = new Point(e.X, e.Y);
            Vec2i pos = new Vec2i(e.X, e.Y);
            mMousePos = pos;
            if (mMode == ViewMode.Objects)
            {
                bool dragDepthFound = false;
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
                        dragDepthFound = true;
                        break;
                    }
                }
                if (!dragDepthFound)
                {
                    mDragDepth = false;
                    bool dragSecond = e.Button == MouseButtons.Middle;
                    mDragObject = getObjectAt(pos + mRoom.ScrollOffset, dragSecond);
                    mControl.SelectedObject = mDragObject;
                    if (mDragObject != null)
                    {
                        mDragOffset = mDragObject.getPosition() - pos;
                    }
                }
                if (e.Button == MouseButtons.Right)
                {
                    if (mDragObject != null)
                    {
                        if (mDragObject.GetType() == typeof(ObjectInstance))
                            menuObject.Items[2].Text = "Show in Object window";
                        else
                            menuObject.Items[2].Text = "Show in Character window";
                        menuObject.Show(this, click);
                    }
                    else
                        menuRemoveBackground.Show(this, click);
                }
            }
            else if (mMode == ViewMode.Walkmap)
            {
                if (e.Button == MouseButtons.Right)
                {
                    Vec2i cp = clickToWalkmap(e.X, e.Y);
                    ToolStripMenuItem it = menuWalkmap.Items[9] as ToolStripMenuItem;
                    if (mRoom.Walkmap[cp.x, cp.y].hasScript)
                    {
                        it.Enabled = true;
                    }
                    else
                        it.Enabled = false;
                    menuWalkmap.Show(this, click);
                    return;
                }
                modifyWalkmap(pos.x, pos.y);
                mDragDepth = true;
            }
            else if (mMode == ViewMode.Deepmap)
            {
                int depth = (e.Y+mData.WalkGridSize/2+mRoom.ScrollOffset.y) / mData.WalkGridSize;
                if (depth == mRoom.Depthmap.x)
                    mDragDepth = true;
                if (depth == mRoom.Depthmap.y)
                    mDragDepthBottom = true;
            }
            else if (mMode == ViewMode.Inventory)
            {
                if (mRoom.HasInventory)
                {
                    if (e.Button == MouseButtons.Right)
                    {
                        menuInventory.Show(this, click);
                    }
                    int sizex = (int)((mRoom.InvSize.x * 61) * mRoom.InvScale.x) + 30;
                    int sizey = (int)((mRoom.InvSize.y * 59) * mRoom.InvScale.y) + 5;
                    int ydown = -mRoom.ScrollOffset.y + mRoom.InvPos.y + sizey;
                    int xmiddle = -mRoom.ScrollOffset.x + mRoom.InvPos.x + sizex / 2;
                    int ymiddle = -mRoom.ScrollOffset.y + mRoom.InvPos.y + sizey / 2;
                    int xright = -mRoom.ScrollOffset.x + mRoom.InvPos.x + sizex;
                    int xleft = -mRoom.ScrollOffset.x + mRoom.InvPos.x;
                    int yup = -mRoom.ScrollOffset.y + mRoom.InvPos.y;
                    //bottom
                    if (e.X >= xmiddle - 15 && e.X <= xmiddle + 15)
                    {
                        if (e.Y <= ydown + 3 && e.Y >= ydown - 5 - 3)
                        {
                            mDragMode = DragMode.DragBottom;
                            mDragOffset.y = e.Y - ydown;
                        }
                        else
                            mDragMode = DragMode.DragNone;
                    }
                    //right
                    else if (e.Y >= ymiddle - 15 && e.Y <= ymiddle + 15)
                    {
                        if (e.X <= xright + 3 && e.X >= xright - 5 - 3)
                        {
                            mDragMode = DragMode.DragRight;
                            mDragOffset.x = e.X - xright;
                        }
                        else
                            mDragMode = DragMode.DragNone;
                    }
                    //corner
                    else if (e.X >= xright - 11 - 3 && e.X <= xright + 3)
                    {
                        if (e.Y >= ydown - 11 - 3 && e.Y <= ydown + 3)
                            mDragMode = DragMode.DragCorner;
                        else
                            mDragMode = DragMode.DragNone;
                    }
                    else
                        mDragMode = DragMode.DragNone;
                    //elsewhere on inventory
                    if (mDragMode == DragMode.DragNone && e.X >= xleft && e.X <= xright)
                    {
                        if (e.Y >= yup && e.Y <= ydown)
                        {
                            mDragMode = DragMode.DragPosition;
                            mDragOffset = new Vec2i(e.X, e.Y) - mRoom.InvPos;
                        }
                    }
                }
                else
                {
                    if (e.Button == MouseButtons.Right)
                    {
                        return;
                    }
                    mRoom.InvPos = new Vec2i(e.X + mRoom.ScrollOffset.x, e.Y + mRoom.ScrollOffset.y);
                    mRoom.InvScale = new Vec2f(1.0f, 1.0f);
                    mRoom.HasInventory = true;
                    mDragMode = DragMode.DragCorner;
                    this.Cursor = Cursors.SizeNWSE;
                }
            }
            else if (mMode == ViewMode.Specialfx)
            {
                Vec2i clickpos = pos;
                clickpos += mRoom.ScrollOffset;
                for (int shapeidx = 0; shapeidx < mRoom.FXShapes.Count; ++shapeidx)
                {
                    FxShape shape = (FxShape)mRoom.FXShapes[shapeidx];
                    if (!shape.Active)
                        continue;
                    Vec2i center = new Vec2i();
                    for (int i = 0; i < shape.Positions.Length; ++i)
                    {
                        if ((shape.Positions[i] - clickpos).length() <= 4)
                        {
                            mDragMode = DragMode.DragCorner;
                            mDraggingShape = shape;
                            mShapeIndex = i;
                            mDragOffset = clickpos - center;
                            return;
                        }
                        center += shape.Positions[i];
                    }
                    center /= shape.Positions.Length;
                    if ((center - clickpos).length() <= 10)
                    {
                        mDragMode = DragMode.DragPosition;
                        mDraggingShape = shape;
                        mDragOffset = pos;
                        break;
                    }
                    Vec2i depthcenter = new Vec2i(center.x, (int)((shape.Depth + 0.5f) * mData.WalkGridSize - mData.WalkGridSize / 2));
                    if ((depthcenter - clickpos).length() <= 5)
                    {
                        mDragMode = DragMode.DragBottom;
                        mDraggingShape = shape;
                        mDragOffset = depthcenter - pos;
                        return;
                    }
                }
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

            Color bordercolor = Color.Red;
            if (mFramecount < 5 || (mFramecount >= 10 && mFramecount < 15))
                bordercolor = Color.Yellow;
            foreach (System.Collections.Generic.KeyValuePair<int,DrawableObject> pair in blitqueue)
            {
                bool isSelected = mControl.SelectedObject == pair.Value;
                pair.Value.draw(e.Graphics, mMode == ViewMode.Objects, isSelected ? bordercolor : Color.Red);
            }

            //draw view specific stuff
            Vec2i mp = mMousePos + mRoom.ScrollOffset;
            int wmscale = mRoom.DoubleWalkmap ? 2 : 1;
            int wmx = wmscale * mp.x / mData.WalkGridSize;
            int wmy = wmscale * mp.y / mData.WalkGridSize;
            if (wmx < 0)
                wmx = 0;
            if (wmy < 0)
                wmy = 0;
            if (wmx > mRoom.Walkmap.GetUpperBound(0))
                wmx = mRoom.Walkmap.GetUpperBound(0);
            if (wmy > mRoom.Walkmap.GetUpperBound(1))
                wmy = mRoom.Walkmap.GetUpperBound(1);
            bool wmfree = mRoom.Walkmap[wmx, wmy].isFree;
            Font f = new Font(Fonts.DefaultFont.FontFamily, 11);
            if (mMode == ViewMode.Objects){
                string s = String.Format("Position: Room({0}/{1}) Walkmap({2}/{3}){4} Mouse({5}/{6})", mRoom.ScrollOffset.x/mData.WalkGridSize, mRoom.ScrollOffset.y/mData.WalkGridSize, wmx+1, wmy+1, wmfree ? 'F' : 'B', mp.x, mp.y);
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y, s, f);
                string s2 = "Object:";
                DrawableObject drob = getObjectAt(mMousePos+mRoom.ScrollOffset, false);
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
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y + f.Height, s2, f);
            }
            else if (mMode == ViewMode.Walkmap)
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
                    e.Graphics.DrawEllipse(Pens.Black, ((wmx - mWalkmapPaintMode) * mData.WalkGridSize + 2) * scale,
                                    ((wmy - mWalkmapPaintMode) * mData.WalkGridSize + 2) * scale,
                                    (size * mData.WalkGridSize - 4) * scale, (size * mData.WalkGridSize - 4) * scale);
                    e.Graphics.DrawEllipse(Pens.LightGray, ((wmx - mWalkmapPaintMode) * mData.WalkGridSize + 1) * scale,
                                    ((wmy - mWalkmapPaintMode) * mData.WalkGridSize + 1) * scale,
                                    (size * mData.WalkGridSize - 2) * scale, (size * mData.WalkGridSize - 2) * scale);
                }
                else
                {
                    SolidBrush br = null;
                    if (mWalkmapPaintMode == 3)
                        br = new SolidBrush(Color.FromArgb(100, Color.Yellow));
                    else
                        br = new SolidBrush(Color.FromArgb(100, Color.Red));
                    e.Graphics.FillRectangle(br, wmx * mData.WalkGridSize * scale, wmy * mData.WalkGridSize * scale,
                                mData.WalkGridSize * scale, mData.WalkGridSize * scale);
                }
                //draw information
                string s = String.Format("{0} ({1},{2})", wmfree ? "Free" : "Blocked", wmx + 1, wmy + 1);
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y, s, f);
            }
            else if (mMode == ViewMode.Deepmap)
            {
                int y1 = mRoom.Depthmap.x * mData.WalkGridSize;
                int y2 = mRoom.Depthmap.y * mData.WalkGridSize;
                SolidBrush blue1 = new SolidBrush(Color.FromArgb(100, Color.Blue));
                e.Graphics.FillRectangle(blue1, 0, 0, mData.Settings.Resolution.x * 3, y1);
                SolidBrush blue2 = new SolidBrush(Color.FromArgb(40, Color.Blue));
                e.Graphics.FillRectangle(blue2, 0, y1, mData.Settings.Resolution.x * 3, y2-y1);
                Pen bluepen = new Pen(Color.Blue, 3.0f);
                e.Graphics.DrawLine(bluepen, 0, y1, mData.Settings.Resolution.x * 3, y1);
                e.Graphics.DrawLine(bluepen, 0, y2, mData.Settings.Resolution.x * 3, y2);
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, y1-f.Height, "Depth 1: "+mRoom.Depthmap.x+" (Pull me)", f);
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, y2 - f.Height, "Depth 2: " + mRoom.Depthmap.y + " (Pull me)", f);
            }
            else if (mMode == ViewMode.Inventory)
            {
                if (!mRoom.HasInventory)
                {
                    Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y, "Draw an inventoryfield into the room.", f);
                    return;
                }
                SolidBrush blue = new SolidBrush(Color.FromArgb(50, Color.Blue));
                int sizex = (int)((mRoom.InvSize.x * 61) * mRoom.InvScale.x) + 30;
                int sizey = (int)((mRoom.InvSize.y * 59) * mRoom.InvScale.y) + 5;
                e.Graphics.FillRectangle(blue, mRoom.InvPos.x, mRoom.InvPos.y, sizex, sizey);
                e.Graphics.DrawRectangle(Pens.DarkBlue, mRoom.InvPos.x, mRoom.InvPos.y, sizex, sizey);
                for (int x = 0; x < mRoom.InvSize.x; ++x)
                {
                    for (int y = 0; y < mRoom.InvSize.y; ++y)
                    {
                        float posx = x * ((int)(61 * mRoom.InvScale.x)) + 30 + mRoom.InvPos.x;
                        float posy = y * ((int)(59 * mRoom.InvScale.y)) + 5 + mRoom.InvPos.y;
                        e.Graphics.DrawRectangle(Pens.Blue, posx, posy, 50*mRoom.InvScale.x, 48*mRoom.InvScale.y);
                    }
                }
                Pen tbp = new Pen(Color.DarkBlue, 5.0f);
                int ydown = mRoom.InvPos.y+sizey;
                int xmiddle = mRoom.InvPos.x + sizex / 2;
                e.Graphics.DrawLine(tbp, xmiddle - 15, ydown - 3, xmiddle + 15, ydown - 3);
                int ymiddle = mRoom.InvPos.y + sizey / 2;
                int xright = mRoom.InvPos.x + sizex;
                e.Graphics.DrawLine(tbp, xright - 3, ymiddle - 15, xright - 3, ymiddle + 15);
      
                Point[] points = new Point[4];
                points[0] = new Point(xright-11, ydown);
                points[1] = new Point(xright-5, ydown);
                points[2] = new Point(xright, ydown - 5);
                points[3] = new Point(xright, ydown - 11);
                LinearGradientBrush lgb = new LinearGradientBrush(new Point(xright - 8, ydown), new Point(xright, ydown - 8), Color.DarkBlue, Color.Blue);
                e.Graphics.FillPolygon(lgb, points);
                string s = String.Format("Inventory field: {0}x{1} fields", mRoom.InvSize.x, mRoom.InvSize.y);
                Utilities.drawText(e.Graphics, mRoom.ScrollOffset.x, mRoom.ScrollOffset.y, s, f);
            }
            else if (mMode == ViewMode.Specialfx)
            {
                for (int coloridx = 0; coloridx < mRoom.FXShapes.Count; ++coloridx)
                {
                    FxShape shape = (FxShape)mRoom.FXShapes[coloridx];
                    if (!shape.Active)
                        continue;
                    Color basecol = Color.Black;
                    switch (coloridx)
                    {
                        case 2:
                            basecol = Color.Red;
                            break;
                        case 1:
                            basecol = Color.Green;
                            break;
                        case 0:
                            basecol = Color.Blue;
                            break;
                    }
                    SolidBrush b = new SolidBrush(Color.FromArgb(75, basecol));
                    GraphicsPath path = new GraphicsPath();
                    for (int i = 0; i < shape.Positions.Length - 1; ++i)
                    {
                        path.AddLine(shape.Positions[i].x, shape.Positions[i].y, shape.Positions[i + 1].x, shape.Positions[i + 1].y);
                    }
                    path.AddLine(shape.Positions[shape.Positions.Length - 1].x, shape.Positions[shape.Positions.Length - 1].y, shape.Positions[0].x, shape.Positions[0].y);
                    e.Graphics.FillPath(b, path);
                    Pen p = new Pen(basecol);
                    e.Graphics.DrawPath(p, path);
                    Vec2i center = new Vec2i();
                    for (int i = 0; i < shape.Positions.Length; ++i)
                    {
                        Utilities.drawCircle(e.Graphics, p, 4, shape.Positions[i]);
                        center += shape.Positions[i];
                    }
                    center /= shape.Positions.Length;
                    Utilities.drawCrosshair(e.Graphics, center, basecol, 1.0f);
                    string s = "";
                    if (shape.Effect == FxShape.FxEffect.WALL_MIRROR)
                    {
                        Utilities.drawDepthHandle(e.Graphics, mData, center, shape.Depth + 0.5f, basecol);
                        s = "Wall-Mirror";
                    }
                    else if (shape.Effect == FxShape.FxEffect.FLOOR_MIRROR)
                        s = "Floor-Mirror";
                    else if (shape.Effect == FxShape.FxEffect.PARTICLE_BARRIER)
                        s = "Particle-Barrier";
                    Utilities.drawText(e.Graphics, shape.Positions[0].x, shape.Positions[0].y, s, f);
                }
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
                if (pair.Key < iter.Value.Key)
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

        private DrawableObject getObjectAt(Vec2i pos, bool secondObject)
        {
            DrawableObject ret = null;
            DrawableObject ret2 = null;
            int retdepth = -1;
            foreach (ObjectInstance obj in mRoom.Objects)
            {
                if (obj.isHit(pos))
                {
                    int depth = getDepth(obj);
                    if (depth >= retdepth)
                    {
                        retdepth = depth;
                        ret2 = ret;
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
                        ret2 = ret;
                        ret = chr;
                    }
                }
            }
            return secondObject ? ret2 : ret;
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
            return (chr.Position.y / mData.WalkGridSize);
        }

        public ViewMode Viewmode
        {
            set { mMode = value; Invalidate(); mControl.ViewMode = value; }
        }

        Vec2i clickToWalkmap(int x, int y)
        {
            Vec2i ret = new Vec2i(x, y);
            ret.x += mRoom.ScrollOffset.x;
            ret.y += mRoom.ScrollOffset.y;
            int wmscale = mRoom.DoubleWalkmap ? 2 : 1;
            ret.x = wmscale * ret.x / mData.WalkGridSize;
            ret.y = wmscale * ret.y / mData.WalkGridSize;
            return ret;
        }

        void modifyWalkmap(int x, int y)
        {
            Vec2i wm = clickToWalkmap(x, y);
            if (mWalkmapPaintMode <= 2)
            {
                ToolStripMenuItem it = menuWalkmap.Items[0] as ToolStripMenuItem;
                mRoom.Walkmap[wm.x, wm.y].isFree = it.Checked;
                if (mWalkmapPaintMode > 0)
                {
                    setWalkmapPoint(wm.x + 1, wm.y, it.Checked);
                    setWalkmapPoint(wm.x - 1, wm.y, it.Checked);
                    setWalkmapPoint(wm.x, wm.y + 1, it.Checked);
                    setWalkmapPoint(wm.x, wm.y - 1, it.Checked);
                }
                if (mWalkmapPaintMode > 1)
                {
                    setWalkmapPoint(wm.x + 1, wm.y - 1, it.Checked);
                    setWalkmapPoint(wm.x - 1, wm.y + 1, it.Checked);
                    setWalkmapPoint(wm.x - 1, wm.y - 1, it.Checked);
                    setWalkmapPoint(wm.x + 1, wm.y + 1, it.Checked);
                    setWalkmapPoint(wm.x + 2, wm.y, it.Checked);
                    setWalkmapPoint(wm.x - 2, wm.y, it.Checked);
                    setWalkmapPoint(wm.x, wm.y + 2, it.Checked);
                    setWalkmapPoint(wm.x, wm.y - 2, it.Checked);
                }
            }
            else if (mWalkmapPaintMode == 3)
            {
                //script paster
                Script scr;
                string name = Script.toScriptName(wm.x, wm.y, mRoom.Name, mData);
                if (mRoom.Walkmap[wm.x, wm.y].hasScript)
                {
                    scr = mData.getScript(Script.Type.WALKMAP, name);
                }
                else
                {
                    scr = new Script(Script.Type.WALKMAP);
                    scr.Name = name;
                    mData.addScript(scr);
                    mRoom.Walkmap[wm.x, wm.y].hasScript = true;
                }
                scr.Text = (string)mCopiedWMScript.Clone();
            }
            else if (mWalkmapPaintMode == 4)
            {
                //script eraser
                string name = Script.toScriptName(wm.x, wm.y, mRoom.Name, mData);
                if (mRoom.Walkmap[wm.x, wm.y].hasScript)
                {
                    mData.removeScript(Script.Type.WALKMAP, name);
                    mRoom.Walkmap[wm.x, wm.y].hasScript = false;
                }
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

        //general
        private Room mRoom;
        private AdvData mData;
        private DrawableObject mDragObject;
        private Vec2i mDragOffset;
        private bool mDragDepth;
        private RoomCtrlDlg mControl;
        private Vec2i mMousePos;
        private ViewMode mMode;
        private Timer mTimer;
        private MainForm mOwner;
        //objects
        private int mFramecount;
        //walkmap
        private string mPendingImage;
        private int mWalkmapPaintMode = 0;
        private string mCopiedWMScript = null;
        //deepmap
        bool mDragDepthBottom;
        //inventory and fxshapes
        DragMode mDragMode = DragMode.DragNone;
        //fxshapes
        private FxShape mDraggingShape;
        private int mShapeIndex;

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
            MainForm form = mOwner;
            string scrname = Script.toScriptName(pos.x, pos.y, mRoom.Name, mData);
            form.showScript(Script.Type.WALKMAP, scrname);
        }

        private void copyWalkmapScriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem item1 = menuWalkmap.Items[6] as ToolStripMenuItem;
            ToolStripMenuItem item2 = menuWalkmap.Items[10] as ToolStripMenuItem;
            item1.Enabled = true;
            item2.Enabled = true;
            Vec2i pos = new Vec2i(mMousePos.x / mData.WalkGridSize, mMousePos.y / mData.WalkGridSize);
            string scrname = Script.toScriptName(pos.x, pos.y, mRoom.Name, mData);
            Script scr = mData.getScript(Script.Type.WALKMAP, scrname);
            mCopiedWMScript = scr.Text;
        }

        private void pasteWalkmapScriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int oldmode = mWalkmapPaintMode;
            mWalkmapPaintMode = 3;
            modifyWalkmap(mMousePos.x, mMousePos.y);
            mWalkmapPaintMode = oldmode;
        }

        private void removeFieldToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mRoom.HasInventory = false;
        }

        private void correctShapeBoundary(int index, Vec2i correctingDiff, ref Vec2i diff)
        {
            for (int j = 0; j <= index; ++j)
            {
                mDraggingShape.Positions[j] += correctingDiff;
            }
            diff += correctingDiff;
        }

        public void updateRoom()
        {
            this.Text = "Room (" + mRoom.Name + ")";
        }

        private void removeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DrawableObject obj = mControl.SelectedObject;
            if (obj is ObjectInstance)
            {
                ObjectInstance inst = (ObjectInstance)obj;
                DialogResult res = MessageBox.Show("Delete the Object \"" + inst.Name + "\" from your room?", "Confirmation", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (res == DialogResult.Yes)
                    mRoom.removeObject(inst);
            }
            else
            {
                CharacterInstance inst = (CharacterInstance)obj;
                DialogResult res = MessageBox.Show("Delete the Character \"" + inst.Name + "\" from your room?", "Confirmation", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (res == DialogResult.Yes)
                    mRoom.removeCharacter(inst);
            }
            mControl.SelectedObject = null;
        }

        private void showInToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DrawableObject obj = mControl.SelectedObject;
            MainForm form = mOwner;
            if (obj is ObjectInstance)
            {
                ObjectInstance inst = (ObjectInstance)obj;
                form.showObject(inst.Object.Name);
            }
            else
            {
                CharacterInstance inst = (CharacterInstance)obj;
                form.showCharacter(inst.Character.Name);
            }
        }

        private void setToBackToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DrawableObject obj = mControl.SelectedObject;
            if (obj is ObjectInstance)
            {
                mRoom.Objects.Remove(obj);
                mRoom.Objects.Insert(0, obj);
            }
            else
            {
                mRoom.Characters.Remove(obj);
                mRoom.Characters.Insert(0, obj);
            }
        }

        private void setToFrontToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DrawableObject obj = mControl.SelectedObject;
            if (obj is ObjectInstance)
            {
                mRoom.Objects.Remove(obj);
                mRoom.Objects.Add(obj);
            }
            else
            {
                mRoom.Characters.Remove(obj);
                mRoom.Characters.Add(obj);
            }
        }
    }
}