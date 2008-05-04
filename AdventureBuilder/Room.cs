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
      ObjProperty prop = new ObjProperty("description");
      m_properties.Add(prop);
      ObjValue val = new ObjValue(0);
      prop.Values.Add(val);
      ObjResponse resp = new ObjResponse("Das ist ein Raum");
      val.Responses.Add(resp);
		}

    public Room(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.Room;
    }
	}
}
