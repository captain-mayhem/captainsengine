using System;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Factory.
	/// </summary>
	public class Factory
	{
    public enum ObjectType{
      Graph,
      GraphNode,
      GraphEdge,
      RoomGraph,
      Room,
      Settings,
      MultipleChoiceNode,
      StateGraph,
    }

		public Factory()
		{
			//
			// TODO: Add constructor logic here
			//
		}

    public static object makeObject(BinaryReader reader){
      ObjectType type = (ObjectType)reader.ReadInt32();
      return makeObject(type, reader);
    }

    private static object makeObject(ObjectType type, BinaryReader reader)
    {
      switch(type){
        case ObjectType.Graph:
          return new Graph(reader);
        case ObjectType.GraphEdge:
          return new GraphEdge(reader);
        case ObjectType.GraphNode:
          return new GraphNode(reader);
        case ObjectType.Room:
          return new Room(reader);
        case ObjectType.RoomGraph:
          return new RoomGraph(reader);
        case ObjectType.Settings:
          return new Settings(reader);
        case ObjectType.MultipleChoiceNode:
          return new MultipleChoiceNode(reader);
        case ObjectType.StateGraph:
          return new StateGraph(reader);
      }
      return null;
    }
	}
}
