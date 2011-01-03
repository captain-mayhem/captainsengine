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
            this.stateFrameImage1.Data = mData.Cursor;
            this.stateFrameImage1.PictureBoxSize = new Size(96, 96);
            this.stateFrameImage1.PictureChanged += new EventHandler(stateFrameImage1_PictureChanged);
            this.command.Items.Add("walkto (standard)");
            foreach (KeyValuePair<string,string> pair in mData.Settings.Commands)
            {
                this.command.Items.Add(pair.Key);
            }
            this.command.Items.Add("loading icon");
            this.command.SelectedIndex = 0;
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