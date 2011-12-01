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
            this.SuspendLayout();
            // 
            // scripttext
            // 
            this.scripttext.AcceptsTab = true;
            this.scripttext.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.scripttext.Font = new System.Drawing.Font("Courier New", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.scripttext.Location = new System.Drawing.Point(12, 41);
            this.scripttext.Name = "scripttext";
            this.scripttext.Size = new System.Drawing.Size(461, 309);
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
            // ScriptDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(485, 362);
            this.Controls.Add(this.add);
            this.Controls.Add(this.brackets);
            this.Controls.Add(this.undo);
            this.Controls.Add(this.font);
            this.Controls.Add(this.scripttext);
            this.Name = "ScriptDlg";
            this.Text = "ScriptDlg";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.RichTextBox scripttext;
        private System.Windows.Forms.Button font;
        private System.Windows.Forms.Button undo;
        private System.Windows.Forms.Button brackets;
        private System.Windows.Forms.Button add;
    }
}