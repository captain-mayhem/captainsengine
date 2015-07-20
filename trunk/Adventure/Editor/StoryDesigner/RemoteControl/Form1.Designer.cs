namespace RemoteControl
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
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.connectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.disconnectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.modeTabs = new System.Windows.Forms.TabControl();
            this.scriptTab = new System.Windows.Forms.TabPage();
            this.remoteTab = new System.Windows.Forms.TabPage();
            this.script = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.modeTabs.SuspendLayout();
            this.scriptTab.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(634, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.connectToolStripMenuItem,
            this.disconnectToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(81, 20);
            this.fileToolStripMenuItem.Text = "Commands";
            // 
            // connectToolStripMenuItem
            // 
            this.connectToolStripMenuItem.Name = "connectToolStripMenuItem";
            this.connectToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.connectToolStripMenuItem.Text = "Connect";
            this.connectToolStripMenuItem.Click += new System.EventHandler(this.connectToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.settingsToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.settingsToolStripMenuItem.Text = "Settings";
            this.settingsToolStripMenuItem.Click += new System.EventHandler(this.settingsToolStripMenuItem_Click);
            // 
            // disconnectToolStripMenuItem
            // 
            this.disconnectToolStripMenuItem.Name = "disconnectToolStripMenuItem";
            this.disconnectToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.disconnectToolStripMenuItem.Text = "Disconnect";
            this.disconnectToolStripMenuItem.Click += new System.EventHandler(this.disconnectToolStripMenuItem_Click);
            // 
            // modeTabs
            // 
            this.modeTabs.Controls.Add(this.scriptTab);
            this.modeTabs.Controls.Add(this.remoteTab);
            this.modeTabs.Location = new System.Drawing.Point(0, 27);
            this.modeTabs.Name = "modeTabs";
            this.modeTabs.SelectedIndex = 0;
            this.modeTabs.Size = new System.Drawing.Size(634, 457);
            this.modeTabs.TabIndex = 1;
            // 
            // scriptTab
            // 
            this.scriptTab.Controls.Add(this.button1);
            this.scriptTab.Controls.Add(this.script);
            this.scriptTab.Location = new System.Drawing.Point(4, 22);
            this.scriptTab.Name = "scriptTab";
            this.scriptTab.Padding = new System.Windows.Forms.Padding(3);
            this.scriptTab.Size = new System.Drawing.Size(626, 431);
            this.scriptTab.TabIndex = 0;
            this.scriptTab.Text = "Script window";
            this.scriptTab.UseVisualStyleBackColor = true;
            // 
            // remoteTab
            // 
            this.remoteTab.Location = new System.Drawing.Point(4, 22);
            this.remoteTab.Name = "remoteTab";
            this.remoteTab.Padding = new System.Windows.Forms.Padding(3);
            this.remoteTab.Size = new System.Drawing.Size(626, 431);
            this.remoteTab.TabIndex = 1;
            this.remoteTab.Text = "Remote control";
            this.remoteTab.UseVisualStyleBackColor = true;
            // 
            // script
            // 
            this.script.Location = new System.Drawing.Point(8, 66);
            this.script.Multiline = true;
            this.script.Name = "script";
            this.script.Size = new System.Drawing.Size(610, 226);
            this.script.TabIndex = 0;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(279, 298);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 1;
            this.button1.Text = "send";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(634, 482);
            this.Controls.Add(this.modeTabs);
            this.Controls.Add(this.menuStrip1);
            this.Name = "MainForm";
            this.Text = "CGE Remote Control Interface";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.modeTabs.ResumeLayout(false);
            this.scriptTab.ResumeLayout(false);
            this.scriptTab.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem connectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem disconnectToolStripMenuItem;
        private System.Windows.Forms.TabControl modeTabs;
        private System.Windows.Forms.TabPage scriptTab;
        private System.Windows.Forms.TabPage remoteTab;
        private System.Windows.Forms.TextBox script;
        private System.Windows.Forms.Button button1;

    }
}

