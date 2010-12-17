using System;
using System.Collections.Generic;
using System.Text;

namespace StoryDesigner
{
    enum ResourceID
    {
        SCRIPT = 0,
        ITEM = 1,
        MUSIC = 2,
        OBJECT = 3,
        IMAGE = 4,
        ROOM = 5,
        SOUND = 6,
        FOLDER = 11,
        CHARACTER = 15,
    }

    struct ProjectSettings
    {
        public string Directory;
        public Vec2i Resolution;
        public string TsBackground;
        public string Startscript;
    }

    class AdvData
    {
        public AdvData(AdvFileReader reader)
        {
            Settings = new ProjectSettings();
            mImages = new Dictionary<string, string>();
            mReader = reader;
        }

        public System.Drawing.Bitmap getImage(string filename)
        {
            return mReader.getImage(filename);
        }

        public Dictionary<string, string> Images
        {
            get { return mImages; }
            set { mImages = value; }
        }

        public ProjectSettings Settings;
        Dictionary<string, string> mImages;
        AdvFileReader mReader;
    }
}
