using System;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphVisitor.
	/// </summary>
	public abstract class GraphVisitor
	{
		public GraphVisitor()
		{
			//
			// TODO: Add constructor logic here
			//
		}

    public abstract object visit(Graph graph);
    public abstract object visit(GraphNode node);
    public abstract object visit(GraphEdge edge);
	}
}
