using System;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphDrawVisitor.
	/// </summary>
	public class GraphDrawVisitor : GraphVisitor
	{
		public GraphDrawVisitor(Graphics graph, GraphNode selection)
		{
			//
			// TODO: Add constructor logic here
			//
      m_gc = graph;
      m_selected = selection;
		}

    //~GraphDrawVisitor(){
    //  m_gc.Dispose();
    //}

    public override object visit(Graph graph){
      foreach (GraphNode node in graph.Roots)
      {
        visit(node);
      }
      return null;
    }

    public override object visit(GraphEdge edge){
      Pen p = new Pen(Brushes.Black);
      p.EndCap = LineCap.ArrowAnchor;
      m_gc.DrawLine(p, edge.From.Location.X, 
        edge.From.Location.Y, 
        edge.To.Location.X, 
        edge.To.Location.Y);
      if (!edge.IsLoop)
        visit(edge.To);
      return null;
    }

    public override object visit(GraphNode node)
    {
      //draw node
      Pen p;
      if (node == m_selected){
        p = new Pen(Brushes.IndianRed,2.5f);
      }
      else
        p = new Pen(Brushes.BlueViolet,1.5f);
      m_gc.DrawEllipse(p, node.Location.X-node.Width/2, node.Location.Y-node.Height/2, node.Width, node.Height);
      
      //visit edges
      foreach (GraphEdge edge in node.Successors)
      {
        visit(edge);
      }
      return null;
    }

    private Graphics m_gc;
    private GraphNode m_selected;
	}
}
