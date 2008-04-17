using System;
using System.Collections;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Graph.
	/// </summary>
	public class Graph
	{
		public Graph()
		{
      m_roots = new ArrayList();
		}

    public void addUnconnectedNode(GraphNode node){
      m_roots.Add(node);
    }

    public object getVisited(GraphVisitor visitor){
      return visitor.visit(this);
    }

    public ArrayList Roots{
      get{return m_roots;}
    }

    private ArrayList m_roots;
	}
}
