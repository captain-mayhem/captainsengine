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
      m_name = "Raum";
			m_type = Factory.ObjectType.Room;
      ObjProperty prop = new ObjProperty("description");
      m_properties.Add(prop);
      ObjValue val = new ObjValue(0);
      prop.Values.Add(val);
      ObjResponse resp = new ObjResponse("text");
      resp.Operation = Opcode.textout;
      resp.Arguments[0] = "Raumbeschreibung";
      val.Responses.Add(resp);
      ObjProperty state = new ObjProperty("state");
      m_properties.Add(state);
      ObjValue stateval = new ObjValue(0);
      state.Values.Add(stateval);
		}

    public Room(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.Room;
    }

    public override object getVisited(GraphVisitor visitor)
    {
      return visitor.visit(this);
    }

    public override object getVisited(AdventureGraphVisitor visitor){
      return visitor.visit(this);
    }

	}
}
