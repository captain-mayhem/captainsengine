using System;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for MultipleChoiceNode.
	/// </summary>
  public class MultipleChoiceNode : GraphNode
  {
    public MultipleChoiceNode(Point p) : base(p)
    {
      m_type = Factory.ObjectType.MultipleChoiceNode;
      m_name = "Auswahl";
    }

    public MultipleChoiceNode(BinaryReader reader) : base(reader)
    {
      m_type = Factory.ObjectType.MultipleChoiceNode;
    }

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
