using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for CppCompiler.
	/// </summary>
	public class CppCompiler : System.Windows.Forms.Form
	{
    private System.Windows.Forms.Button start;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
    private Graph m_graph;
    private Settings m_settings;

		public CppCompiler(Settings sett, Graph graph)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
      m_settings = sett;
      m_graph = graph;
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
      this.start = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // start
      // 
      this.start.Location = new System.Drawing.Point(32, 224);
      this.start.Name = "start";
      this.start.TabIndex = 0;
      this.start.Text = "Start";
      this.start.Click += new System.EventHandler(this.start_Click);
      // 
      // CppCompiler
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(292, 266);
      this.Controls.Add(this.start);
      this.Name = "CppCompiler";
      this.Text = "CppCompiler";
      this.ResumeLayout(false);

    }
		#endregion

    private void start_Click(object sender, System.EventArgs e)
    {
      CppSqlGenerationVisitor csgv = new CppSqlGenerationVisitor(m_settings, m_graph);
      m_graph.getVisited(csgv);
      csgv.Dispose();
    }
	}
}
