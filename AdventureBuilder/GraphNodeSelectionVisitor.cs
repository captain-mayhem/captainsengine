using System;
using System.Drawing;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphNodeSelectionVisitor.
	/// </summary>
	public class GraphNodeSelectionVisitor : GraphVisitor
	{
		public GraphNodeSelectionVisitor(Point hitpoint)
		{
      m_hitpoint = hitpoint;
		}

    public override object visit(Graph graph)
    {
      foreach (GraphNode node in graph.Roots)
      {
        object hit = node.getVisited(this);
        if (hit != null)
          return hit;
      }
      return null;
    }

    public override object visit(RoomGraph graph){
      return visit((Graph)graph);
    }

    public override object visit(GraphEdge edge)
    {
      if (edge.IsLoop || edge.From != GraphNode.getFirstNonLoopingPredecessor(edge.To))
        return null;
      object found = edge.To.getVisited(this);
      if (found != null)
        return found;
      return null;
    }

    public override object visit(GraphNode node)
    {
      //test against the box that surrounds the ellipsis
      if (node.Location.X - node.Width/2 <= m_hitpoint.X && m_hitpoint.X <= node.Location.X + node.Width/2){
        if (node.Location.Y - node.Height/2 <= m_hitpoint.Y && m_hitpoint.Y <= node.Location.Y + node.Height/2)
        {
          return node;
        }
      }
      foreach (GraphEdge edge in node.Successors)
      {
        object found = edge.getVisited(this);
        if (found != null)
          return found;
      }
      return null;
    }

    public override object visit(Room room){
      return visit((GraphNode)room);
    }

    private Point m_hitpoint;

	}
}
