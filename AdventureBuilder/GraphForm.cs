using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for GraphForm.
	/// </summary>
	public class GraphForm : System.Windows.Forms.Form
	{
    protected Graph m_graph;
    protected GraphNode m_selection;
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

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
      m_graph = new Graph();
      this.MouseDown +=new MouseEventHandler(GraphForm_MouseDown);
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
      if (e.Button != MouseButtons.Left)
        return;
      GraphNodeSelectionVisitor gnsv = new GraphNodeSelectionVisitor(new Point(e.X,e.Y));
      m_selection = (GraphNode)m_graph.getVisited(gnsv);
      if (m_selection != null)
      {
        //it was clicked on a node
      }
      else
      {
        GraphNode node = new GraphNode(new Point(e.X,e.Y));
        m_graph.addUnconnectedNode(node);
      }
      Invalidate();
    }

    private void GraphForm_Paint(object sender, PaintEventArgs e)
    {
      GraphDrawVisitor gdv = new GraphDrawVisitor(e.Graphics, m_selection);
      m_graph.getVisited(gdv);
    }
	}
}
