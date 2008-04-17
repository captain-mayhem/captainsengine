using System;
using System.Collections;
using System.Drawing;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphNode.
	/// </summary>
	public class GraphNode
	{
		public GraphNode()
		{
			//
			// TODO: Add constructor logic here
			//
      m_location = new Point(0,0);
      m_size = new Point(0,0);
      m_succs = new ArrayList();
      m_preds = new ArrayList();
		}

    public GraphNode(Point p){
      m_location = p;
      m_size = new Point(50,30);
      m_succs = new ArrayList();
      m_preds = new ArrayList();
    }

    public Point Location{
      get{return m_location;}
      set{m_location = value;}
    }

    public int Width{
      get{return m_size.X;}
    }

    public int Height{
      get{return m_size.Y;}
    }

    public ArrayList Successors{
      get{return m_succs;}
    }

    public ArrayList Predecessors{
      get{return m_preds;}
    }

    private ArrayList m_succs;
    private ArrayList m_preds;
    private Point m_location;
    private Point m_size;
	}
}
