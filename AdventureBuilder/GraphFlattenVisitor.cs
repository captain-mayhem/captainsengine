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
        visit(node);
      }
      return null;
    }

    public override object visit(GraphEdge edge)
    {
      m_edges.Add(edge);
      if (!edge.IsLoop)
      {
        visit(edge.To);
      }
      return null;
    }

    public override object visit(GraphNode node)
    {
      m_nodes.Add(node);
      foreach (GraphEdge edge in node.Successors)
      {
        visit(edge);
      }
      return null;
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
