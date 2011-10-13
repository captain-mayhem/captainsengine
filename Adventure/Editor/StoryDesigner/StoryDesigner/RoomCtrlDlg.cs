using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class RoomCtrlDlg : Form
    {
        public RoomCtrlDlg(Room room, AdvData data)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
            for (int i = 1; i <= 9; ++i)
            {
                zoomfactor.Items.Add(i);
            }
            for (int i = 0; i <= 10; ++i)
            {
                obj_state.Items.Add(i);
            }
            zoomfactor.SelectedItem = mRoom.Zoom;
            doublewalkmap.Checked = mRoom.DoubleWalkmap;
            roomwidth.Maximum = data.Settings.Resolution.x * 3;
            roomheight.Maximum = data.Settings.Resolution.y * 2;
            roomwidth.Value = mRoom.Size.x;
            roomheight.Value = mRoom.Size.y;
            mDragOffset.x = -1;
            scroller.Paint += new PaintEventHandler(scroller_Paint);
            scroller.MouseDown += new MouseEventHandler(scroller_MouseDown);
            scroller.MouseMove += new MouseEventHandler(scroller_MouseMove);
            scroller.MouseUp += new MouseEventHandler(scroller_MouseUp);
        }

        void scroller_MouseUp(object sender, MouseEventArgs e)
        {
            mDragOffset.x = -1;
        }

        const float divider = 2.0f;

        void scroller_MouseMove(object sender, MouseEventArgs e)
        {
            if (mDragOffset.x == -1)
                return;
            int realx = (int)((e.X/divider - mDragOffset.x) + mScrollOffset.x);
            int realy = (int)((e.Y/divider - mDragOffset.y)+ mScrollOffset.y);
            Console.WriteLine(realx);
            if (realx < 0)
                realx = 0;
            if (realy < 0)
                realy = 0;
            mRoom.ScrollOffset.x = (int)(realx * 3.0f / scroller.Width * mData.Settings.Resolution.x);
            mRoom.ScrollOffset.y = (int)(realy * 2.0f / scroller.Height * mData.Settings.Resolution.y);
            if (mRoom.ScrollOffset.x > mRoom.Size.x - mData.Settings.Resolution.x+3)
                mRoom.ScrollOffset.x = mRoom.Size.x - mData.Settings.Resolution.x+3;
            if (mRoom.ScrollOffset.y > mRoom.Size.y - mData.Settings.Resolution.y + 7)
                mRoom.ScrollOffset.y = mRoom.Size.y - mData.Settings.Resolution.y + 7;
            scroller.Invalidate();
        }

        void scroller_MouseDown(object sender, MouseEventArgs e)
        {
            mDragOffset.x = e.X/(int)divider;
            mDragOffset.y = e.Y/(int)divider;
            mScrollOffset.x = scroller.Width / 3.0f * mRoom.ScrollOffset.x / mData.Settings.Resolution.x;
            mScrollOffset.y = scroller.Height / 2.0f * mRoom.ScrollOffset.y / mData.Settings.Resolution.y;
        }

        void scroller_Paint(object sender, PaintEventArgs e)
        {
            PictureBox ctrl = (PictureBox)sender;
            e.Graphics.DrawRectangle(Pens.Black, 0, 0, ctrl.Width-1, ctrl.Height-1);
            Rectangle roomrect = new Rectangle(0, 0,
                (int)(ctrl.Width / 3.0f * mRoom.Size.x / mData.Settings.Resolution.x),
                (int)(ctrl.Height / 2.0f * mRoom.Size.y / mData.Settings.Resolution.y));
            e.Graphics.FillRectangle(Brushes.LightGray, roomrect);
            e.Graphics.DrawRectangle(Pens.Black, roomrect);
            roomrect.X = (int)(ctrl.Width / 3.0f * mRoom.ScrollOffset.x / mData.Settings.Resolution.x);
            roomrect.Y = (int)(ctrl.Height / 2.0f * mRoom.ScrollOffset.y / mData.Settings.Resolution.y);
            roomrect.Width = ctrl.Width / 3;
            roomrect.Height = ctrl.Height / 2;
            e.Graphics.FillRectangle(Brushes.White, roomrect);
            e.Graphics.DrawRectangle(Pens.Black, roomrect);
        }

        public DrawableObject SelectedObject
        {
            get { return mObject; }
            set {
                mObject = value;
                objectgroup.Enabled = false;
                charactergroup.Enabled = false;
                if (mObject == null)
                    return;
                if (mObject is ObjectInstance)
                {
                    objectgroup.Enabled = true;
                    ObjectInstance obj = (ObjectInstance)mObject;
                    obj_instname.Text = obj.Name;
                    obj_locked.Checked = obj.Locked;
                    obj_state.SelectedIndex = obj.State;
                    switch (obj.Layer)
                    {
                        case 0:
                            draw_back.Checked = true;
                            break;
                        case 1:
                            draw_middle.Checked = true;
                            break;
                        case 2:
                            draw_front.Checked = true;
                            break;
                    }
                }
                else if (mObject is CharacterInstance)
                {
                    charactergroup.Enabled = true;
                    CharacterInstance chr = (CharacterInstance)mObject;
                    chr_instancename.Text = chr.Name;
                    chr_locked.Checked = chr.Locked;
                    chr_unmovable.Checked = chr.Unmovable;
                    switch (chr.LookDir)
                    {
                        case 1:
                            look_down.Checked = true;
                            break;
                        case 2:
                            look_up.Checked = true;
                            break;
                        case 3:
                            look_right.Checked = true;
                            break;
                        case 4:
                            look_left.Checked = true;
                            break;
                    }
                }
            }
        }

        private Room mRoom;
        private AdvData mData;
        private DrawableObject mObject;
        private Vec2i mDragOffset;
        private Vec2f mScrollOffset;
    }
}