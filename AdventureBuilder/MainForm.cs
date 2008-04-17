using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace AdventureBuilder
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class MainForm : GraphForm
	{
    private System.Windows.Forms.MainMenu mainMenu1;
    private System.Windows.Forms.MenuItem menu_new;
    private System.Windows.Forms.MenuItem menu_exit;
    private System.Windows.Forms.MenuItem menu_file;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public MainForm()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
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
      this.mainMenu1 = new System.Windows.Forms.MainMenu();
      this.menu_file = new System.Windows.Forms.MenuItem();
      this.menu_new = new System.Windows.Forms.MenuItem();
      this.menu_exit = new System.Windows.Forms.MenuItem();
      // 
      // mainMenu1
      // 
      this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.menu_file});
      // 
      // menu_file
      // 
      this.menu_file.Index = 0;
      this.menu_file.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.menu_new,
                                                                              this.menu_exit});
      this.menu_file.Text = "File";
      // 
      // menu_new
      // 
      this.menu_new.Index = 0;
      this.menu_new.Text = "New";
      this.menu_new.Click += new System.EventHandler(this.menu_new_Click);
      // 
      // menu_exit
      // 
      this.menu_exit.Index = 1;
      this.menu_exit.Text = "Exit";
      this.menu_exit.Click += new System.EventHandler(this.menu_exit_Click);
      // 
      // MainForm
      // 
      this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
      this.ClientSize = new System.Drawing.Size(1016, 713);
      this.Menu = this.mainMenu1;
      this.Name = "MainForm";
      this.Text = "Adventure Builder";
      this.Load += new System.EventHandler(this.Form1_Load);

    }
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new MainForm());
		}

    private void menu_exit_Click(object sender, System.EventArgs e)
    {
      Application.Exit();
    }

    private void Form1_Load(object sender, System.EventArgs e)
    {
    
    }

    private void menu_new_Click(object sender, System.EventArgs e)
    {
      m_graph.Roots.Clear();
      Invalidate();
    }
  }
}
