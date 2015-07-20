using System;
using System.Collections;
using System.Drawing;
using System.IO;

namespace AdventureBuilder
{
  public delegate GraphNode GenNode(Point p);
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
      m_properties = new ArrayList();
      m_type = Factory.ObjectType.GraphNode;
      m_name = "Node";
      m_lower_graph = null;
		}

    public GraphNode(Point p) : this()
    {
      m_location = p;
      m_size = new Point(50,30);
    }

    public GraphNode(BinaryReader reader) : this() {
      m_location.X = reader.ReadInt32();
      m_location.Y = reader.ReadInt32();
      m_size.X = reader.ReadInt32();
      m_size.Y = reader.ReadInt32();
      m_name = reader.ReadString();
      m_type = Factory.ObjectType.GraphNode;
      int count = reader.ReadInt32();
      for (int i = 0; i < count; ++i){
        m_properties.Add(new ObjProperty(reader));
      }
      bool subgraph = reader.ReadBoolean();
      if (subgraph){
        //load subgraph
        //attention: currently, the parent member cannot be restored. 
        //It is re-set when needed by the GraphFrom
        m_lower_graph = (Graph)Factory.makeObject(reader);
      }
    }

    public void save(BinaryWriter writer){
      writer.Write((int)m_type);
      writer.Write(m_location.X);
      writer.Write(m_location.Y);
      writer.Write(m_size.X);
      writer.Write(m_size.Y);
      writer.Write(m_name);
      writer.Write(m_properties.Count);
      foreach (ObjProperty prop in m_properties)
      {
        prop.save(writer);
      }
      if (m_lower_graph == null){
        writer.Write(false);
      }
      else{
        writer.Write(true);
        m_lower_graph.save(writer);
      }
    }

    public GraphNode getRoot(){
      GraphNode current = this;
      while (current.Predecessors.Count > 0){
        GraphNode tmp = getFirstNonLoopingPredecessor(current);
        //it is a pseudo-root
        if (tmp == null)
          return current;
        current = tmp;
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

    virtual public object getVisited(GraphVisitor visitor)
    {
      return visitor.visit(this);
    }

    virtual public object getVisited(AdventureGraphVisitor visitor){
      return visitor.visit(this);
    }

    public override String ToString(){
      return m_name;
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

    public string Name{
      get{return m_name;}
      set{m_name = value;}
    }

    public ArrayList Properties{
      get{return m_properties;}
      set{m_properties = value;}
    }

    public Graph LowerGraph{
      get{return m_lower_graph;}
      set{m_lower_graph = value;}
    }

    private ArrayList m_succs;
    private ArrayList m_preds;
    private Point m_location;
    private Point m_size;
    protected Factory.ObjectType m_type;
    protected string m_name;
    protected ArrayList m_properties;
    private Graph m_lower_graph;
	}
}
