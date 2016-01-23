using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace StoryWriter
{
    class Document
    {
        public Document()
        {

        }

        public Document(string filename)
        {
            Scenes = new ArrayList();
            Chapters = new ArrayList();
            XmlDocument doc = new XmlDocument();
            doc.Load(filename);
            if (doc.DocumentElement.Name == "YWRITER5")
            {
                foreach (XmlNode node in doc.DocumentElement.ChildNodes)
                {
                    if (node.Name == "PROJECT")
                    {
                        readProjectSettings(node.ChildNodes);
                    }
                    else if (node.Name == "SCENES")
                    {
                        foreach (XmlNode scene in node.ChildNodes)
                        {
                            addScene(scene);
                        }
                    }
                    else if (node.Name == "CHAPTERS")
                    {
                        foreach (XmlNode chapter in node.ChildNodes)
                        {
                            addChapter(chapter);
                        }
                        Chapters.Sort();
                    }
                }
            }           
        }

        public Chapter getChapter(int id)
        {
            foreach (Chapter c in Chapters)
            {
                if (id == c.ID)
                    return c;
            }
            return null;
        }

        void readProjectSettings(XmlNodeList list)
        {
            foreach (XmlNode node in list){
                switch(node.Name){
                    case "Title":
                        Title = node.InnerText;
                        break;
                    case "AuthorName":
                        AuthorName = node.InnerText;
                        break;
                }
            }
        }

        void addChapter(XmlNode chapter)
        {
            Chapter c = new Chapter();
            foreach (XmlNode node in chapter.ChildNodes){
                switch (node.Name){
                    case "ID":
                        c.ID = Convert.ToInt32(node.InnerText);
                        break;
                    case "Title":
                        c.Title = node.InnerText;
                        break;
                    case "Desc":
                        c.Description = node.InnerText;
                        break;
                    case "SortOrder":
                        c.SortID = Convert.ToInt32(node.InnerText);
                        break;
                    case "Unused":
                        c.Unused = node.InnerText == "-1";
                        break;
                    case "Type":
                        c.Type = (Chapter.ChType)Convert.ToInt32(node.InnerText);
                        break;
                    case "Scenes":
                        foreach (XmlNode scene in node.ChildNodes)
                        {
                            int id = Convert.ToInt32(scene.InnerText);
                            Scene s = Scenes[id - 1] as Scene;
                            c.Scenes.Add(s);
                            s.Parent = c;
                        }
                        break;
                }
            }
            Chapters.Add(c);
        }

        void addScene(XmlNode scene)
        {
            Scene s = new Scene();
            foreach (XmlNode node in scene.ChildNodes)
            {
                switch (node.Name)
                {
                    case "ID":
                        s.ID = Convert.ToInt32(node.InnerText);
                        break;
                    case "Title":
                        s.Title = node.InnerText;
                        break;
                    case "WordCount":
                        s.WordCount = Convert.ToInt32(node.InnerText);
                        break;
                }
            }
            Scenes.Add(s);
        }

        public string Title;
        public string AuthorName;
        public ArrayList Chapters;
        public ArrayList Scenes;
    }
}
