namespace StoryDesigner
{
    partial class ScriptDlg
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
            this.scripttext = new System.Windows.Forms.RichTextBox();
            this.font = new System.Windows.Forms.Button();
            this.undo = new System.Windows.Forms.Button();
            this.brackets = new System.Windows.Forms.Button();
            this.add = new System.Windows.Forms.Button();
            this.linenumberbox = new System.Windows.Forms.PictureBox();
            this.matches = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.info = new System.Windows.Forms.Label();
            this.addMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.pRESETSToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.customPresetsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.itemToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.characterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.objectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.roomToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.textsceneLToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel2RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel3RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel4RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel5RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel6RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel7RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel8RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.textsceneLevel9RowsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.onToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.linenumberbox)).BeginInit();
            this.addMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // scripttext
            // 
            this.scripttext.AcceptsTab = true;
            this.scripttext.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.scripttext.Font = new System.Drawing.Font("Courier New", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.scripttext.Location = new System.Drawing.Point(39, 41);
            this.scripttext.Name = "scripttext";
            this.scripttext.Size = new System.Drawing.Size(535, 309);
            this.scripttext.TabIndex = 0;
            this.scripttext.Text = "";
            this.scripttext.WordWrap = false;
            // 
            // font
            // 
            this.font.Location = new System.Drawing.Point(12, 12);
            this.font.Name = "font";
            this.font.Size = new System.Drawing.Size(36, 23);
            this.font.TabIndex = 1;
            this.font.Text = "Font";
            this.font.UseVisualStyleBackColor = true;
            this.font.Click += new System.EventHandler(this.font_Click);
            // 
            // undo
            // 
            this.undo.Location = new System.Drawing.Point(54, 12);
            this.undo.Name = "undo";
            this.undo.Size = new System.Drawing.Size(41, 23);
            this.undo.TabIndex = 2;
            this.undo.Text = "Undo";
            this.undo.UseVisualStyleBackColor = true;
            this.undo.Click += new System.EventHandler(this.undo_Click);
            // 
            // brackets
            // 
            this.brackets.Location = new System.Drawing.Point(101, 12);
            this.brackets.Name = "brackets";
            this.brackets.Size = new System.Drawing.Size(28, 23);
            this.brackets.TabIndex = 3;
            this.brackets.Text = "{ }";
            this.brackets.UseVisualStyleBackColor = true;
            this.brackets.Click += new System.EventHandler(this.brackets_Click);
            // 
            // add
            // 
            this.add.Location = new System.Drawing.Point(135, 12);
            this.add.Name = "add";
            this.add.Size = new System.Drawing.Size(75, 23);
            this.add.TabIndex = 4;
            this.add.Text = "Add";
            this.add.UseVisualStyleBackColor = true;
            this.add.Click += new System.EventHandler(this.add_Click);
            // 
            // linenumberbox
            // 
            this.linenumberbox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.linenumberbox.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.linenumberbox.Location = new System.Drawing.Point(1, 41);
            this.linenumberbox.Name = "linenumberbox";
            this.linenumberbox.Size = new System.Drawing.Size(38, 309);
            this.linenumberbox.TabIndex = 5;
            this.linenumberbox.TabStop = false;
            // 
            // matches
            // 
            this.matches.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.matches.FormattingEnabled = true;
            this.matches.Location = new System.Drawing.Point(580, 60);
            this.matches.Name = "matches";
            this.matches.Size = new System.Drawing.Size(124, 290);
            this.matches.Sorted = true;
            this.matches.TabIndex = 6;
            // 
            // label1
            // 
            this.label1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(577, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(28, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "Info:";
            // 
            // info
            // 
            this.info.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.info.AutoSize = true;
            this.info.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.info.Location = new System.Drawing.Point(602, 9);
            this.info.MaximumSize = new System.Drawing.Size(120, 0);
            this.info.Name = "info";
            this.info.Size = new System.Drawing.Size(61, 13);
            this.info.TabIndex = 8;
            this.info.Text = "Command";
            // 
            // addMenu
            // 
            this.addMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.pRESETSToolStripMenuItem,
            this.toolStripSeparator1,
            this.onToolStripMenuItem});
            this.addMenu.Name = "contextMenuStrip1";
            this.addMenu.Size = new System.Drawing.Size(153, 76);
            // 
            // pRESETSToolStripMenuItem
            // 
            this.pRESETSToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.customPresetsToolStripMenuItem,
            this.toolStripSeparator2,
            this.itemToolStripMenuItem,
            this.characterToolStripMenuItem,
            this.objectToolStripMenuItem,
            this.roomToolStripMenuItem,
            this.toolStripSeparator3,
            this.textsceneLToolStripMenuItem,
            this.textsceneLevel2RowsToolStripMenuItem,
            this.textsceneLevel3RowsToolStripMenuItem,
            this.textsceneLevel4RowsToolStripMenuItem,
            this.textsceneLevel5RowsToolStripMenuItem,
            this.textsceneLevel6RowsToolStripMenuItem,
            this.textsceneLevel7RowsToolStripMenuItem,
            this.textsceneLevel8RowsToolStripMenuItem,
            this.textsceneLevel9RowsToolStripMenuItem});
            this.pRESETSToolStripMenuItem.Name = "pRESETSToolStripMenuItem";
            this.pRESETSToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.pRESETSToolStripMenuItem.Text = "PRESETS";
            // 
            // customPresetsToolStripMenuItem
            // 
            this.customPresetsToolStripMenuItem.Enabled = false;
            this.customPresetsToolStripMenuItem.Name = "customPresetsToolStripMenuItem";
            this.customPresetsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.customPresetsToolStripMenuItem.Text = "Custom Presets";
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(196, 6);
            // 
            // itemToolStripMenuItem
            // 
            this.itemToolStripMenuItem.Name = "itemToolStripMenuItem";
            this.itemToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.itemToolStripMenuItem.Text = "Item";
            this.itemToolStripMenuItem.Click += new System.EventHandler(this.itemToolStripMenuItem_Click);
            // 
            // characterToolStripMenuItem
            // 
            this.characterToolStripMenuItem.Name = "characterToolStripMenuItem";
            this.characterToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.characterToolStripMenuItem.Text = "Character";
            this.characterToolStripMenuItem.Click += new System.EventHandler(this.characterToolStripMenuItem_Click);
            // 
            // objectToolStripMenuItem
            // 
            this.objectToolStripMenuItem.Name = "objectToolStripMenuItem";
            this.objectToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.objectToolStripMenuItem.Text = "Object";
            this.objectToolStripMenuItem.Click += new System.EventHandler(this.objectToolStripMenuItem_Click);
            // 
            // roomToolStripMenuItem
            // 
            this.roomToolStripMenuItem.Name = "roomToolStripMenuItem";
            this.roomToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.roomToolStripMenuItem.Text = "Room";
            this.roomToolStripMenuItem.Click += new System.EventHandler(this.roomToolStripMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(196, 6);
            // 
            // textsceneLToolStripMenuItem
            // 
            this.textsceneLToolStripMenuItem.Name = "textsceneLToolStripMenuItem";
            this.textsceneLToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLToolStripMenuItem.Text = "Textscene Level, 1 Row";
            this.textsceneLToolStripMenuItem.Click += new System.EventHandler(this.textsceneLToolStripMenuItem_Click);
            // 
            // textsceneLevel2RowsToolStripMenuItem
            // 
            this.textsceneLevel2RowsToolStripMenuItem.Name = "textsceneLevel2RowsToolStripMenuItem";
            this.textsceneLevel2RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel2RowsToolStripMenuItem.Text = "Textscene Level, 2 Rows";
            this.textsceneLevel2RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel2RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel3RowsToolStripMenuItem
            // 
            this.textsceneLevel3RowsToolStripMenuItem.Name = "textsceneLevel3RowsToolStripMenuItem";
            this.textsceneLevel3RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel3RowsToolStripMenuItem.Text = "Textscene Level, 3 Rows";
            this.textsceneLevel3RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel3RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel4RowsToolStripMenuItem
            // 
            this.textsceneLevel4RowsToolStripMenuItem.Name = "textsceneLevel4RowsToolStripMenuItem";
            this.textsceneLevel4RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel4RowsToolStripMenuItem.Text = "Textscene Level, 4 Rows";
            this.textsceneLevel4RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel4RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel5RowsToolStripMenuItem
            // 
            this.textsceneLevel5RowsToolStripMenuItem.Name = "textsceneLevel5RowsToolStripMenuItem";
            this.textsceneLevel5RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel5RowsToolStripMenuItem.Text = "Textscene Level, 5 Rows";
            this.textsceneLevel5RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel5RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel6RowsToolStripMenuItem
            // 
            this.textsceneLevel6RowsToolStripMenuItem.Name = "textsceneLevel6RowsToolStripMenuItem";
            this.textsceneLevel6RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel6RowsToolStripMenuItem.Text = "Textscene Level, 6 Rows";
            this.textsceneLevel6RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel6RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel7RowsToolStripMenuItem
            // 
            this.textsceneLevel7RowsToolStripMenuItem.Name = "textsceneLevel7RowsToolStripMenuItem";
            this.textsceneLevel7RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel7RowsToolStripMenuItem.Text = "Textscene Level, 7 Rows";
            this.textsceneLevel7RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel7RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel8RowsToolStripMenuItem
            // 
            this.textsceneLevel8RowsToolStripMenuItem.Name = "textsceneLevel8RowsToolStripMenuItem";
            this.textsceneLevel8RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel8RowsToolStripMenuItem.Text = "Textscene Level, 8 Rows";
            this.textsceneLevel8RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel8RowsToolStripMenuItem_Click);
            // 
            // textsceneLevel9RowsToolStripMenuItem
            // 
            this.textsceneLevel9RowsToolStripMenuItem.Name = "textsceneLevel9RowsToolStripMenuItem";
            this.textsceneLevel9RowsToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.textsceneLevel9RowsToolStripMenuItem.Text = "Textscene Level, 9 Rows";
            this.textsceneLevel9RowsToolStripMenuItem.Click += new System.EventHandler(this.textsceneLevel9RowsToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(149, 6);
            // 
            // onToolStripMenuItem
            // 
            this.onToolStripMenuItem.Name = "onToolStripMenuItem";
            this.onToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.onToolStripMenuItem.Text = "On";
            // 
            // ScriptDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(716, 362);
            this.Controls.Add(this.info);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.matches);
            this.Controls.Add(this.linenumberbox);
            this.Controls.Add(this.add);
            this.Controls.Add(this.brackets);
            this.Controls.Add(this.undo);
            this.Controls.Add(this.font);
            this.Controls.Add(this.scripttext);
            this.Name = "ScriptDlg";
            this.Text = "ScriptDlg";
            ((System.ComponentModel.ISupportInitialize)(this.linenumberbox)).EndInit();
            this.addMenu.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RichTextBox scripttext;
        private System.Windows.Forms.Button font;
        private System.Windows.Forms.Button undo;
        private System.Windows.Forms.Button brackets;
        private System.Windows.Forms.Button add;
        private System.Windows.Forms.PictureBox linenumberbox;
        private System.Windows.Forms.ListBox matches;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label info;
        private System.Windows.Forms.ContextMenuStrip addMenu;
        private System.Windows.Forms.ToolStripMenuItem pRESETSToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem onToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem customPresetsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem itemToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem characterToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem objectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem roomToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem textsceneLToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel2RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel3RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel4RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel5RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel6RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel7RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel8RowsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem textsceneLevel9RowsToolStripMenuItem;
    }
}