using System;
using System.Drawing;

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

    public override object visit(GraphNode node)
    {
      Pen p;
      if (node == m_selected){
        p = new Pen(Brushes.IndianRed,3.0f);
      }
      else
        p = new Pen(Brushes.BlueViolet,1.5f);
      m_gc.DrawEllipse(p, node.Location.X-node.Width/2, node.Location.Y-node.Height/2, node.Width, node.Height);
      return null;
    }

    private Graphics m_gc;
    private GraphNode m_selected;
	}
}
