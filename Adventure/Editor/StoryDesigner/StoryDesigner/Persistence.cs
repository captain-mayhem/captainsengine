using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml.Serialization;
using System.Drawing;
using System.Windows.Forms;
using System.ComponentModel;

namespace StoryDesigner
{
    [Serializable()]
    public class Persistence
    {
        public string LastOpenPath = "";
        public string ImageLoadPath = "";
        public string MusicLoadPath = "";
        public string SoundLoadPath = "";
        public string VideoLoadPath = "";
        public bool FitImages = true;
        public bool CreateBackups = true;
        public Point MainFormPosition = new Point(0, 100);
        public Point RoomDlgPosition = new Point(250, 200);
        public Point ObjectDlgPosition = new Point(300, 250);
        public Point ScriptDlgPosition = new Point(250, 300);
        public Point ItemDlgPosition = new Point(400, 300);
        public Point CharacterDlgPosition = new Point(450, 250);
        [XmlIgnore]
        public Color CharBackColor = Color.Black;
        [EditorBrowsable(EditorBrowsableState.Never), XmlElement("CharBackColor")]
        public string CharBackColorXml
        {
            get { return ColorTranslator.ToHtml(CharBackColor); }
            set { CharBackColor = ColorTranslator.FromHtml(value); }
        }
        public int LCID = System.Threading.Thread.CurrentThread.CurrentUICulture.LCID;
        public string UpdateURL = "http://captainsengine.sourceforge.net/update.xml";
        public DateTime LastUpdate = DateTime.Now;
        public bool CropTransparentImages = false;

        protected Persistence()
        {
        }

        public void save()
        {
            try
            {
                FileStream fs = new FileStream(getSavepath(true), FileMode.Create);
                XmlSerializer serializer = new XmlSerializer(typeof(Persistence));
                serializer.Serialize(fs, this);
                fs.Close();
            }
            catch (IOException)
            {
                MessageBox.Show("Failed to write StoryDesigner.xml. Is another process accesing the file?");
            }
        }

        public static Persistence load()
        {
            try
            {
                XmlSerializer serializer = new XmlSerializer(typeof(Persistence));
                FileStream fs = new FileStream(getSavepath(false), FileMode.Open);
                Persistence p = (Persistence)serializer.Deserialize(fs);
                fs.Close();
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
