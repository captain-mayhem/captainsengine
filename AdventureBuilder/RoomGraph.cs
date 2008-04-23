using System;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for RoomNode.
	/// </summary>
	public class RoomGraph : Graph
	{
		public RoomGraph()
		{
			m_type = Factory.ObjectType.RoomGraph;
		}

    public RoomGraph(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.RoomGraph;
    }

    public override GraphNode newNode(Point p)
    {
      return new Room(p);
    }
	}
}
