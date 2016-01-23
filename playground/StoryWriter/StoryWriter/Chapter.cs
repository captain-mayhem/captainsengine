using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StoryWriter
{
    class Chapter : IComparable
    {
        public enum ChType
        {
            Chapter,
            Other,
        }

        public int ID;
        public string Title;
        public bool Unused = false;
        public ChType Type = ChType.Chapter;
        public int SortID;
        public ArrayList Scenes;
        public string Description;

        public Chapter()
        {
            Scenes = new ArrayList();
        }

        public object[] toArray(ref int wordaccu)
        {
            object[] ret = new object[5];
            ret[0] = Title;
            ret[1] = WordCount;
            wordaccu += WordCount;
            ret[2] = Scenes.Count;
            ret[3] = wordaccu;
            ret[4] = Description;
            return ret;
        }

        public Color TextColor
        {
            get
            {
                if (Unused)
                    return Color.Gray;
                if (Type == ChType.Chapter)
                    return Color.Green;
                else if (Type == ChType.Other)
                    return Color.Blue;
                return Color.Black;
            }
        }

        public int CompareTo(object obj)
        {
            if (obj == null)
                return 1;
            Chapter other = obj as Chapter;
            if (other != null)
                return SortID.CompareTo(other.SortID);
            else
                throw new ArgumentException("Object is not a Chapter");
        }

        public int WordCount
        {
            get
            {
                if (Type != ChType.Chapter)
                    return 0;
                int accu = 0;
                foreach (Scene s in Scenes)
                {
                    accu += s.WordCount;
                }
                return accu;
            }
        }
    }
}
