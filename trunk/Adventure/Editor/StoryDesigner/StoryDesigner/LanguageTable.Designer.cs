namespace StoryDesigner
{
    partial class LanguageTable
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

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.grid = new System.Windows.Forms.DataGridView();
            this.command = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.text = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Separator = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Voicefile_translated = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.text_translated = new System.Windows.Forms.DataGridViewTextBoxColumn();
            ((System.ComponentModel.ISupportInitialize)(this.grid)).BeginInit();
            this.SuspendLayout();
            // 
            // grid
            // 
            this.grid.AllowUserToResizeColumns = false;
            this.grid.AllowUserToResizeRows = false;
            this.grid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.grid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.command,
            this.text,
            this.Separator,
            this.Voicefile_translated,
            this.text_translated});
            this.grid.Location = new System.Drawing.Point(0, 0);
            this.grid.Name = "grid";
            this.grid.RowHeadersVisible = false;
            this.grid.Size = new System.Drawing.Size(854, 327);
            this.grid.TabIndex = 2;
            // 
            // command
            // 
            this.command.HeaderText = "Voicefile";
            this.command.Name = "command";
            this.command.ReadOnly = true;
            // 
            // text
            // 
            this.text.HeaderText = "Text - Origin";
            this.text.Name = "text";
            this.text.ReadOnly = true;
            this.text.Width = 305;
            // 
            // Separator
            // 
            this.Separator.HeaderText = "";
            this.Separator.Name = "Separator";
            this.Separator.ReadOnly = true;
            this.Separator.Width = 20;
            // 
            // Voicefile_translated
            // 
            this.Voicefile_translated.HeaderText = "Voicefile";
            this.Voicefile_translated.Name = "Voicefile_translated";
            // 
            // text_translated
            // 
            this.text_translated.HeaderText = "Text - Translated";
            this.text_translated.Name = "text_translated";
            this.text_translated.Width = 305;
            // 
            // LanguageTable
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.grid);
            this.Name = "LanguageTable";
            this.Size = new System.Drawing.Size(856, 328);
            ((System.ComponentModel.ISupportInitialize)(this.grid)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataGridView grid;
        private System.Windows.Forms.DataGridViewTextBoxColumn command;
        private System.Windows.Forms.DataGridViewTextBoxColumn text;
        private System.Windows.Forms.DataGridViewTextBoxColumn Separator;
        private System.Windows.Forms.DataGridViewTextBoxColumn Voicefile_translated;
        private System.Windows.Forms.DataGridViewTextBoxColumn text_translated;
    }
}
