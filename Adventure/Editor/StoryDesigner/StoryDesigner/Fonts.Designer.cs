namespace StoryDesigner
{
    partial class Fonts
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
            this.fontlist = new System.Windows.Forms.ListView();
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.preview = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.font = new System.Windows.Forms.ComboBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ol_none = new System.Windows.Forms.RadioButton();
            this.ol_shadow_a = new System.Windows.Forms.RadioButton();
            this.ol_shadow_b = new System.Windows.Forms.RadioButton();
            this.ol_shadow_ab = new System.Windows.Forms.RadioButton();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.sh_none = new System.Windows.Forms.RadioButton();
            this.sh_25 = new System.Windows.Forms.RadioButton();
            this.sh_50 = new System.Windows.Forms.RadioButton();
            this.sh_75 = new System.Windows.Forms.RadioButton();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.fill_scanlines = new System.Windows.Forms.RadioButton();
            this.fill_falloff_2 = new System.Windows.Forms.RadioButton();
            this.fill_falloff_1 = new System.Windows.Forms.RadioButton();
            this.fill_normal = new System.Windows.Forms.RadioButton();
            this.label2 = new System.Windows.Forms.Label();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.bold = new System.Windows.Forms.CheckBox();
            this.italic = new System.Windows.Forms.CheckBox();
            this.comboBox2 = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.button3 = new System.Windows.Forms.Button();
            this.coin_fading_indicator = new System.Windows.Forms.Label();
            this.coin_fading = new System.Windows.Forms.NumericUpDown();
            this.label29 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.preview)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.coin_fading)).BeginInit();
            this.SuspendLayout();
            // 
            // fontlist
            // 
            this.fontlist.Location = new System.Drawing.Point(12, 41);
            this.fontlist.Name = "fontlist";
            this.fontlist.Size = new System.Drawing.Size(265, 154);
            this.fontlist.TabIndex = 0;
            this.fontlist.UseCompatibleStateImageBehavior = false;
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(12, 12);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 1;
            this.button1.Text = "Add";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(202, 12);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 2;
            this.button2.Text = "Remove";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // preview
            // 
            this.preview.BackColor = System.Drawing.SystemColors.ControlDarkDark;
            this.preview.Location = new System.Drawing.Point(309, 41);
            this.preview.Name = "preview";
            this.preview.Size = new System.Drawing.Size(263, 154);
            this.preview.TabIndex = 3;
            this.preview.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(422, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(45, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Preview";
            // 
            // font
            // 
            this.font.FormattingEnabled = true;
            this.font.Location = new System.Drawing.Point(12, 233);
            this.font.Name = "font";
            this.font.Size = new System.Drawing.Size(164, 21);
            this.font.TabIndex = 5;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ol_shadow_ab);
            this.groupBox1.Controls.Add(this.ol_shadow_b);
            this.groupBox1.Controls.Add(this.ol_shadow_a);
            this.groupBox1.Controls.Add(this.ol_none);
            this.groupBox1.Location = new System.Drawing.Point(202, 201);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(200, 113);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Outline";
            // 
            // ol_none
            // 
            this.ol_none.AutoSize = true;
            this.ol_none.Location = new System.Drawing.Point(6, 19);
            this.ol_none.Name = "ol_none";
            this.ol_none.Size = new System.Drawing.Size(51, 17);
            this.ol_none.TabIndex = 0;
            this.ol_none.TabStop = true;
            this.ol_none.Text = "None";
            this.ol_none.UseVisualStyleBackColor = true;
            // 
            // ol_shadow_a
            // 
            this.ol_shadow_a.AutoSize = true;
            this.ol_shadow_a.Location = new System.Drawing.Point(6, 42);
            this.ol_shadow_a.Name = "ol_shadow_a";
            this.ol_shadow_a.Size = new System.Drawing.Size(74, 17);
            this.ol_shadow_a.TabIndex = 1;
            this.ol_shadow_a.TabStop = true;
            this.ol_shadow_a.Text = "Shadow A";
            this.ol_shadow_a.UseVisualStyleBackColor = true;
            // 
            // ol_shadow_b
            // 
            this.ol_shadow_b.AutoSize = true;
            this.ol_shadow_b.Location = new System.Drawing.Point(6, 65);
            this.ol_shadow_b.Name = "ol_shadow_b";
            this.ol_shadow_b.Size = new System.Drawing.Size(74, 17);
            this.ol_shadow_b.TabIndex = 1;
            this.ol_shadow_b.TabStop = true;
            this.ol_shadow_b.Text = "Shadow B";
            this.ol_shadow_b.UseVisualStyleBackColor = true;
            // 
            // ol_shadow_ab
            // 
            this.ol_shadow_ab.AutoSize = true;
            this.ol_shadow_ab.Location = new System.Drawing.Point(6, 88);
            this.ol_shadow_ab.Name = "ol_shadow_ab";
            this.ol_shadow_ab.Size = new System.Drawing.Size(87, 17);
            this.ol_shadow_ab.TabIndex = 2;
            this.ol_shadow_ab.TabStop = true;
            this.ol_shadow_ab.Text = "Shadow A+B";
            this.ol_shadow_ab.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.sh_75);
            this.groupBox2.Controls.Add(this.sh_50);
            this.groupBox2.Controls.Add(this.sh_25);
            this.groupBox2.Controls.Add(this.sh_none);
            this.groupBox2.Location = new System.Drawing.Point(202, 327);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(80, 108);
            this.groupBox2.TabIndex = 7;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Shadow";
            // 
            // sh_none
            // 
            this.sh_none.AutoSize = true;
            this.sh_none.Location = new System.Drawing.Point(6, 19);
            this.sh_none.Name = "sh_none";
            this.sh_none.Size = new System.Drawing.Size(51, 17);
            this.sh_none.TabIndex = 0;
            this.sh_none.TabStop = true;
            this.sh_none.Text = "None";
            this.sh_none.UseVisualStyleBackColor = true;
            // 
            // sh_25
            // 
            this.sh_25.AutoSize = true;
            this.sh_25.Location = new System.Drawing.Point(6, 42);
            this.sh_25.Name = "sh_25";
            this.sh_25.Size = new System.Drawing.Size(48, 17);
            this.sh_25.TabIndex = 1;
            this.sh_25.TabStop = true;
            this.sh_25.Text = "25 %";
            this.sh_25.UseVisualStyleBackColor = true;
            // 
            // sh_50
            // 
            this.sh_50.AutoSize = true;
            this.sh_50.Location = new System.Drawing.Point(6, 65);
            this.sh_50.Name = "sh_50";
            this.sh_50.Size = new System.Drawing.Size(48, 17);
            this.sh_50.TabIndex = 2;
            this.sh_50.TabStop = true;
            this.sh_50.Text = "50 %";
            this.sh_50.UseVisualStyleBackColor = true;
            // 
            // sh_75
            // 
            this.sh_75.AutoSize = true;
            this.sh_75.Location = new System.Drawing.Point(6, 85);
            this.sh_75.Name = "sh_75";
            this.sh_75.Size = new System.Drawing.Size(48, 17);
            this.sh_75.TabIndex = 3;
            this.sh_75.TabStop = true;
            this.sh_75.Text = "75 %";
            this.sh_75.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.fill_scanlines);
            this.groupBox3.Controls.Add(this.fill_falloff_2);
            this.groupBox3.Controls.Add(this.fill_falloff_1);
            this.groupBox3.Controls.Add(this.fill_normal);
            this.groupBox3.Location = new System.Drawing.Point(288, 327);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(114, 108);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Fill";
            // 
            // fill_scanlines
            // 
            this.fill_scanlines.AutoSize = true;
            this.fill_scanlines.Location = new System.Drawing.Point(6, 85);
            this.fill_scanlines.Name = "fill_scanlines";
            this.fill_scanlines.Size = new System.Drawing.Size(71, 17);
            this.fill_scanlines.TabIndex = 3;
            this.fill_scanlines.TabStop = true;
            this.fill_scanlines.Text = "Scanlines";
            this.fill_scanlines.UseVisualStyleBackColor = true;
            // 
            // fill_falloff_2
            // 
            this.fill_falloff_2.AutoSize = true;
            this.fill_falloff_2.Location = new System.Drawing.Point(6, 65);
            this.fill_falloff_2.Name = "fill_falloff_2";
            this.fill_falloff_2.Size = new System.Drawing.Size(62, 17);
            this.fill_falloff_2.TabIndex = 2;
            this.fill_falloff_2.TabStop = true;
            this.fill_falloff_2.Text = "Falloff 2";
            this.fill_falloff_2.UseVisualStyleBackColor = true;
            // 
            // fill_falloff_1
            // 
            this.fill_falloff_1.AutoSize = true;
            this.fill_falloff_1.Location = new System.Drawing.Point(6, 42);
            this.fill_falloff_1.Name = "fill_falloff_1";
            this.fill_falloff_1.Size = new System.Drawing.Size(62, 17);
            this.fill_falloff_1.TabIndex = 1;
            this.fill_falloff_1.TabStop = true;
            this.fill_falloff_1.Text = "Falloff 1";
            this.fill_falloff_1.UseVisualStyleBackColor = true;
            // 
            // fill_normal
            // 
            this.fill_normal.AutoSize = true;
            this.fill_normal.Location = new System.Drawing.Point(6, 19);
            this.fill_normal.Name = "fill_normal";
            this.fill_normal.Size = new System.Drawing.Size(58, 17);
            this.fill_normal.TabIndex = 0;
            this.fill_normal.TabStop = true;
            this.fill_normal.Text = "Normal";
            this.fill_normal.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 266);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(46, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "Charset:";
            // 
            // comboBox1
            // 
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Location = new System.Drawing.Point(12, 282);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(164, 21);
            this.comboBox1.TabIndex = 5;
            // 
            // bold
            // 
            this.bold.AutoSize = true;
            this.bold.Location = new System.Drawing.Point(29, 352);
            this.bold.Name = "bold";
            this.bold.Size = new System.Drawing.Size(47, 17);
            this.bold.TabIndex = 9;
            this.bold.Text = "Bold";
            this.bold.UseVisualStyleBackColor = true;
            // 
            // italic
            // 
            this.italic.AutoSize = true;
            this.italic.Location = new System.Drawing.Point(28, 375);
            this.italic.Name = "italic";
            this.italic.Size = new System.Drawing.Size(48, 17);
            this.italic.TabIndex = 10;
            this.italic.Text = "Italic";
            this.italic.UseVisualStyleBackColor = true;
            // 
            // comboBox2
            // 
            this.comboBox2.FormattingEnabled = true;
            this.comboBox2.Location = new System.Drawing.Point(107, 371);
            this.comboBox2.Name = "comboBox2";
            this.comboBox2.Size = new System.Drawing.Size(69, 21);
            this.comboBox2.TabIndex = 11;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(126, 355);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(27, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "Size";
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(457, 311);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 13;
            this.button3.Text = "OK";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // coin_fading_indicator
            // 
            this.coin_fading_indicator.AutoSize = true;
            this.coin_fading_indicator.Location = new System.Drawing.Point(53, 438);
            this.coin_fading_indicator.Name = "coin_fading_indicator";
            this.coin_fading_indicator.Size = new System.Drawing.Size(43, 13);
            this.coin_fading_indicator.TabIndex = 16;
            this.coin_fading_indicator.Text = "<none>";
            // 
            // coin_fading
            // 
            this.coin_fading.Location = new System.Drawing.Point(56, 415);
            this.coin_fading.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
            this.coin_fading.Name = "coin_fading";
            this.coin_fading.Size = new System.Drawing.Size(120, 20);
            this.coin_fading.TabIndex = 15;
            // 
            // label29
            // 
            this.label29.AutoSize = true;
            this.label29.Location = new System.Drawing.Point(8, 417);
            this.label29.Name = "label29";
            this.label29.Size = new System.Drawing.Size(42, 13);
            this.label29.TabIndex = 14;
            this.label29.Text = "Fading:";
            // 
            // Fonts
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 462);
            this.Controls.Add(this.coin_fading_indicator);
            this.Controls.Add(this.coin_fading);
            this.Controls.Add(this.label29);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.comboBox2);
            this.Controls.Add(this.italic);
            this.Controls.Add(this.bold);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.comboBox1);
            this.Controls.Add(this.font);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.preview);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.fontlist);
            this.Name = "Fonts";
            this.Text = "Fonts";
            ((System.ComponentModel.ISupportInitialize)(this.preview)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.coin_fading)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView fontlist;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.PictureBox preview;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox font;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton ol_shadow_a;
        private System.Windows.Forms.RadioButton ol_none;
        private System.Windows.Forms.RadioButton ol_shadow_ab;
        private System.Windows.Forms.RadioButton ol_shadow_b;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.RadioButton sh_none;
        private System.Windows.Forms.RadioButton sh_75;
        private System.Windows.Forms.RadioButton sh_50;
        private System.Windows.Forms.RadioButton sh_25;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.RadioButton fill_scanlines;
        private System.Windows.Forms.RadioButton fill_falloff_2;
        private System.Windows.Forms.RadioButton fill_falloff_1;
        private System.Windows.Forms.RadioButton fill_normal;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ComboBox comboBox1;
        private System.Windows.Forms.CheckBox bold;
        private System.Windows.Forms.CheckBox italic;
        private System.Windows.Forms.ComboBox comboBox2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Label coin_fading_indicator;
        private System.Windows.Forms.NumericUpDown coin_fading;
        private System.Windows.Forms.Label label29;
    }
}