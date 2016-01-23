using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace StoryWriter
{
    class Scene
    {
        public int ID;
        public string Title;
        public Chapter Parent;
        public int WordCount;

        public object[] toArray()
        {
            object[] ret = new object[3];
            ret[1] = WordCount;
            ret[2] = Title;
            return ret;
        }
    }
}
