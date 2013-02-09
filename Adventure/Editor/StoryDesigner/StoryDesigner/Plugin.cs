using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Reflection;

namespace StoryDesigner
{
    public interface Plugin
    {
        string Name { get; set; }
        PluginHost Host { get; set; }
        void show();
    }

    public interface PluginHost
    {
        bool registerPlugin(Plugin plug);
    }

    public partial class MainForm
    {
        private void MainForm_Load(object sender, EventArgs e)
        {
            string path = Application.StartupPath;
            string[] plugins = Directory.GetFiles(path, "*.dll");
            mPlugins = new Plugin[plugins.Length];
            for (int i = 0; i < plugins.Length; ++i)
            {
                int lastPathSep = plugins[i].LastIndexOf(Path.DirectorySeparatorChar);
                string args = plugins[i].Substring(lastPathSep + 1, plugins[i].Length - 4 - lastPathSep - 1);
                Type objType = null;
                try
                {
                    Assembly ass = Assembly.Load(args);
                    if (ass != null)
                    {
                        objType = ass.GetType(args + ".Plugin");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                try
                {
                    if (objType != null)
                    {
                        mPlugins[i] = (Plugin)Activator.CreateInstance(objType);
                        mPlugins[i].Host = this;
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }

        private void pluginClicked(object sender, System.EventArgs e)
        {
            string name = ((ToolStripMenuItem)sender).Text;
            for (int i = 0; i < mPlugins.Length; ++i)
            {
                if (mPlugins[i] == null)
                    continue;
                if (mPlugins[i].Name == name)
                {
                    mPlugins[i].show();
                    break;
                }
            }
        }

        public bool registerPlugin(Plugin plug)
        {
            //MenuItem it = (MenuItem)pluginsToolStripMenuItem;
            ToolStripMenuItem item = new ToolStripMenuItem(plug.Name);
            item.Click += new EventHandler(pluginClicked);
            pluginsToolStripMenuItem.DropDownItems.Add(item);
            return true;
        }

        Plugin[] mPlugins;
    }
}
