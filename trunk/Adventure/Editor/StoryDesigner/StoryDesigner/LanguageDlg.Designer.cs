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
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.tabControl2 = new System.Windows.Forms.TabControl();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.speechGrid = new StoryDesigner.LanguageTable();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.offspeechGrid = new StoryDesigner.LanguageTable();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.showinfoGrid = new StoryDesigner.LanguageTable();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.textoutGrid = new StoryDesigner.LanguageTable();
            this.tabPage20 = new System.Windows.Forms.TabPage();
            this.setstringGrid = new StoryDesigner.LanguageTable();
            this.tabPage21 = new System.Windows.Forms.TabPage();
            this.textsceneGrid = new StoryDesigner.LanguageTable();
            this.tabPage22 = new System.Windows.Forms.TabPage();
            this.commandsGrid = new StoryDesigner.LanguageTable();
            this.delete_language = new System.Windows.Forms.Button();
            this.language = new System.Windows.Forms.ComboBox();
            this.label_language = new System.Windows.Forms.Label();
            this.addLanguage = new System.Windows.Forms.Button();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.tabPage2.SuspendLayout();
            this.tabControl2.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.tabPage20.SuspendLayout();
            this.tabPage21.SuspendLayout();
            this.tabPage22.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabPage1
            // 
            this.tabPage1.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(874, 398);
            this.tabPage1.TabIndex = 3;
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
            // tabControl2
            // 
            this.tabControl2.Controls.Add(this.tabPage4);
            this.tabControl2.Controls.Add(this.tabPage5);
            this.tabControl2.Controls.Add(this.tabPage6);
            this.tabControl2.Controls.Add(this.tabPage7);
            this.tabControl2.Controls.Add(this.tabPage20);
            this.tabControl2.Controls.Add(this.tabPage21);
            this.tabControl2.Controls.Add(this.tabPage22);
            this.tabControl2.Location = new System.Drawing.Point(6, 49);
            this.tabControl2.Name = "tabControl2";
            this.tabControl2.SelectedIndex = 0;
            this.tabControl2.Size = new System.Drawing.Size(868, 353);
            this.tabControl2.TabIndex = 4;
            // 
            // tabPage4
            // 
            this.tabPage4.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage4.Controls.Add(this.speechGrid);
            this.tabPage4.Location = new System.Drawing.Point(4, 22);
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage4.Size = new System.Drawing.Size(860, 327);
            this.tabPage4.TabIndex = 0;
            this.tabPage4.Text = "Speech";
            // 
            // speechGrid
            // 
            this.speechGrid.Location = new System.Drawing.Point(0, 0);
            this.speechGrid.Name = "speechGrid";
            this.speechGrid.Size = new System.Drawing.Size(855, 328);
            this.speechGrid.TabIndex = 0;
            this.speechGrid.VoiceAndText = true;
            // 
            // tabPage5
            // 
            this.tabPage5.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage5.Controls.Add(this.offspeechGrid);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(860, 327);
            this.tabPage5.TabIndex = 1;
            this.tabPage5.Text = "Offspeech";
            // 
            // offspeechGrid
            // 
            this.offspeechGrid.Location = new System.Drawing.Point(0, 0);
            this.offspeechGrid.Name = "offspeechGrid";
            this.offspeechGrid.Size = new System.Drawing.Size(882, 328);
            this.offspeechGrid.TabIndex = 2;
            this.offspeechGrid.VoiceAndText = true;
            // 
            // tabPage6
            // 
            this.tabPage6.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage6.Controls.Add(this.showinfoGrid);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage6.Size = new System.Drawing.Size(860, 327);
            this.tabPage6.TabIndex = 2;
            this.tabPage6.Text = "Showinfo";
            // 
            // showinfoGrid
            // 
            this.showinfoGrid.Location = new System.Drawing.Point(0, 0);
            this.showinfoGrid.Name = "showinfoGrid";
            this.showinfoGrid.Size = new System.Drawing.Size(856, 328);
            this.showinfoGrid.TabIndex = 0;
            this.showinfoGrid.VoiceAndText = false;
            // 
            // tabPage7
            // 
            this.tabPage7.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage7.Controls.Add(this.textoutGrid);
            this.tabPage7.Location = new System.Drawing.Point(4, 22);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage7.Size = new System.Drawing.Size(860, 327);
            this.tabPage7.TabIndex = 3;
            this.tabPage7.Text = "Textout";
            // 
            // textoutGrid
            // 
            this.textoutGrid.Location = new System.Drawing.Point(0, 0);
            this.textoutGrid.Name = "textoutGrid";
            this.textoutGrid.Size = new System.Drawing.Size(856, 328);
            this.textoutGrid.TabIndex = 0;
            this.textoutGrid.VoiceAndText = false;
            // 
            // tabPage20
            // 
            this.tabPage20.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage20.Controls.Add(this.setstringGrid);
            this.tabPage20.Location = new System.Drawing.Point(4, 22);
            this.tabPage20.Name = "tabPage20";
            this.tabPage20.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage20.Size = new System.Drawing.Size(860, 327);
            this.tabPage20.TabIndex = 4;
            this.tabPage20.Text = "Setstring";
            // 
            // setstringGrid
            // 
            this.setstringGrid.Location = new System.Drawing.Point(0, 0);
            this.setstringGrid.Name = "setstringGrid";
            this.setstringGrid.Size = new System.Drawing.Size(856, 328);
            this.setstringGrid.TabIndex = 0;
            this.setstringGrid.VoiceAndText = false;
            // 
            // tabPage21
            // 
            this.tabPage21.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage21.Controls.Add(this.textsceneGrid);
            this.tabPage21.Location = new System.Drawing.Point(4, 22);
            this.tabPage21.Name = "tabPage21";
            this.tabPage21.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage21.Size = new System.Drawing.Size(860, 327);
            this.tabPage21.TabIndex = 5;
            this.tabPage21.Text = "Row (textscene)";
            // 
            // textsceneGrid
            // 
            this.textsceneGrid.Location = new System.Drawing.Point(0, 0);
            this.textsceneGrid.Name = "textsceneGrid";
            this.textsceneGrid.Size = new System.Drawing.Size(856, 328);
            this.textsceneGrid.TabIndex = 0;
            this.textsceneGrid.VoiceAndText = false;
            // 
            // tabPage22
            // 
            this.tabPage22.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage22.Controls.Add(this.commandsGrid);
            this.tabPage22.Location = new System.Drawing.Point(4, 22);
            this.tabPage22.Name = "tabPage22";
            this.tabPage22.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage22.Size = new System.Drawing.Size(860, 327);
            this.tabPage22.TabIndex = 6;
            this.tabPage22.Text = "Commands";
            // 
            // commandsGrid
            // 
            this.commandsGrid.Location = new System.Drawing.Point(0, 0);
            this.commandsGrid.Name = "commandsGrid";
            this.commandsGrid.Size = new System.Drawing.Size(856, 328);
            this.commandsGrid.TabIndex = 0;
            this.commandsGrid.VoiceAndText = false;
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
            // language
            // 
            this.language.FormattingEnabled = true;
            this.language.Location = new System.Drawing.Point(136, 22);
            this.language.Name = "language";
            this.language.Size = new System.Drawing.Size(121, 21);
            this.language.TabIndex = 2;
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
            // addLanguage
            // 
            this.addLanguage.Location = new System.Drawing.Point(6, 20);
            this.addLanguage.Name = "addLanguage";
            this.addLanguage.Size = new System.Drawing.Size(88, 23);
            this.addLanguage.TabIndex = 0;
            this.addLanguage.Text = "Add language";
            this.addLanguage.UseVisualStyleBackColor = true;
            // 
            // tabControl1
            // 
            this.tabControl1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl1.Controls.Add(this.tabPage2);
            this.tabControl1.Controls.Add(this.tabPage1);
            this.tabControl1.Controls.Add(this.tabPage3);
            this.tabControl1.Location = new System.Drawing.Point(0, 0);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(882, 424);
            this.tabControl1.TabIndex = 0;
            // 
            // tabPage3
            // 
            this.tabPage3.BackColor = System.Drawing.SystemColors.Control;
            this.tabPage3.Location = new System.Drawing.Point(4, 22);
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage3.Size = new System.Drawing.Size(874, 398);
            this.tabPage3.TabIndex = 4;
            this.tabPage3.Text = "Create sound lists";
            // 
            // LanguageDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(880, 424);
            this.Controls.Add(this.tabControl1);
            this.Name = "LanguageDlg";
            this.Text = "Language and sound tools";
            this.tabPage2.ResumeLayout(false);
            this.tabPage2.PerformLayout();
            this.tabControl2.ResumeLayout(false);
            this.tabPage4.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.tabPage6.ResumeLayout(false);
            this.tabPage7.ResumeLayout(false);
            this.tabPage20.ResumeLayout(false);
            this.tabPage21.ResumeLayout(false);
            this.tabPage22.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.TabControl tabControl2;
        private System.Windows.Forms.TabPage tabPage4;
        private LanguageTable speechGrid;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.Button delete_language;
        private System.Windows.Forms.ComboBox language;
        private System.Windows.Forms.Label label_language;
        private System.Windows.Forms.Button addLanguage;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabPage3;
        private LanguageTable offspeechGrid;
        private System.Windows.Forms.TabPage tabPage6;
        private LanguageTable showinfoGrid;
        private System.Windows.Forms.TabPage tabPage7;
        private LanguageTable textoutGrid;
        private System.Windows.Forms.TabPage tabPage20;
        private LanguageTable setstringGrid;
        private System.Windows.Forms.TabPage tabPage21;
        private System.Windows.Forms.TabPage tabPage22;
        private LanguageTable textsceneGrid;
        private LanguageTable commandsGrid;

    }
}