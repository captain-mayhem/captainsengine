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
			//
			// TODO: Add constructor logic here
			//
		}

    public RoomGraph(BinaryReader reader) : base(reader)
    {

    }

    public override GraphNode newNode(Point p)
    {
      return new Room(p);
    }
	}
}
