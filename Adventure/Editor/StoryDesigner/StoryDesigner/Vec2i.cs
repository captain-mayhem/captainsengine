using System;
using System.Collections.Generic;
using System.Text;

namespace StoryDesigner
{
    public struct Vec2i
    {
        public Vec2i(int x, int y)
        {
            this.x = x;
            this.y = y;
        }
        public static Vec2i operator -(Vec2i vec1, Vec2i vec2)
        {
            Vec2i ret;
            ret.x = vec1.x - vec2.x;
            ret.y = vec1.y - vec2.y;
            return ret;
        }
        public static Vec2i operator *(Vec2i vec, float val){
            vec.x = (int)(vec.x * val);
            vec.y = (int)(vec.y * val);
            return vec;
        }
        public static Vec2i operator /(Vec2i vec, float val)
        {
            vec.x = (int)(vec.x / val);
            vec.y = (int)(vec.y / val);
            return vec;
        }
        public float length()
        {
            return (float)Math.Sqrt(x * x + y * y);
        }
        public int x;
        public int y;
    }
}
