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
            this.panel1.AccessibleDescription = null;
            this.panel1.AccessibleName = null;
            resources.ApplyResources(this.panel1, "panel1");
            this.panel1.BackgroundImage = null;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.hideAll);
            this.panel1.Controls.Add(this.showAll);
            this.panel1.Controls.Add(this.gp_newFolder);
            this.panel1.Controls.Add(this.gamepool_delete);
            this.panel1.Controls.Add(this.gamepool_add);
            this.panel1.Controls.Add(this.mediaPool);
            this.panel1.Controls.Add(this.gamePool);
            this.panel1.Font = null;
            this.panel1.Name = "panel1";
            // 
            // hideAll
            // 
            this.hideAll.AccessibleDescription = null;
            this.hideAll.AccessibleName = null;
            resources.ApplyResources(this.hideAll, "hideAll");
            this.hideAll.BackgroundImage = null;
            this.hideAll.Font = null;
            this.hideAll.Name = "hideAll";
            this.hideAll.UseVisualStyleBackColor = true;
            this.hideAll.Click += new System.EventHandler(this.hideAll_Click);
            // 
            // showAll
            // 
            this.showAll.AccessibleDescription = null;
            this.showAll.AccessibleName = null;
            resources.ApplyResources(this.showAll, "showAll");
            this.showAll.BackgroundImage = null;
            this.showAll.Font = null;
            this.showAll.Name = "showAll";
            this.showAll.UseVisualStyleBackColor = true;
            this.showAll.Click += new System.EventHandler(this.button3_Click);
            // 
            // gp_newFolder
            // 
            this.gp_newFolder.AccessibleDescription = null;
            this.gp_newFolder.AccessibleName = null;
            resources.ApplyResources(this.gp_newFolder, "gp_newFolder");
            this.gp_newFolder.BackgroundImage = null;
            this.gp_newFolder.Font = null;
            this.gp_newFolder.Name = "gp_newFolder";
            this.gp_newFolder.UseVisualStyleBackColor = true;
            this.gp_newFolder.Click += new System.EventHandler(this.gp_newFolder_Click);
            // 
            // gamepool_delete
            // 
            this.gamepool_delete.AccessibleDescription = null;
            this.gamepool_delete.AccessibleName = null;
            resources.ApplyResources(this.gamepool_delete, "gamepool_delete");
            this.gamepool_delete.BackgroundImage = null;
            this.gamepool_delete.Font = null;
            this.gamepool_delete.Name = "gamepool_delete";
            this.gamepool_delete.UseVisualStyleBackColor = true;
            this.gamepool_delete.Click += new System.EventHandler(this.gamepool_delete_Click);
            // 
            // gamepool_add
            // 
            this.gamepool_add.AccessibleDescription = null;
            this.gamepool_add.AccessibleName = null;
            resources.ApplyResources(this.gamepool_add, "gamepool_add");
            this.gamepool_add.BackgroundImage = null;
            this.gamepool_add.Font = null;
            this.gamepool_add.Name = "gamepool_add";
            this.gamepool_add.UseVisualStyleBackColor = true;
            this.gamepool_add.Click += new System.EventHandler(this.gamepool_add_Click);
            // 
            // mediaPool
            // 
            this.mediaPool.AccessibleDescription = null;
            this.mediaPool.AccessibleName = null;
            this.mediaPool.AllowDrop = true;
            resources.ApplyResources(this.mediaPool, "mediaPool");
            this.mediaPool.BackgroundImage = null;
            this.mediaPool.Font = null;
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
            this.gamePool.AccessibleDescription = null;
            this.gamePool.AccessibleName = null;
            this.gamePool.AllowDrop = true;
            resources.ApplyResources(this.gamePool, "gamePool");
            this.gamePool.BackgroundImage = null;
            this.gamePool.Font = null;
            this.gamePool.ImageList = this.iconList;
            this.gamePool.Name = "gamePool";
            // 
            // menuPool
            // 
            this.menuPool.AccessibleDescription = null;
            this.menuPool.AccessibleName = null;
            resources.ApplyResources(this.menuPool, "menuPool");
            this.menuPool.BackgroundImage = null;
            this.menuPool.Font = null;
            this.menuPool.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.renameToolStripMenuItem,
            this.newSubdirectoryToolStripMenuItem,
            this.duplicateToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.exportToolStripMenuItem1});
            this.menuPool.Name = "menuPool";
            // 
            // renameToolStripMenuItem
            // 
            this.renameToolStripMenuItem.AccessibleDescription = null;
            this.renameToolStripMenuItem.AccessibleName = null;
            resources.ApplyResources(this.renameToolStripMenuItem, "renameToolStripMenuItem");
            this.renameToolStripMenuItem.BackgroundImage = null;
            this.renameToolStripMenuItem.Name = "renameToolStripMenuItem";
            this.renameToolStripMenuItem.ShortcutKeyDisplayString = null;
            this.renameToolStripMenuItem.Click += new System.EventHandler(this.renameToolStripMenuItem_Click);
            // 
            // newSubdirectoryToolStripMenuItem
            // 
            this.newSubdirectoryToolStripMenuItem.AccessibleDescription = null;
            this.newSubdirectoryToolStripMenuItem.AccessibleName = null;
            resources.ApplyResources(this.newSubdirectoryToolStripMenuItem, "newSubdirectoryToolStripMenuItem");
            this.newSubdirectoryToolStripMenuItem.BackgroundImage = null;
            this.newSubdirectoryToolStripMenuItem.Name = "newSubdirectoryToolStripMenuItem";
            this.newSubdirectoryToolStripMenuItem.ShortcutKeyDisplayString = null;
            this.newSubdirectoryToolStripMenuItem.Click += new System.EventHandler(this.newSubdirectoryToolStripMenuItem_Click);
            // 
            // duplicateToolStripMenuItem
            // 
            this.duplicateToolStripMenuItem.AccessibleDescription = null;
            this.duplicateToolStripMenuItem.AccessibleName = null;
            resources.ApplyResources(this.duplicateToolStripMenuItem, "duplicateToolStripMenuItem");
            this.duplicateToolStripMenuItem.BackgroundImage = null;
            this.duplicateToolStripMenuItem.Name = "duplicateToolStripMenuItem";
            this.duplicateToolStripMenuItem.ShortcutKeyDisplayString = null;
            this.duplicateToolStripMenuItem.Click += new System.EventHandler(this.duplicateToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.AccessibleDescription = null;
            this.deleteToolStripMenuItem.AccessibleName = null;
            resources.ApplyResources(this.deleteToolStripMenuItem, "deleteToolStripMenuItem");
            this.deleteToolStripMenuItem.BackgroundImage = null;
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.ShortcutKeyDisplayString = null;
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // exportToolStripMenuItem1
            // 
            this.exportToolStripMenuItem1.AccessibleDescription = null;
            this.exportToolStripMenuItem1.AccessibleName = null;
            resources.ApplyResources(this.exportToolStripMenuItem1, "exportToolStripMenuItem1");
            this.exportToolStripMenuItem1.BackgroundImage = null;
            this.exportToolStripMenuItem1.Name = "exportToolStripMenuItem1";
            this.exportToolStripMenuItem1.ShortcutKeyDisplayString = null;
            // 
            // MainForm
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            this.AllowDrop = true;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.ControlBox = false;
            this.Controls.Add(this.panel1);
            this.DoubleBuffered = true;
            this.Font = null;
            this.ForeColor = System.Drawing.SystemColors.ControlText;
            this.Icon = null;
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

