using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
using System.IO;

namespace StoryDesigner
{
    internal class UpdatableFile : Updatable
    {
        internal UpdatableFile(Updatable parent, string name, string path)
        {
            this.parent = parent;
            this.name = name;
            this.path = path;
            string verfile = Path.Combine(Path.GetDirectoryName(path), Path.GetFileNameWithoutExtension(path)) + ".ver";
            try
            {
                version = new Version(File.ReadAllText(verfile));
            }
            catch
            {
                version = new Version("0.0.0.0");
            }               
        }

        public string ApplicationName
        {
            get { return name; }
        }

        public string ApplicationID
        {
            get { return Path.GetFileName(path); }
        }

        public Version ApplicationVersion
        {
            get { return version; }
        }

        public string ApplicationLocation
        {
            get { return path; }
        }

        public Icon ApplicationIcon
        {
            get { return parent.ApplicationIcon; }
        }

        public Uri UpdateXmlLocation
        {
            get { return parent.UpdateXmlLocation; }
        }

        public Form Context
        {
            get { return parent.Context; }
        }

        public UpdateAction UpdateMode
        {
            get { return UpdateAction.FILECOPY; }
        }

        private Updatable parent;
        private string name;
        private string path;
        private Version version;
    }
}
