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
        node.getVisited(this);
      }
      return null;
    }

    public override object visit(GraphEdge edge){
      Pen p;
      if (edge.IsLoop)
        p = new Pen(Brushes.Green,2.0f);
      else
        p = new Pen(Brushes.Black);
      p.EndCap = LineCap.ArrowAnchor;
      Point p2 = intersectLineCircle(edge.From.Location, edge.To.Location, edge.To.Location, edge.To.Height/2);
      Point p1 = intersectLineCircle(edge.To.Location, edge.From.Location, edge.From.Location, edge.From.Height/2);
      m_gc.DrawLine(p, p1.X, 
        p1.Y, 
        p2.X, 
        p2.Y);
      if (!edge.IsLoop  && edge.From == GraphNode.getFirstNonLoopingPredecessor(edge.To))
        edge.To.getVisited(this);
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
      if (node.getRoot() == node)
        p.DashStyle = DashStyle.Dash;
      m_gc.DrawEllipse(p, node.Location.X-node.Width/2, node.Location.Y-node.Height/2, node.Width, node.Height);
      
      //with its text
      m_gc.DrawString(node.Name, new Font(FontFamily.GenericSerif, 8), 
        Brushes.DarkGreen, 
        node.Location.X-node.Name.Length*3, node.Location.Y-5);

      //visit edges
      foreach (GraphEdge edge in node.Successors)
      {
        edge.getVisited(this);
      }
      return null;
    }

    private Point intersectLineCircle(Point lineStart, Point lineEnd, Point center, int radius){
      Point o = lineStart;
      Point d = new Point(lineEnd.X - lineStart.X, lineEnd.Y-lineStart.Y);
      double length = Math.Sqrt(d.X*d.X+d.Y*d.Y);
      double dx = d.X/length;
      double dy = d.Y/length;
      Point c = center;
      double lx = c.X - o.X; double ly = c.Y - o.Y;
      double s = lx*dx+ly*dy;
      double l2 = lx*lx+ly*ly;
      double m2 = l2 - s*s;
      double r2 = radius*radius;
      double q = Math.Sqrt(r2-m2);
      double t;
      if (l2 > r2)
        t = s - q;
      else
        t = s + q;
      return new Point(o.X+(int)(t*dx), o.Y+(int)(t*dy));
    }

    private Graphics m_gc;
    private GraphNode m_selected;
	}
}
