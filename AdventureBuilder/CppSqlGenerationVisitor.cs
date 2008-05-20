using System;
using Finisar.SQLite;
using System.Windows.Forms;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphDrawVisitor.
	/// </summary>
	public class CppSqlGenerationVisitor : AdventureGraphVisitor, IDisposable
	{
		public CppSqlGenerationVisitor(Settings sett, Graph rooms)
		{
      m_settings = sett;
      m_graph = rooms;
      //System.Diagnostics.Process proc = new System.Diagnostics.Process();
      //proc.StartInfo.FileName = "sqlite.exe";
      m_conn = new SQLiteConnection("Data Source=adventure.hac;Version=3;New=True;Compress=True");
      m_conn.Open();
      m_cmd = m_conn.CreateCommand();
      m_propid = -1;
      m_statpropid = -1;
      m_respid = 0;
		}

    ~CppSqlGenerationVisitor(){
      m_conn.Close();
    }

    #region IDisposable Members

    public void Dispose()
    {
      // TODO:  Add CppSqlGenerationVisitor.Dispose implementation
      m_conn.Close();
    }

    #endregion

    public override object visit(Graph graph){
      return null;
    }

    public override object visit(RoomGraph graph)
    {
      //flatten the graph for further processing
      m_flatter = new GraphFlattenVisitor();
      m_graph.getVisited(m_flatter);

      //initialize tables
      m_cmd.CommandText = "CREATE TABLE characters (chid INTEGER PRIMARY KEY, name VARCHAR(50), curr_room INTEGER);";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE rooms (rid INTEGER, chid INTEGER, name VARCHAR(50), PRIMARY KEY(rid,chid));";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE room_connections (frm INTEGER, too INTEGER, chid INTEGER, type INTEGER(1), PRIMARY KEY (frm, too, chid));";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE staticproperties (propid INTEGER PRIMARY KEY, rid INTEGER, objid INTEGER, property VARCHAR(50));";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE properties (propid INTEGER, chid INTEGER, rid INTEGER, objid INTEGER, property VARCHAR(50), value INTEGER, PRIMARY KEY (propid, chid));";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE responses (respid INTEGER PRIMARY KEY, propid INTEGER, value INTEGER, opcode INTEGER, argument1 TEXT, argument2 TEXT);";
      m_cmd.ExecuteNonQuery();

      //insert default character
      Room start = m_settings.StartRoom;
      if (start == null)
        MessageBox.Show("Error: No starting room set!");
      else
      {
        int idx = m_flatter.Nodes.IndexOf(start);
        m_cmd.CommandText = "INSERT INTO characters (chid, name, curr_room) VALUES (0, 'Default', "+idx+");";
        m_cmd.ExecuteNonQuery();
      }

      //visit graph
      foreach (GraphNode node in graph.Roots)
      {
        node.getVisited(this);
      }
      return null;
    }

    public override object visit(GraphEdge edge){
      int frm = m_flatter.Nodes.IndexOf(edge.From);
      int to = m_flatter.Nodes.IndexOf(edge.To);
      m_cmd.CommandText = "INSERT INTO room_connections (frm, too, chid, type) VALUES ("+frm+", "+to+",0 , 0);";
      m_cmd.ExecuteNonQuery();
      if (!edge.IsLoop  && edge.From == GraphNode.getFirstNonLoopingPredecessor(edge.To))
        edge.To.getVisited(this);
      return null;
    }

    public override object visit(Room room){
      int rid = m_flatter.Nodes.IndexOf(room);
      String name = room.Name;
      m_cmd.CommandText = "INSERT INTO rooms (rid, chid, name) VALUES ("+rid+", 1,'"+name+"');";
      m_cmd.ExecuteNonQuery();
      foreach (ObjProperty prop in room.Properties)
      {
        //we have values, but no responses
        if (prop.Values.Count != 0 && ((ObjValue)prop.Values[0]).Responses.Count != 0)
        {
          //static property
          ++m_statpropid;
          m_cmd.CommandText = "INSERT INTO staticproperties (propid, rid, objid, property) VALUES ("+m_statpropid+", "+rid+", NULL, '"+prop.Name+"');";
          m_cmd.ExecuteNonQuery();
        }
        foreach (ObjValue val in prop.Values)
        {
          string value = Convert.ToString(val.Value);
          if (val.Responses.Count == 0)
          {
            //dynamic property
            ++m_propid;
            m_cmd.CommandText = "INSERT INTO properties (propid, chid, rid, objid, property, value) VALUES ("+m_propid+", 0, "+rid+", NULL, '"+prop.Name+"', "+value+");";
            m_cmd.ExecuteNonQuery();
          }
          foreach (ObjResponse resp in val.Responses)
          {
            m_cmd.CommandText = "INSERT INTO responses (respid, propid, value, opcode, argument1, argument2) VALUES ("+m_respid+", "+m_statpropid+", "+value+", "+Convert.ToString((int)resp.Operation)+", '"+(string)resp.Arguments[0]+"', '"+(string)resp.Arguments[1]+"');";
            m_cmd.ExecuteNonQuery();
            ++m_respid;
          }
        }
      }
      //visit edges
      foreach (GraphEdge edge in room.Successors)
      {
        edge.getVisited(this);
      }
      return null;
    }

    public override object visit(GraphNode node)
    {
      return null;
    }

    private Settings m_settings;
    private Graph m_graph;
    private SQLiteConnection m_conn;
    private SQLiteCommand m_cmd;
    private GraphFlattenVisitor m_flatter;
    private int m_statpropid;
    private int m_propid;
    private int m_respid;
  }
}
