namespace StoryDesigner
{
    partial class LanguageDlg
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
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.addLanguage = new System.Windows.Forms.Button();
            this.label_language = new System.Windows.Forms.Label();
            this.language = new System.Windows.Forms.ComboBox();
            this.delete_language = new System.Windows.Forms.Button();
            this.tabControl2 = new System.Windows.Forms.TabControl();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.commandGrid = new System.Windows.Forms.DataGridView();
            this.command = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.text = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Separator = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Voicefile_translated = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.text_translated = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.tabControl1.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.tabControl2.SuspendLayout();
            this.tabPage4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.commandGrid)).BeginInit();
            this.SuspendLayout();
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(882, 424);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage1
            // 
            this.tabPage1.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(470, 212);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Export dialogues";
            // 
            // tabPage2
            // 
            this.tabPage2.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage2.Controls.Add(this.tabControl2);
            this.tabPage2.Controls.Add(this.delete_language);
            this.tabPage2.Controls.Add(this.language);
            this.tabPage2.Controls.Add(this.label_language);
            this.tabPage2.Controls.Add(this.addLanguage);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(874, 398);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Translate";
            // 
            // tabPage3
            // 
            this.tabPage3.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(470, 212);
            this.tabPage3.TabIndex = 2;
            this.tabPage3.Text = "Create sound lists";
            // 
            // addLanguage
            // 
            this.addLanguage.Location = new System.Drawing.Point(6, 20);
            this.addLanguage.Name = "addLanguage";
            this.addLanguage.Size = new System.Drawing.Size(88, 23);
            this.addLanguage.TabIndex = 0;
            this.addLanguage.Text = "Add language";
            this.addLanguage.UseVisualStyleBackColor = true;
            // 
            // label_language
            // 
            this.label_language.AutoSize = true;
            this.label_language.Location = new System.Drawing.Point(170, 3);
            this.label_language.Name = "label_language";
            this.label_language.Size = new System.Drawing.Size(58, 13);
            this.label_language.TabIndex = 1;
            this.label_language.Text = "Language:";
            // 
            // language
            // 
            this.language.FormattingEnabled = true;
            this.language.Location = new System.Drawing.Point(136, 22);
            this.language.Name = "language";
            this.language.Size = new System.Drawing.Size(121, 21);
            this.language.TabIndex = 2;
            // 
            // delete_language
            // 
            this.delete_language.Location = new System.Drawing.Point(297, 20);
            this.delete_language.Name = "delete_language";
            this.delete_language.Size = new System.Drawing.Size(99, 23);
            this.delete_language.TabIndex = 3;
            this.delete_language.Text = "Delete language";
            this.delete_language.UseVisualStyleBackColor = true;
            // 
            // tabControl2
            // 
            this.tabControl2.Controls.Add(this.tabPage4);
            this.tabControl2.Controls.Add(this.tabPage5);
            this.tabControl2.Location = new System.Drawing.Point(6, 49);
            this.tabControl2.Name = "tabControl2";
            this.tabControl2.SelectedIndex = 0;
            this.tabControl2.Size = new System.Drawing.Size(868, 353);
            this.tabControl2.TabIndex = 4;
            // 
            // tabPage4
            // 
            this.tabPage4.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage4.Controls.Add(this.commandGrid);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(860, 327);
            this.tabPage4.TabIndex = 0;
            this.tabPage4.Text = "Speech";
            // 
            // tabPage5
            // 
            this.tabPage5.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(860, 246);
            this.tabPage5.TabIndex = 1;
            this.tabPage5.Text = "Offspeech";
            // 
            // commandGrid
            // 
            this.commandGrid.AllowUserToResizeColumns = false;
            this.commandGrid.AllowUserToResizeRows = false;
            this.commandGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.commandGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.command,
            this.text,
            this.Separator,
            this.Voicefile_translated,
            this.text_translated});
            this.commandGrid.Location = new System.Drawing.Point(0, 0);
            this.commandGrid.Name = "commandGrid";
            this.commandGrid.Size = new System.Drawing.Size(868, 327);
            this.commandGrid.TabIndex = 1;
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
            this.text.Width = 300;
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
            this.text_translated.Width = 300;
            // 
            // LanguageDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(880, 424);
            this.Controls.Add(this.tabControl1);
            this.Name = "LanguageDlg";
            this.Text = "Language and sound tools";
            this.tabControl1.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabControl2.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.commandGrid)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabPage tabPage3;
        private System.Windows.Forms.Button addLanguage;
        private System.Windows.Forms.Button delete_language;
        private System.Windows.Forms.ComboBox language;
        private System.Windows.Forms.Label label_language;
        private System.Windows.Forms.TabControl tabControl2;
        private System.Windows.Forms.TabPage tabPage4;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.DataGridView commandGrid;
        private System.Windows.Forms.DataGridViewTextBoxColumn command;
        private System.Windows.Forms.DataGridViewTextBoxColumn text;
        private System.Windows.Forms.DataGridViewTextBoxColumn Separator;
        private System.Windows.Forms.DataGridViewTextBoxColumn Voicefile_translated;
        private System.Windows.Forms.DataGridViewTextBoxColumn text_translated;
    }
}