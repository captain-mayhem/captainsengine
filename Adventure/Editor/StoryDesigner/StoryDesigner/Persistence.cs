using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml.Serialization;

namespace StoryDesigner
{
    [Serializable()]
    public class Persistence
    {
        public string LastOpenPath = "";
        public string ImageLoadPath = "";
        public bool FitImages = true;
        public bool CreateBackups = true;

        protected Persistence()
        {
        }

        public void save()
        {
            FileStream fs = new FileStream(getSavepath(true), FileMode.Create);
            XmlSerializer serializer = new XmlSerializer(typeof(Persistence));
            serializer.Serialize(fs, this);
            fs.Close();
        }

        public static Persistence load()
        {
            try
            {
                XmlSerializer serializer = new XmlSerializer(typeof(Persistence));
                FileStream fs = new FileStream(getSavepath(false), FileMode.Open);
                Persistence p = (Persistence)serializer.Deserialize(fs);
                return p;
            }
            catch (Exception)
            {
                return new Persistence();
            }
        }

        protected static string getSavepath(bool create)
        {
            string dir = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + Path.DirectorySeparatorChar +
                "StoryDesigner";
            if (create && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);
            return  dir+Path.DirectorySeparatorChar+"StoryDesigner.xml";
        }
    }
}
