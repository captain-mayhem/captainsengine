using System;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for AdventureGraphVisitor.
	/// </summary>
	public abstract class AdventureGraphVisitor
	{
		public AdventureGraphVisitor()
		{
			//
			// TODO: Add constructor logic here
			//
		}

    public abstract object visit(Graph graph);
    public abstract object visit(GraphNode node);
    public abstract object visit(GraphEdge edge);
    public abstract object visit(Room room);
    public abstract object visit(RoomGraph graph);
	}
}
