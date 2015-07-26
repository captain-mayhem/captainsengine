using System;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
  /// <summary>
  /// Summary description for RoomNode.
  /// </summary>
  public class StateGraph : Graph
  {
    public StateGraph()
    {
      m_type = Factory.ObjectType.StateGraph;
    }

    public StateGraph(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.StateGraph;
    }

    //public override GraphNode newNode(Point p)
    //{
    //  return new Room(p);
    //}

    public override object getVisited(GraphVisitor visitor)
    {
      return visitor.visit(this);
    }

    public override object getVisited(AdventureGraphVisitor visitor)
    {
      return visitor.visit(this);
    }
  }
}