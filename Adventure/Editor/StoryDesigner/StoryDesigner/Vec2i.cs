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
        public Vec2i(System.Drawing.Size size)
        {
            x = size.Width;
            y = size.Height;
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
        public static bool operator !=(Vec2i v1, Vec2i v2)
        {
            return v1.x != v2.x || v1.y != v2.y;
        }
        public static bool operator ==(Vec2i v1, Vec2i v2)
        {
            return v1.x == v2.x && v1.y == v2.y;
        }
        public float length()
        {
            return (float)Math.Sqrt(x * x + y * y);
        }
        public override bool Equals(object o)
        {
            if (o is Vec2i)
            {
                Vec2i v = (Vec2i)o;
                return v == this;
            }
            return false;
        }
        public override int GetHashCode()
        {
            return 2 * x * x + 3 * y;
        }
        public int x;
        public int y;
    }
}
