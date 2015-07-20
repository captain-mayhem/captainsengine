namespace StoryDesigner
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.panel1 = new System.Windows.Forms.Panel();
            this.hideAll = new System.Windows.Forms.Button();
            this.showAll = new System.Windows.Forms.Button();
            this.gp_newFolder = new System.Windows.Forms.Button();
            this.gamepool_delete = new System.Windows.Forms.Button();
            this.gamepool_add = new System.Windows.Forms.Button();
            this.mediaPool = new System.Windows.Forms.TreeView();
            this.iconList = new System.Windows.Forms.ImageList(this.components);
            this.gamePool = new System.Windows.Forms.TreeView();
            this.menuPool = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.renameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newSubdirectoryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.duplicateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.panel1.SuspendLayout();
            this.menuPool.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            resources.ApplyResources(this.panel1, "panel1");
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.hideAll);
            this.panel1.Controls.Add(this.showAll);
            this.panel1.Controls.Add(this.gp_newFolder);
            this.panel1.Controls.Add(this.gamepool_delete);
            this.panel1.Controls.Add(this.gamepool_add);
            this.panel1.Controls.Add(this.mediaPool);
            this.panel1.Controls.Add(this.gamePool);
            this.panel1.Name = "panel1";
            // 
            // hideAll
            // 
            resources.ApplyResources(this.hideAll, "hideAll");
            this.hideAll.Name = "hideAll";
            this.hideAll.UseVisualStyleBackColor = true;
            this.hideAll.Click += new System.EventHandler(this.hideAll_Click);
            // 
            // showAll
            // 
            resources.ApplyResources(this.showAll, "showAll");
            this.showAll.Name = "showAll";
            this.showAll.UseVisualStyleBackColor = true;
            this.showAll.Click += new System.EventHandler(this.button3_Click);
            // 
            // gp_newFolder
            // 
            resources.ApplyResources(this.gp_newFolder, "gp_newFolder");
            this.gp_newFolder.Name = "gp_newFolder";
            this.gp_newFolder.UseVisualStyleBackColor = true;
            this.gp_newFolder.Click += new System.EventHandler(this.gp_newFolder_Click);
            // 
            // gamepool_delete
            // 
            resources.ApplyResources(this.gamepool_delete, "gamepool_delete");
            this.gamepool_delete.Name = "gamepool_delete";
            this.gamepool_delete.UseVisualStyleBackColor = true;
            this.gamepool_delete.Click += new System.EventHandler(this.gamepool_delete_Click);
            // 
            // gamepool_add
            // 
            resources.ApplyResources(this.gamepool_add, "gamepool_add");
            this.gamepool_add.Name = "gamepool_add";
            this.gamepool_add.UseVisualStyleBackColor = true;
            this.gamepool_add.Click += new System.EventHandler(this.gamepool_add_Click);
            // 
            // mediaPool
            // 
            this.mediaPool.AllowDrop = true;
            resources.ApplyResources(this.mediaPool, "mediaPool");
            this.mediaPool.ImageList = this.iconList;
            this.mediaPool.Name = "mediaPool";
            // 
            // iconList
            // 
            this.iconList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("iconList.ImageStream")));
            this.iconList.TransparentColor = System.Drawing.Color.Transparent;
            this.iconList.Images.SetKeyName(0, "Empty.ico");
            this.iconList.Images.SetKeyName(1, "Folder.ico");
            // 
            // gamePool
            // 
            this.gamePool.AllowDrop = true;
            resources.ApplyResources(this.gamePool, "gamePool");
            this.gamePool.ImageList = this.iconList;
            this.gamePool.Name = "gamePool";
            // 
            // menuPool
            // 
            this.menuPool.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuPool.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.renameToolStripMenuItem,
            this.newSubdirectoryToolStripMenuItem,
            this.duplicateToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.exportToolStripMenuItem1});
            this.menuPool.Name = "menuPool";
            resources.ApplyResources(this.menuPool, "menuPool");
            // 
            // renameToolStripMenuItem
            // 
            this.renameToolStripMenuItem.Name = "renameToolStripMenuItem";
            resources.ApplyResources(this.renameToolStripMenuItem, "renameToolStripMenuItem");
            this.renameToolStripMenuItem.Click += new System.EventHandler(this.renameToolStripMenuItem_Click);
            // 
            // newSubdirectoryToolStripMenuItem
            // 
            this.newSubdirectoryToolStripMenuItem.Name = "newSubdirectoryToolStripMenuItem";
            resources.ApplyResources(this.newSubdirectoryToolStripMenuItem, "newSubdirectoryToolStripMenuItem");
            this.newSubdirectoryToolStripMenuItem.Click += new System.EventHandler(this.newSubdirectoryToolStripMenuItem_Click);
            // 
            // duplicateToolStripMenuItem
            // 
            this.duplicateToolStripMenuItem.Name = "duplicateToolStripMenuItem";
            resources.ApplyResources(this.duplicateToolStripMenuItem, "duplicateToolStripMenuItem");
            this.duplicateToolStripMenuItem.Click += new System.EventHandler(this.duplicateToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            resources.ApplyResources(this.deleteToolStripMenuItem, "deleteToolStripMenuItem");
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // exportToolStripMenuItem1
            // 
            this.exportToolStripMenuItem1.Name = "exportToolStripMenuItem1";
            resources.ApplyResources(this.exportToolStripMenuItem1, "exportToolStripMenuItem1");
            // 
            // MainForm
            // 
            this.AllowDrop = true;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ControlBox = false;
            this.Controls.Add(this.panel1);
            this.DoubleBuffered = true;
            this.ForeColor = System.Drawing.SystemColors.ControlText;
            this.Name = "MainForm";
            this.panel1.ResumeLayout(false);
            this.menuPool.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TreeView gamePool;
        private System.Windows.Forms.TreeView mediaPool;
        private System.Windows.Forms.Button gamepool_add;
        private System.Windows.Forms.ContextMenuStrip menuPool;
        private System.Windows.Forms.ToolStripMenuItem renameToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newSubdirectoryToolStripMenuItem;
        private System.Windows.Forms.ImageList iconList;
        private System.Windows.Forms.Button gamepool_delete;
        private System.Windows.Forms.Button hideAll;
        private System.Windows.Forms.Button showAll;
        private System.Windows.Forms.Button gp_newFolder;
        private System.Windows.Forms.ToolStripMenuItem duplicateToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportToolStripMenuItem1;
    }
}

