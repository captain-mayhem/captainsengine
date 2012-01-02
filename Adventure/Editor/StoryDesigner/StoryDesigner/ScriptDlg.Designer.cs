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
            this.scripttext = new System.Windows.Forms.RichTextBox();
            this.font = new System.Windows.Forms.Button();
            this.undo = new System.Windows.Forms.Button();
            this.brackets = new System.Windows.Forms.Button();
            this.add = new System.Windows.Forms.Button();
            this.linenumberbox = new System.Windows.Forms.PictureBox();
            this.matches = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.info = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.linenumberbox)).BeginInit();
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
            // 
            // undo
            // 
            this.undo.Location = new System.Drawing.Point(54, 12);
            this.undo.Name = "undo";
            this.undo.Size = new System.Drawing.Size(41, 23);
            this.undo.TabIndex = 2;
            this.undo.Text = "Undo";
            this.undo.UseVisualStyleBackColor = true;
            // 
            // brackets
            // 
            this.brackets.Location = new System.Drawing.Point(101, 12);
            this.brackets.Name = "brackets";
            this.brackets.Size = new System.Drawing.Size(28, 23);
            this.brackets.TabIndex = 3;
            this.brackets.Text = "{ }";
            this.brackets.UseVisualStyleBackColor = true;
            // 
            // add
            // 
            this.add.Location = new System.Drawing.Point(135, 12);
            this.add.Name = "add";
            this.add.Size = new System.Drawing.Size(75, 23);
            this.add.TabIndex = 4;
            this.add.Text = "Add";
            this.add.UseVisualStyleBackColor = true;
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
    }
}