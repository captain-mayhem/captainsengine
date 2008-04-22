using System;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphEdge.
	/// </summary>
	public class GraphEdge
	{
		public GraphEdge(GraphNode from, GraphNode to)
		{
			//
			// TODO: Add constructor logic here
			//
      m_from = from;
      m_to = to;
      if (to.Successors.Count > 0)
        m_loop = true;
      else
        m_loop = false;
		}

    public GraphEdge(BinaryReader reader){
      m_from = null;
      m_to = null;
      m_loop = reader.ReadBoolean();
    }

    public void save(BinaryWriter writer){
      writer.Write(m_loop);
    }

    public GraphNode From{
      set{m_from = value;}
      get{return m_from;}
    }

    public GraphNode To{
      set{m_to = value;}
      get{return m_to;}
    }

    public bool IsLoop{
      //set{m_loop = value;}
      get{return m_loop;}
    }

    private GraphNode m_from;
    private GraphNode m_to;
    private bool m_loop;
	}
}
