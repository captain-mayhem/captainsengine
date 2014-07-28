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
        internal UpdatableFile(Updatable parent, string path)
        {
            this.parent = parent;
            this.path = path;
        }

        public string ApplicationName
        {
            get { return "Engine runtime"; }
        }

        public string ApplicationID
        {
            get { return Path.GetFileName(path); }
        }

        public Version ApplicationVersion
        {
            get { return new Version("0.0.0.0"); }
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

        private Updatable parent;
        private string path;
    }
}
