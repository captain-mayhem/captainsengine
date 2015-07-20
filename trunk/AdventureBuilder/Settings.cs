using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Settings.
	/// </summary>
	public class Settings : System.Windows.Forms.Form, ISavable
	{
    private System.Windows.Forms.ComboBox startingroom;
    private System.Windows.Forms.Label label1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
    private RoomGraph m_graph;
    private Room m_startroom;
    protected Factory.ObjectType m_type;
    private int m_startroomindex;

		public Settings(RoomGraph graph)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			m_graph = graph;
      m_startroom = null;
      m_type = Factory.ObjectType.Settings;
      m_startroomindex = -1;
		}

    public Settings(BinaryReader reader) : this((RoomGraph)null){
      m_type = Factory.ObjectType.Settings;
      bool start_room_saved = reader.ReadBoolean();
      if (start_room_saved)
      {
        m_startroomindex = reader.ReadInt32();
      }
    }

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
      this.startingroom = new System.Windows.Forms.ComboBox();
      this.label1 = new System.Windows.Forms.Label();
      this.SuspendLayout();
      // 
      // startingroom
      // 
      this.startingroom.Location = new System.Drawing.Point(88, 16);
      this.startingroom.Name = "startingroom";
      this.startingroom.Size = new System.Drawing.Size(192, 21);
      this.startingroom.TabIndex = 0;
      this.startingroom.SelectedIndexChanged +=new EventHandler(startingroom_SelectedIndexChanged);
      // 
      // label1
      // 
      this.label1.Location = new System.Drawing.Point(8, 16);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(80, 16);
      this.label1.TabIndex = 1;
      this.label1.Text = "Starting Room:";
      // 
      // Settings
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(792, 566);
      this.Controls.Add(this.label1);
      this.Controls.Add(this.startingroom);
      this.Name = "Settings";
      this.Text = "Settings";
      this.ResumeLayout(false);

    }
		#endregion

    public void synchronize()
    {
      GraphFlattenVisitor gfv = new GraphFlattenVisitor();
      m_graph.getVisited(gfv);
      startingroom.Items.Clear();
      foreach (GraphNode node in gfv.Nodes)
      {
        startingroom.Items.Add(node);
      }
      startingroom.SelectedItem = m_startroom;
      if (m_startroom != null)
        startingroom.Text = m_startroom.Name;
    }

    private void startingroom_SelectedIndexChanged(object sender, EventArgs e)
    {
      m_startroom = (Room)((ComboBox)sender).SelectedItem;
    }

    #region ISavable Members

    public void save(System.IO.BinaryWriter writer)
    {
      writer.Write((int)m_type);
      if (m_startroom != null)
      {
        writer.Write(true);
        GraphFlattenVisitor gfv = new GraphFlattenVisitor();
        m_graph.getVisited(gfv);
        int idx = gfv.Nodes.IndexOf(m_startroom);
        writer.Write(idx);
      }
      else
        writer.Write(false);
    }

    #endregion

    public RoomGraph Graph{
      set
      {
        m_graph = value;
        if (m_startroomindex != -1)
        {
          GraphFlattenVisitor gfv = new GraphFlattenVisitor();
          m_graph.getVisited(gfv);
          m_startroom = (Room)gfv.Nodes[m_startroomindex];
          m_startroomindex = -1;
        }
      }
    }

    public Room StartRoom{
      get{return m_startroom;}
    }
  }
}
