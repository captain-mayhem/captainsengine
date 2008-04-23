using System;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Room.
	/// </summary>
	public class Room : GraphNode
	{
		public Room(Point p) : base(p)
		{
			m_type = Factory.ObjectType.Room;
		}

    public Room(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.Room;
    }
	}
}
