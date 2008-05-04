using System;
using Finisar.SQLite;
using System.Windows.Forms;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphDrawVisitor.
	/// </summary>
	public class CppSqlGenerationVisitor : GraphVisitor, IDisposable
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

    public override object visit(Graph graph)
    {
      //flatten the graph for further processing
      m_flatter = new GraphFlattenVisitor();
      m_graph.getVisited(m_flatter);

      //initialize tables
      m_cmd.CommandText = "CREATE TABLE characters (chid INTEGER PRIMARY KEY, name VARCHAR(50), curr_room INTEGER);";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE rooms (rid INTEGER, chid INTEGER, name VARCHAR(50), PRIMARY KEY(rid,chid));";
      m_cmd.ExecuteNonQuery();

      m_cmd.CommandText = "CREATE TABLE room_connections (frm INTEGER, too INTEGER, type INTEGER(1), PRIMARY KEY (frm, too));";
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
        visit(node);
      }
      return null;
    }

    public override object visit(GraphEdge edge){
      int frm = m_flatter.Nodes.IndexOf(edge.From);
      int to = m_flatter.Nodes.IndexOf(edge.To);
      m_cmd.CommandText = "INSERT INTO room_connections (frm, too, type) VALUES ("+frm+", "+to+", 0);";
      m_cmd.ExecuteNonQuery();
      if (!edge.IsLoop  && edge.From == GraphNode.getFirstNonLoopingPredecessor(edge.To))
        visit(edge.To);
      return null;
    }

    public override object visit(GraphNode node)
    {
      int rid = m_flatter.Nodes.IndexOf(node);
      String name = node.Name;
      m_cmd.CommandText = "INSERT INTO rooms (rid, chid, name) VALUES ("+rid+", 1,'"+name+"');";
      m_cmd.ExecuteNonQuery();
      //visit edges
      foreach (GraphEdge edge in node.Successors)
      {
        visit(edge);
      }
      return null;
    }

    private Settings m_settings;
    private Graph m_graph;
    private SQLiteConnection m_conn;
    private SQLiteCommand m_cmd;
    private GraphFlattenVisitor m_flatter;
  }
}
