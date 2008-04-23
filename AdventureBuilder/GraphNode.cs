using System;
using System.Collections;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphNode.
	/// </summary>
	public class GraphNode
	{
		public GraphNode()
		{
      m_location = new Point(0,0);
      m_size = new Point(0,0);
      m_succs = new ArrayList();
      m_preds = new ArrayList();
      m_type = Factory.ObjectType.GraphNode;
		}

    public GraphNode(Point p)
    {
      m_location = p;
      m_size = new Point(50,30);
      m_succs = new ArrayList();
      m_preds = new ArrayList();
      m_type = Factory.ObjectType.GraphNode;
    }

    public GraphNode(BinaryReader reader) : this() {
      m_location.X = reader.ReadInt32();
      m_location.Y = reader.ReadInt32();
      m_size.X = reader.ReadInt32();
      m_size.Y = reader.ReadInt32();
      m_type = Factory.ObjectType.GraphNode;
    }

    public void save(BinaryWriter writer){
      writer.Write((int)m_type);
      writer.Write(m_location.X);
      writer.Write(m_location.Y);
      writer.Write(m_size.X);
      writer.Write(m_size.Y);
    }

    public GraphNode getRoot(){
      GraphNode current = this;
      while (current.Predecessors.Count > 0){
        current = getFirstNonLoopingPredecessor(current);
        //it is a pseudo-root
        if (current == null)
          return current;
      }
      return current;
    }

    public static GraphNode getFirstNonLoopingPredecessor(GraphNode node){
      //get the non-looping predecessor
      for (int i = 0; i < node.Predecessors.Count; ++i)
      {
        GraphEdge edge = (GraphEdge)node.Predecessors[i];
        if (!edge.IsLoop)
        {
          return edge.From;
        }
      }
      return null;
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
    protected Factory.ObjectType m_type;
	}
}
