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
            this.button1 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.preview = new System.Windows.Forms.PictureBox();
            this.label1 = new System.Windows.Forms.Label();
            this.font = new System.Windows.Forms.ComboBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ol_shadow_ab = new System.Windows.Forms.RadioButton();
            this.ol_shadow_b = new System.Windows.Forms.RadioButton();
            this.ol_shadow_a = new System.Windows.Forms.RadioButton();
            this.ol_none = new System.Windows.Forms.RadioButton();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.sh_75 = new System.Windows.Forms.RadioButton();
            this.sh_50 = new System.Windows.Forms.RadioButton();
            this.sh_25 = new System.Windows.Forms.RadioButton();
            this.sh_none = new System.Windows.Forms.RadioButton();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.fill_scanlines = new System.Windows.Forms.RadioButton();
            this.fill_falloff_2 = new System.Windows.Forms.RadioButton();
            this.fill_falloff_1 = new System.Windows.Forms.RadioButton();
            this.fill_normal = new System.Windows.Forms.RadioButton();
            this.label2 = new System.Windows.Forms.Label();
            this.charset = new System.Windows.Forms.ComboBox();
            this.bold = new System.Windows.Forms.CheckBox();
            this.italic = new System.Windows.Forms.CheckBox();
            this.fontsize = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.button3 = new System.Windows.Forms.Button();
            this.fading_indicator = new System.Windows.Forms.Label();
            this.fading = new System.Windows.Forms.NumericUpDown();
            this.label29 = new System.Windows.Forms.Label();
            this.fontlist = new System.Windows.Forms.ListBox();
            this.font_up = new System.Windows.Forms.Button();
            this.button_down = new System.Windows.Forms.Button();
            this.letterspacing = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.preview)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fading)).BeginInit();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(16, 15);
            this.button1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(100, 28);
            this.button1.TabIndex = 1;
            this.button1.Text = "Add";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // button2
            // 
            this.button2.Location = new System.Drawing.Point(269, 15);
            this.button2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(100, 28);
            this.button2.TabIndex = 2;
            this.button2.Text = "Remove";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // preview
            // 
            this.preview.BackColor = System.Drawing.SystemColors.ControlDark;
            this.preview.Location = new System.Drawing.Point(412, 50);
            this.preview.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.preview.Name = "preview";
            this.preview.Size = new System.Drawing.Size(351, 181);
            this.preview.TabIndex = 3;
            this.preview.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(563, 27);
            this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(57, 17);
            this.label1.TabIndex = 4;
            this.label1.Text = "Preview";
            // 
            // font
            // 
            this.font.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.font.FormattingEnabled = true;
            this.font.Location = new System.Drawing.Point(16, 287);
            this.font.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.font.Name = "font";
            this.font.Size = new System.Drawing.Size(217, 23);
            this.font.TabIndex = 5;
            this.font.SelectedIndexChanged += new System.EventHandler(this.font_SelectedIndexChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ol_shadow_ab);
            this.groupBox1.Controls.Add(this.ol_shadow_b);
            this.groupBox1.Controls.Add(this.ol_shadow_a);
            this.groupBox1.Controls.Add(this.ol_none);
            this.groupBox1.Location = new System.Drawing.Point(269, 247);
            this.groupBox1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox1.Size = new System.Drawing.Size(267, 139);
            this.groupBox1.TabIndex = 6;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Outline";
            // 
            // ol_shadow_ab
            // 
            this.ol_shadow_ab.AutoSize = true;
            this.ol_shadow_ab.Location = new System.Drawing.Point(8, 108);
            this.ol_shadow_ab.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.ol_shadow_ab.Name = "ol_shadow_ab";
            this.ol_shadow_ab.Size = new System.Drawing.Size(109, 21);
            this.ol_shadow_ab.TabIndex = 2;
            this.ol_shadow_ab.TabStop = true;
            this.ol_shadow_ab.Text = "Shadow A+B";
            this.ol_shadow_ab.UseVisualStyleBackColor = true;
            this.ol_shadow_ab.CheckedChanged += new System.EventHandler(this.ol_shadow_ab_CheckedChanged);
            // 
            // ol_shadow_b
            // 
            this.ol_shadow_b.AutoSize = true;
            this.ol_shadow_b.Location = new System.Drawing.Point(8, 80);
            this.ol_shadow_b.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.ol_shadow_b.Name = "ol_shadow_b";
            this.ol_shadow_b.Size = new System.Drawing.Size(92, 21);
            this.ol_shadow_b.TabIndex = 1;
            this.ol_shadow_b.TabStop = true;
            this.ol_shadow_b.Text = "Shadow B";
            this.ol_shadow_b.UseVisualStyleBackColor = true;
            this.ol_shadow_b.CheckedChanged += new System.EventHandler(this.ol_shadow_b_CheckedChanged);
            // 
            // ol_shadow_a
            // 
            this.ol_shadow_a.AutoSize = true;
            this.ol_shadow_a.Location = new System.Drawing.Point(8, 52);
            this.ol_shadow_a.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.ol_shadow_a.Name = "ol_shadow_a";
            this.ol_shadow_a.Size = new System.Drawing.Size(92, 21);
            this.ol_shadow_a.TabIndex = 1;
            this.ol_shadow_a.TabStop = true;
            this.ol_shadow_a.Text = "Shadow A";
            this.ol_shadow_a.UseVisualStyleBackColor = true;
            this.ol_shadow_a.CheckedChanged += new System.EventHandler(this.ol_shadow_a_CheckedChanged);
            // 
            // ol_none
            // 
            this.ol_none.AutoSize = true;
            this.ol_none.Location = new System.Drawing.Point(8, 23);
            this.ol_none.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.ol_none.Name = "ol_none";
            this.ol_none.Size = new System.Drawing.Size(63, 21);
            this.ol_none.TabIndex = 0;
            this.ol_none.TabStop = true;
            this.ol_none.Text = "None";
            this.ol_none.UseVisualStyleBackColor = true;
            this.ol_none.CheckedChanged += new System.EventHandler(this.ol_none_CheckedChanged);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.sh_75);
            this.groupBox2.Controls.Add(this.sh_50);
            this.groupBox2.Controls.Add(this.sh_25);
            this.groupBox2.Controls.Add(this.sh_none);
            this.groupBox2.Location = new System.Drawing.Point(269, 402);
            this.groupBox2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox2.Size = new System.Drawing.Size(107, 133);
            this.groupBox2.TabIndex = 7;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Shadow";
            // 
            // sh_75
            // 
            this.sh_75.AutoSize = true;
            this.sh_75.Location = new System.Drawing.Point(8, 105);
            this.sh_75.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.sh_75.Name = "sh_75";
            this.sh_75.Size = new System.Drawing.Size(61, 21);
            this.sh_75.TabIndex = 3;
            this.sh_75.TabStop = true;
            this.sh_75.Text = "75 %";
            this.sh_75.UseVisualStyleBackColor = true;
            this.sh_75.CheckedChanged += new System.EventHandler(this.sh_75_CheckedChanged);
            // 
            // sh_50
            // 
            this.sh_50.AutoSize = true;
            this.sh_50.Location = new System.Drawing.Point(8, 80);
            this.sh_50.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.sh_50.Name = "sh_50";
            this.sh_50.Size = new System.Drawing.Size(61, 21);
            this.sh_50.TabIndex = 2;
            this.sh_50.TabStop = true;
            this.sh_50.Text = "50 %";
            this.sh_50.UseVisualStyleBackColor = true;
            this.sh_50.CheckedChanged += new System.EventHandler(this.sh_50_CheckedChanged);
            // 
            // sh_25
            // 
            this.sh_25.AutoSize = true;
            this.sh_25.Location = new System.Drawing.Point(8, 52);
            this.sh_25.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.sh_25.Name = "sh_25";
            this.sh_25.Size = new System.Drawing.Size(61, 21);
            this.sh_25.TabIndex = 1;
            this.sh_25.TabStop = true;
            this.sh_25.Text = "25 %";
            this.sh_25.UseVisualStyleBackColor = true;
            this.sh_25.CheckedChanged += new System.EventHandler(this.sh_25_CheckedChanged);
            // 
            // sh_none
            // 
            this.sh_none.AutoSize = true;
            this.sh_none.Location = new System.Drawing.Point(8, 23);
            this.sh_none.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.sh_none.Name = "sh_none";
            this.sh_none.Size = new System.Drawing.Size(63, 21);
            this.sh_none.TabIndex = 0;
            this.sh_none.TabStop = true;
            this.sh_none.Text = "None";
            this.sh_none.UseVisualStyleBackColor = true;
            this.sh_none.CheckedChanged += new System.EventHandler(this.sh_none_CheckedChanged);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.fill_scanlines);
            this.groupBox3.Controls.Add(this.fill_falloff_2);
            this.groupBox3.Controls.Add(this.fill_falloff_1);
            this.groupBox3.Controls.Add(this.fill_normal);
            this.groupBox3.Location = new System.Drawing.Point(384, 402);
            this.groupBox3.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Padding = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.groupBox3.Size = new System.Drawing.Size(152, 133);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Fill";
            // 
            // fill_scanlines
            // 
            this.fill_scanlines.AutoSize = true;
            this.fill_scanlines.Location = new System.Drawing.Point(8, 105);
            this.fill_scanlines.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fill_scanlines.Name = "fill_scanlines";
            this.fill_scanlines.Size = new System.Drawing.Size(90, 21);
            this.fill_scanlines.TabIndex = 3;
            this.fill_scanlines.TabStop = true;
            this.fill_scanlines.Text = "Scanlines";
            this.fill_scanlines.UseVisualStyleBackColor = true;
            this.fill_scanlines.CheckedChanged += new System.EventHandler(this.fill_scanlines_CheckedChanged);
            // 
            // fill_falloff_2
            // 
            this.fill_falloff_2.AutoSize = true;
            this.fill_falloff_2.Location = new System.Drawing.Point(8, 80);
            this.fill_falloff_2.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fill_falloff_2.Name = "fill_falloff_2";
            this.fill_falloff_2.Size = new System.Drawing.Size(79, 21);
            this.fill_falloff_2.TabIndex = 2;
            this.fill_falloff_2.TabStop = true;
            this.fill_falloff_2.Text = "Falloff 2";
            this.fill_falloff_2.UseVisualStyleBackColor = true;
            this.fill_falloff_2.CheckedChanged += new System.EventHandler(this.fill_falloff_2_CheckedChanged);
            // 
            // fill_falloff_1
            // 
            this.fill_falloff_1.AutoSize = true;
            this.fill_falloff_1.Location = new System.Drawing.Point(8, 52);
            this.fill_falloff_1.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fill_falloff_1.Name = "fill_falloff_1";
            this.fill_falloff_1.Size = new System.Drawing.Size(79, 21);
            this.fill_falloff_1.TabIndex = 1;
            this.fill_falloff_1.TabStop = true;
            this.fill_falloff_1.Text = "Falloff 1";
            this.fill_falloff_1.UseVisualStyleBackColor = true;
            this.fill_falloff_1.CheckedChanged += new System.EventHandler(this.fill_falloff_1_CheckedChanged);
            // 
            // fill_normal
            // 
            this.fill_normal.AutoSize = true;
            this.fill_normal.Location = new System.Drawing.Point(8, 23);
            this.fill_normal.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fill_normal.Name = "fill_normal";
            this.fill_normal.Size = new System.Drawing.Size(74, 21);
            this.fill_normal.TabIndex = 0;
            this.fill_normal.TabStop = true;
            this.fill_normal.Text = "Normal";
            this.fill_normal.UseVisualStyleBackColor = true;
            this.fill_normal.CheckedChanged += new System.EventHandler(this.fill_normal_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(16, 327);
            this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(61, 17);
            this.label2.TabIndex = 8;
            this.label2.Text = "Charset:";
            // 
            // charset
            // 
            this.charset.FormattingEnabled = true;
            this.charset.Location = new System.Drawing.Point(16, 347);
            this.charset.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.charset.Name = "charset";
            this.charset.Size = new System.Drawing.Size(217, 24);
            this.charset.TabIndex = 5;
            this.charset.SelectedIndexChanged += new System.EventHandler(this.charset_SelectedIndexChanged);
            // 
            // bold
            // 
            this.bold.AutoSize = true;
            this.bold.Location = new System.Drawing.Point(39, 433);
            this.bold.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.bold.Name = "bold";
            this.bold.Size = new System.Drawing.Size(58, 21);
            this.bold.TabIndex = 9;
            this.bold.Text = "Bold";
            this.bold.UseVisualStyleBackColor = true;
            this.bold.CheckedChanged += new System.EventHandler(this.bold_CheckedChanged);
            // 
            // italic
            // 
            this.italic.AutoSize = true;
            this.italic.Location = new System.Drawing.Point(39, 462);
            this.italic.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.italic.Name = "italic";
            this.italic.Size = new System.Drawing.Size(58, 21);
            this.italic.TabIndex = 10;
            this.italic.Text = "Italic";
            this.italic.UseVisualStyleBackColor = true;
            this.italic.CheckedChanged += new System.EventHandler(this.italic_CheckedChanged);
            // 
            // fontsize
            // 
            this.fontsize.FormattingEnabled = true;
            this.fontsize.Location = new System.Drawing.Point(128, 402);
            this.fontsize.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fontsize.Name = "fontsize";
            this.fontsize.Size = new System.Drawing.Size(91, 24);
            this.fontsize.TabIndex = 11;
            this.fontsize.SelectedIndexChanged += new System.EventHandler(this.fontsize_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(153, 382);
            this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(35, 17);
            this.label3.TabIndex = 12;
            this.label3.Text = "Size";
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(609, 383);
            this.button3.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(100, 28);
            this.button3.TabIndex = 13;
            this.button3.Text = "OK";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // fading_indicator
            // 
            this.fading_indicator.AutoSize = true;
            this.fading_indicator.Location = new System.Drawing.Point(71, 539);
            this.fading_indicator.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.fading_indicator.Name = "fading_indicator";
            this.fading_indicator.Size = new System.Drawing.Size(56, 17);
            this.fading_indicator.TabIndex = 16;
            this.fading_indicator.Text = "<none>";
            // 
            // fading
            // 
            this.fading.Location = new System.Drawing.Point(75, 511);
            this.fading.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fading.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
            this.fading.Name = "fading";
            this.fading.Size = new System.Drawing.Size(160, 22);
            this.fading.TabIndex = 15;
            this.fading.ValueChanged += new System.EventHandler(this.fading_ValueChanged);
            // 
            // label29
            // 
            this.label29.AutoSize = true;
            this.label29.Location = new System.Drawing.Point(11, 513);
            this.label29.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
            this.label29.Name = "label29";
            this.label29.Size = new System.Drawing.Size(55, 17);
            this.label29.TabIndex = 14;
            this.label29.Text = "Fading:";
            // 
            // fontlist
            // 
            this.fontlist.FormattingEnabled = true;
            this.fontlist.ItemHeight = 16;
            this.fontlist.Location = new System.Drawing.Point(15, 50);
            this.fontlist.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.fontlist.Name = "fontlist";
            this.fontlist.Size = new System.Drawing.Size(352, 180);
            this.fontlist.TabIndex = 17;
            // 
            // font_up
            // 
            this.font_up.Location = new System.Drawing.Point(372, 101);
            this.font_up.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.font_up.Name = "font_up";
            this.font_up.Size = new System.Drawing.Size(32, 28);
            this.font_up.TabIndex = 18;
            this.font_up.Text = "up";
            this.font_up.UseVisualStyleBackColor = true;
            this.font_up.Click += new System.EventHandler(this.font_up_Click);
            // 
            // button_down
            // 
            this.button_down.Location = new System.Drawing.Point(372, 137);
            this.button_down.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.button_down.Name = "button_down";
            this.button_down.Size = new System.Drawing.Size(32, 28);
            this.button_down.TabIndex = 19;
            this.button_down.Text = "down";
            this.button_down.UseVisualStyleBackColor = true;
            this.button_down.Click += new System.EventHandler(this.button_down_Click);
            // 
            // letterspacing
            // 
            this.letterspacing.FormattingEnabled = true;
            this.letterspacing.Location = new System.Drawing.Point(128, 460);
            this.letterspacing.Name = "letterspacing";
            this.letterspacing.Size = new System.Drawing.Size(91, 24);
            this.letterspacing.TabIndex = 20;
            this.letterspacing.SelectedIndexChanged += new System.EventHandler(this.letterspacing_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(125, 440);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(98, 17);
            this.label4.TabIndex = 21;
            this.label4.Text = "Letter spacing";
            // 
            // Fonts
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(779, 569);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.letterspacing);
            this.Controls.Add(this.button_down);
            this.Controls.Add(this.font_up);
            this.Controls.Add(this.fontlist);
            this.Controls.Add(this.fading_indicator);
            this.Controls.Add(this.fading);
            this.Controls.Add(this.label29);
            this.Controls.Add(this.button3);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.fontsize);
            this.Controls.Add(this.italic);
            this.Controls.Add(this.bold);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.charset);
            this.Controls.Add(this.font);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.preview);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.button1);
            this.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
            this.Name = "Fonts";
            this.Text = "Fonts";
            ((System.ComponentModel.ISupportInitialize)(this.preview)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fading)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

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
        private System.Windows.Forms.ComboBox charset;
        private System.Windows.Forms.CheckBox bold;
        private System.Windows.Forms.CheckBox italic;
        private System.Windows.Forms.ComboBox fontsize;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Label fading_indicator;
        private System.Windows.Forms.NumericUpDown fading;
        private System.Windows.Forms.Label label29;
        private System.Windows.Forms.ListBox fontlist;
        private System.Windows.Forms.Button font_up;
        private System.Windows.Forms.Button button_down;
        private System.Windows.Forms.ComboBox letterspacing;
        private System.Windows.Forms.Label label4;
    }
}