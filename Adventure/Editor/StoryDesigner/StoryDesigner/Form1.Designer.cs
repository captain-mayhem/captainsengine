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
            this.panel1.AutoSize = true;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.hideAll);
            this.panel1.Controls.Add(this.showAll);
            this.panel1.Controls.Add(this.gp_newFolder);
            this.panel1.Controls.Add(this.gamepool_delete);
            this.panel1.Controls.Add(this.gamepool_add);
            this.panel1.Controls.Add(this.mediaPool);
            this.panel1.Controls.Add(this.gamePool);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(278, 562);
            this.panel1.TabIndex = 1;
            // 
            // hideAll
            // 
            this.hideAll.Location = new System.Drawing.Point(131, 262);
            this.hideAll.Name = "hideAll";
            this.hideAll.Size = new System.Drawing.Size(26, 23);
            this.hideAll.TabIndex = 6;
            this.hideAll.Text = "|--|";
            this.hideAll.UseVisualStyleBackColor = true;
            this.hideAll.Click += new System.EventHandler(this.hideAll_Click);
            // 
            // showAll
            // 
            this.showAll.Location = new System.Drawing.Point(99, 262);
            this.showAll.Name = "showAll";
            this.showAll.Size = new System.Drawing.Size(26, 23);
            this.showAll.TabIndex = 5;
            this.showAll.Text = "|+|";
            this.showAll.UseVisualStyleBackColor = true;
            this.showAll.Click += new System.EventHandler(this.button3_Click);
            // 
            // gp_newFolder
            // 
            this.gp_newFolder.Location = new System.Drawing.Point(67, 262);
            this.gp_newFolder.Name = "gp_newFolder";
            this.gp_newFolder.Size = new System.Drawing.Size(26, 23);
            this.gp_newFolder.TabIndex = 4;
            this.gp_newFolder.Text = "N";
            this.gp_newFolder.UseVisualStyleBackColor = true;
            this.gp_newFolder.Click += new System.EventHandler(this.gp_newFolder_Click);
            // 
            // gamepool_delete
            // 
            this.gamepool_delete.Location = new System.Drawing.Point(35, 262);
            this.gamepool_delete.Name = "gamepool_delete";
            this.gamepool_delete.Size = new System.Drawing.Size(26, 23);
            this.gamepool_delete.TabIndex = 3;
            this.gamepool_delete.Text = "-";
            this.gamepool_delete.UseVisualStyleBackColor = true;
            this.gamepool_delete.Click += new System.EventHandler(this.gamepool_delete_Click);
            // 
            // gamepool_add
            // 
            this.gamepool_add.Location = new System.Drawing.Point(3, 262);
            this.gamepool_add.Name = "gamepool_add";
            this.gamepool_add.Size = new System.Drawing.Size(26, 23);
            this.gamepool_add.TabIndex = 2;
            this.gamepool_add.Text = "+";
            this.gamepool_add.UseVisualStyleBackColor = true;
            this.gamepool_add.Click += new System.EventHandler(this.gamepool_add_Click);
            // 
            // mediaPool
            // 
            this.mediaPool.AllowDrop = true;
            this.mediaPool.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.mediaPool.ImageIndex = 0;
            this.mediaPool.ImageList = this.iconList;
            this.mediaPool.Location = new System.Drawing.Point(0, 291);
            this.mediaPool.Name = "mediaPool";
            this.mediaPool.SelectedImageIndex = 0;
            this.mediaPool.Size = new System.Drawing.Size(276, 269);
            this.mediaPool.TabIndex = 1;
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
            this.gamePool.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.gamePool.ImageIndex = 0;
            this.gamePool.ImageList = this.iconList;
            this.gamePool.Location = new System.Drawing.Point(0, 0);
            this.gamePool.Name = "gamePool";
            this.gamePool.SelectedImageIndex = 0;
            this.gamePool.Size = new System.Drawing.Size(276, 256);
            this.gamePool.TabIndex = 0;
            // 
            // menuPool
            // 
            this.menuPool.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.renameToolStripMenuItem,
            this.newSubdirectoryToolStripMenuItem,
            this.duplicateToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.exportToolStripMenuItem1});
            this.menuPool.Name = "menuPool";
            this.menuPool.Size = new System.Drawing.Size(153, 114);
            // 
            // renameToolStripMenuItem
            // 
            this.renameToolStripMenuItem.Name = "renameToolStripMenuItem";
            this.renameToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.renameToolStripMenuItem.Text = "Rename";
            this.renameToolStripMenuItem.Click += new System.EventHandler(this.renameToolStripMenuItem_Click);
            // 
            // newSubdirectoryToolStripMenuItem
            // 
            this.newSubdirectoryToolStripMenuItem.Name = "newSubdirectoryToolStripMenuItem";
            this.newSubdirectoryToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.newSubdirectoryToolStripMenuItem.Text = "New Subfolder";
            this.newSubdirectoryToolStripMenuItem.Click += new System.EventHandler(this.newSubdirectoryToolStripMenuItem_Click);
            // 
            // duplicateToolStripMenuItem
            // 
            this.duplicateToolStripMenuItem.Name = "duplicateToolStripMenuItem";
            this.duplicateToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.duplicateToolStripMenuItem.Text = "Duplicate";
            this.duplicateToolStripMenuItem.Click += new System.EventHandler(this.duplicateToolStripMenuItem_Click);
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
            // 
            // exportToolStripMenuItem1
            // 
            this.exportToolStripMenuItem1.Name = "exportToolStripMenuItem1";
            this.exportToolStripMenuItem1.Size = new System.Drawing.Size(152, 22);
            this.exportToolStripMenuItem1.Text = "Export";
            // 
            // MainForm
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(278, 562);
            this.Controls.Add(this.panel1);
            this.DoubleBuffered = true;
            this.ForeColor = System.Drawing.SystemColors.ControlText;
            this.Name = "MainForm";
            this.Text = "StoryDesigner";
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

