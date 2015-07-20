using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class MouseIcons : Form
    {
        public MouseIcons(AdvData data)
        {
            InitializeComponent();
            mData = data;
            this.stateFrameImage1.StateChanged += new StateFrameImage.StateEventHandler(stateFrameImage1_StateChanged);
            this.stateFrameImage1.PictureBoxSize = new Size(96, 96);
            this.stateFrameImage1.PictureChanged += new EventHandler(stateFrameImage1_PictureChanged);
            this.command.SelectedIndexChanged += new EventHandler(command_SelectedIndexChanged);
            this.FormClosed += new FormClosedEventHandler(MouseIcons_FormClosed);
            this.rc_change.CheckedChanged += new EventHandler(rc_change_CheckedChanged);
            this.command.Items.Add("walkto (standard)");
            this.command.Items.Add("loading icon");
            foreach (KeyValuePair<string,string> pair in mData.Settings.Commands)
            {
                this.command.Items.Add(pair.Key);
            }
            this.command.SelectedIndex = 0;
            string[] labels = new string[10] { "S", "1", "2", "3", "4", "5", "6", "7", "8", "L"};
            this.stateFrameImage1.setStateLables(labels);
            this.stateFrameImage1.Data = mData.Cursor;
            switch (data.Settings.RightClick)
            {
                case 0:
                    this.rc_change.Checked = true;
                    break;
                case 1:
                    this.rc_icon_one.Checked = true;
                    break;
                case 2:
                    this.rc_nothing.Checked = true;
                    break;
            }
            this.mouse_wheel.Checked = data.Settings.UseMouseWheel;
        }

        void rc_change_CheckedChanged(object sender, EventArgs e)
        {
            this.mouse_wheel.Enabled = this.rc_change.Checked;
        }

        void command_SelectedIndexChanged(object sender, EventArgs e)
        {
            mData.Cursor.setCommand(this.stateFrameImage1.State, this.command.SelectedIndex);
        }

        void stateFrameImage1_StateChanged(object sender, StateFrameImage.StateEventArgs e)
        {
            //save command in old state
            //mData.Cursor.setCommand(e.OldState, this.command.SelectedIndex);
            //set command for new state
            if (e.NewState == 0 || e.NewState == 9)
            {
                this.command.Enabled = false;
            }
            else
            {
                this.command.Enabled = true;
            }
            this.command.SelectedIndex = mData.Cursor.getCommand(e.NewState);
        }

        void MouseIcons_FormClosed(object sender, FormClosedEventArgs e)
        {
            if (this.rc_change.Checked)
                mData.Settings.RightClick = 0;
            else if (this.rc_icon_one.Checked)
                mData.Settings.RightClick = 1;
            else if (this.rc_nothing.Checked)
                mData.Settings.RightClick = 2;
            mData.Settings.UseMouseWheel = this.mouse_wheel.Checked;
        }

        void stateFrameImage1_PictureChanged(object sender, EventArgs e)
        {
            Bitmap bmp = this.stateFrameImage1.getBitmap();
            Graphics g = pb_small.CreateGraphics();
            g.DrawImage(bmp, 0, 0, 32, 32);
            g.Dispose();
        }

        AdvData mData;
    }
}