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
        }

        AdvData mData;
    }
}