using System;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Room.
	/// </summary>
	[Serializable]
	public class Room : GraphNode
	{
		public Room(Point p) : base(p)
		{
			//
			// TODO: Add constructor logic here
			//
		}

    public Room(BinaryReader reader) : base(reader)
    {
    }
	}
}
