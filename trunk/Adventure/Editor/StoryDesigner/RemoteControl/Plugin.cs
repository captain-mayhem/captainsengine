using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace RemoteControl
{
    public class Plugin : StoryDesigner.Plugin
    {
        public Plugin()
        {
            mName = "Remote Control";
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
            Form frm = new MainForm();
            frm.Show();
        }

        string mName;
        StoryDesigner.PluginHost mHost;
    }
}
