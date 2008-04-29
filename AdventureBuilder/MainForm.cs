using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Threading;
using System.IO;

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
    private System.Windows.Forms.MenuItem menu_load;
    private System.Windows.Forms.MenuItem menu_save;
    private System.Windows.Forms.MenuItem menu_object;
    private System.Windows.Forms.MenuItem object_properties;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public MainForm() : base()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

      m_graph = new RoomGraph();
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
      this.menu_load = new System.Windows.Forms.MenuItem();
      this.menu_save = new System.Windows.Forms.MenuItem();
      this.menu_exit = new System.Windows.Forms.MenuItem();
      this.menu_object = new System.Windows.Forms.MenuItem();
      this.object_properties = new System.Windows.Forms.MenuItem();
      // 
      // mainMenu1
      // 
      this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.menu_file,
                                                                              this.menu_object});
      // 
      // menu_file
      // 
      this.menu_file.Index = 0;
      this.menu_file.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.menu_new,
                                                                              this.menu_load,
                                                                              this.menu_save,
                                                                              this.menu_exit});
      this.menu_file.Text = "File";
      // 
      // menu_new
      // 
      this.menu_new.Index = 0;
      this.menu_new.Text = "New";
      this.menu_new.Click += new System.EventHandler(this.menu_new_Click);
      // 
      // menu_load
      // 
      this.menu_load.Index = 1;
      this.menu_load.Text = "Load";
      this.menu_load.Click += new System.EventHandler(this.menu_load_Click);
      // 
      // menu_save
      // 
      this.menu_save.Index = 2;
      this.menu_save.Text = "Save";
      this.menu_save.Click += new System.EventHandler(this.menu_save_Click);
      // 
      // menu_exit
      // 
      this.menu_exit.Index = 3;
      this.menu_exit.Text = "Exit";
      this.menu_exit.Click += new System.EventHandler(this.menu_exit_Click);
      // 
      // menu_object
      // 
      this.menu_object.Index = 1;
      this.menu_object.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                this.object_properties});
      this.menu_object.Text = "Object";
      // 
      // object_properties
      // 
      this.object_properties.Index = 0;
      this.object_properties.Text = "Properties";
      this.object_properties.Click += new System.EventHandler(this.object_properties_Click);
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

    private void menu_save_Click(object sender, System.EventArgs e)
    {
      System.Windows.Forms.SaveFileDialog sfd = new System.Windows.Forms.SaveFileDialog();
      sfd.Title = "Save Adventure";
      sfd.Filter = "Adventure Builder Files (*.adv)|*.adv";
      sfd.ShowDialog(this);
      try
      {
        Stream strm = sfd.OpenFile();
        AdventurePersistance pers = new AdventurePersistance(strm);
        pers.save(m_graph);
        m_selection = null;
        m_left_down = false;
        strm.Close();
      }
      catch (System.Exception ex)
      {
      	Console.WriteLine(ex);
      }
    }

    private void menu_load_Click(object sender, System.EventArgs e)
    {
      System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
      ofd.Title = "Load Adventure";
      ofd.Filter = "Adventure Builder Files (*.adv)|*.adv";
      ofd.ShowDialog(this);
      try{
        Stream strm = ofd.OpenFile();
        AdventurePersistance pers = new AdventurePersistance(strm);
        m_graph = (RoomGraph)pers.load();
        strm.Close();
        Invalidate();
      }
      catch (System.Exception ex)
      {
      	Console.WriteLine(ex);
      }
    }

    private void object_properties_Click(object sender, System.EventArgs e)
    {
      ObjectProperties prop = new ObjectProperties(m_selection);
      if (m_selection != null)
      {
        prop.ShowDialog(this);
      }
      else
        System.Windows.Forms.MessageBox.Show(this, "A Node must be selected");
    }
  }
}
