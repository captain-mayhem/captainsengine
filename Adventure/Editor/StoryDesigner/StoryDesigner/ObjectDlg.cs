using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ObjectDlg : Form
    {
        public ObjectDlg(AdvObject obj)
        {
            InitializeComponent();
            //this.stateFrameImage1.ClientSizeChanged += new EventHandler(stateFrameImage1_ClientSizeChanged);
            this.Text = "Object (" + obj.Name + ")";
            this.stateFrameImage1.Data = obj;
        }

        void stateFrameImage1_ClientSizeChanged(object sender, EventArgs e)
        {
            int widthdiff = stateFrameImage1.ClientSize.Width - this.ClientSize.Width;
            this.ClientSize = stateFrameImage1.ClientSize;
            //stateFrameImage1.Location = new Point(stateFrameImage1.Location.X + widthdiff / 2, stateFrameImage1.Location.Y);
        }
    }
}