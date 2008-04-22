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
    //! is the left mouse button held down
    protected bool m_left_down;

    public Graph UsedGraph{
      set{m_graph = value;}
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

      //Monitor.Enter(m_graph);
      m_graph = new Graph();
      //Monitor.Exit(m_graph);
      m_left_down = false;

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
      // 
      // GraphForm
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(480, 382);
      this.Name = "GraphForm";
      this.Text = "GraphForm";

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
        Monitor.Enter(m_graph);
        GraphNodeSelectionVisitor gnsv = new GraphNodeSelectionVisitor(new Point(e.X,e.Y));
        m_selection = (GraphNode)m_graph.getVisited(gnsv);
        if (m_selection != null)
        {
          //it was clicked on a node
          m_left_down = true;
        }
        else
        {
          GraphNode node = m_graph.newNode(new Point(e.X,e.Y));
          m_graph.addUnconnectedNode(node);
        }
        Monitor.Exit(m_graph);
      }
      Invalidate();
    }

    private void GraphForm_Paint(object sender, PaintEventArgs e)
    {
      Monitor.Enter(m_graph);
      GraphDrawVisitor gdv = new GraphDrawVisitor(e.Graphics, m_selection);
      m_graph.getVisited(gdv);
      Monitor.Exit(m_graph);
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
  }
}
