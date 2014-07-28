using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Reflection;
using System.Drawing;
using System.Windows.Forms;

namespace StoryDesigner
{
    public interface Updatable
    {
        string ApplicationName { get; }
        string ApplicationID { get; }
        Version ApplicationVersion { get; }
        string ApplicationLocation { get; }
        Icon ApplicationIcon { get; }
        Uri UpdateXmlLocation { get; }
        Form Context { get; }
    }
}
