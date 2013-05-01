using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ObjectDlg : ChildForm
    {
        public ObjectDlg(AdvObject obj)
        {
            InitializeComponent();
            this.stateFrameImage1.ClientSizeChanged += new EventHandler(stateFrameImage1_ClientSizeChanged);
            mObject = obj;
            updateObject();
            this.stateFrameImage1.Data = obj;
            this.lighten_object.Checked = obj.Lighten;
            this.lighten_object.CheckedChanged += new EventHandler(lighten_object_CheckedChanged);
        }

        void lighten_object_CheckedChanged(object sender, EventArgs e)
        {
            mObject.Lighten = lighten_object.Checked;
        }

        void stateFrameImage1_ClientSizeChanged(object sender, EventArgs e)
        {
            int height = stateFrameImage1.Location.Y+stateFrameImage1.Size.Height - 30;
            this.lighten_object.Location = new Point(lighten_object.Location.X, height);
        }

        public AdvObject Object
        {
            get { return mObject; }
        }

        AdvObject mObject;

        public void updateObject()
        {
            this.Text = "Object (" + mObject.Name + ")";
        }
    }
}