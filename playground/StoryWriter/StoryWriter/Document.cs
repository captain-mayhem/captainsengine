using System;
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
            XmlDocument doc = new XmlDocument();
            doc.Load(filename);
            XmlNode node = doc.DocumentElement.SelectSingleNode("/YWRITER5/PROJECT/Title");
            Title = node.InnerText;
            node = doc.DocumentElement.SelectSingleNode("/YWRITER5/PROJECT/AuthorName");
            AuthorName = node.InnerText;
        }

        public string Title;
        public string AuthorName;
    }
}
