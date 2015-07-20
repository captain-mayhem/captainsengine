using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Painter
{
    public class Plugin : StoryDesigner.Plugin
    {
        public Plugin()
        {
            mName = "Painter";
        }

        public string Name
        {
            get { return mName; }
            set { mName = value; }
        }

        public StoryDesigner.PluginHost Host
        {
            get { return mHost; }
            set { mHost = value; mHost.registerPlugin(this); }
        }

        public void show()
        {
            Form frm = mHost.getActiveWindow();
            Canvas c;
            if (frm is StoryDesigner.ImageViewer)
            {
                StoryDesigner.ImageViewer imgv = (StoryDesigner.ImageViewer)frm;
                c = new Canvas(imgv.Image);
            }
            else
            {
                c = new Canvas(800, 480);
            }
            c.Show();
            return;
        }

        string mName;
        StoryDesigner.PluginHost mHost;
    }
}
