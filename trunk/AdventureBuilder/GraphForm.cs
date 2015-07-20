using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Threading;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphForm.
	/// </summary>
	public class GraphForm : System.Windows.Forms.Form
	{
    //! the graph
    protected Graph m_graph;
    //! which node was selected
    protected GraphNode m_selection;
    //! the clicked point
    protected Point m_click;
    //! is the left mouse button held down
    protected bool m_left_down;
    //! the node generation functions
    protected ArrayList m_node_func;
    //! the menu to generate nodes
    private System.Windows.Forms.ContextMenu NodeMenu;
    //! the current sublevel of the graph
    private System.Windows.Forms.Button graph_level;
    //! the subgraph generation function
    private GenGraph m_gen_graph;

    public Graph UsedGraph{
      set{m_graph = value;}
    }

    public GenGraph GraphGenFunc{
      set{m_gen_graph = value;}
    }

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public GraphForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

      m_graph = new Graph();
      m_left_down = false;
      m_node_func = new ArrayList();

      this.MouseDown +=new MouseEventHandler(GraphForm_MouseDown);
      this.MouseUp +=new MouseEventHandler(GraphForm_MouseUp);
      this.MouseMove +=new MouseEventHandler(GraphForm_MouseMove);
      this.Paint += new PaintEventHandler(GraphForm_Paint);
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
      this.NodeMenu = new System.Windows.Forms.ContextMenu();
      this.graph_level = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // NodeMenu
      // 
      this.NodeMenu.Popup += new System.EventHandler(this.NodeMenu_Popup);
      // 
      // graph_level
      // 
      this.graph_level.Location = new System.Drawing.Point(8, 8);
      this.graph_level.Name = "graph_level";
      this.graph_level.Size = new System.Drawing.Size(48, 24);
      this.graph_level.TabIndex = 0;
      this.graph_level.Text = "level 0";
      this.graph_level.Click += new System.EventHandler(this.graph_level_Click);
      // 
      // GraphForm
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(480, 382);
      this.Controls.Add(this.graph_level);
      this.Name = "GraphForm";
      this.Text = "GraphForm";
      this.ResumeLayout(false);

    }
		#endregion

    private void GraphForm_MouseDown(object sender, MouseEventArgs e)
    {
      if (e.Button == MouseButtons.Right)
      {
        if (m_selection == null)
          return;
        //connect request
        Monitor.Enter(m_graph);
        GraphNodeSelectionVisitor gnsv = new GraphNodeSelectionVisitor(new Point(e.X,e.Y));
        GraphNode target = (GraphNode)m_graph.getVisited(gnsv);
        if (target != null)
        {
          //it was clicked on a node
          m_graph.connect(m_selection, target);
        }
        else{
          m_selection = null;
        }
        Monitor.Exit(m_graph);
      }
      else if (e.Button == MouseButtons.Left)
      {
        GraphNodeSelectionVisitor gnsv = new GraphNodeSelectionVisitor(new Point(e.X,e.Y));
        m_selection = (GraphNode)m_graph.getVisited(gnsv);
        if (m_selection != null)
        {
          //it was clicked on a node
          if (e.Clicks > 1)
          {
            //double click
            if (m_selection.LowerGraph == null)
            {
              m_selection.LowerGraph = m_gen_graph();
            }
            m_selection.LowerGraph.Parent = m_graph;
            m_graph = m_selection.LowerGraph;
            string tmp = graph_level.Text.Substring(6);
            int level = Convert.ToInt32(tmp);
            graph_level.Text = "level "+Convert.ToString(level+1);
            //m_selection.Name = "dc";
          }
          m_left_down = true;
        }
        else
        {
          m_click = new Point(e.X, e.Y); 
          NodeMenu.Show(this, m_click);
        }
      }
      Invalidate();
    }

    private void GraphForm_Paint(object sender, PaintEventArgs e)
    {
      GraphDrawVisitor gdv = new GraphDrawVisitor(e.Graphics, m_selection);
      m_graph.getVisited(gdv);
    }

    private void GraphForm_MouseMove(object sender, MouseEventArgs e)
    {
      if (!m_left_down)
        return;
      m_selection.Location = new Point(e.X,e.Y);
      Invalidate();
    }

    private void GraphForm_MouseUp(object sender, MouseEventArgs e)
    {
      m_left_down = false;
    }

    private void NodeMenu_Popup(object sender, System.EventArgs e)
    {
    }

    private void nodeMenuNodeGen_Click(object sender, System.EventArgs e){
      MenuItem item = (MenuItem)sender;
      GenNode gen = (GenNode)m_node_func[item.Index];
      GraphNode node = gen(m_click);
      if (node == null)
        return;
      m_graph.addUnconnectedNode(node);
    }

    public void addNodeGeneration(string name, int idx, GenNode function){
      EventHandler ev = new System.EventHandler(nodeMenuNodeGen_Click);
      MenuItem item = new MenuItem(name, ev);
      item.Index = idx;
      NodeMenu.MenuItems.Add(idx, item);
      m_node_func.Insert(idx, function);
    }

    private void graph_level_Click(object sender, System.EventArgs e)
    {
      if (m_graph.Parent != null){
        m_graph = m_graph.Parent;
        string tmp = graph_level.Text.Substring(6);
        int level = Convert.ToInt32(tmp);
        graph_level.Text = "level "+Convert.ToString(level-1);
        Invalidate();
      }
    }
  }
}
