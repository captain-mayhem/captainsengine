using System;
using System.Collections;

namespace AdventureBuilder
{
  /// <summary>
  /// Summary description for GraphFlattenVisitor.
  /// </summary>
  public class GraphFlattenVisitor : GraphVisitor
  {
    public GraphFlattenVisitor()
    {
      m_nodes = new ArrayList();
      m_edges = new ArrayList();
    }

    public override object visit(Graph graph)
    {
      foreach (GraphNode node in graph.Roots)
      {
        node.getVisited(this);
      }
      return null;
    }

    public override object visit(RoomGraph graph){
      return visit((Graph)graph);
    }

    public override object visit(GraphEdge edge)
    {
      m_edges.Add(edge);
      //not looping and not second pass
      if (!edge.IsLoop && edge.From == GraphNode.getFirstNonLoopingPredecessor(edge.To))
      {
        edge.To.getVisited(this);
      }
      return null;
    }

    public override object visit(GraphNode node)
    {
      m_nodes.Add(node);
      foreach (GraphEdge edge in node.Successors)
      {
        edge.getVisited(this);
      }
      return null;
    }

    public override object visit(Room room){
      return visit((GraphNode)room);
    }

    public ArrayList Nodes{
      get{return m_nodes;}
    }

    public ArrayList Edges{
      get{return m_edges;}
    }

    protected ArrayList m_nodes;
    protected ArrayList m_edges;
  }
}
