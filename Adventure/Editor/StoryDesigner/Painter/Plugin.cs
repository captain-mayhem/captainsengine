using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

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
            Canvas c = new Canvas(800, 480);
            c.Show();
            return;
        }

        string mName;
        StoryDesigner.PluginHost mHost;
    }
}
