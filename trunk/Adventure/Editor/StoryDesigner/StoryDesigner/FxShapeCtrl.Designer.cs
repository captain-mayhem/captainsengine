namespace StoryDesigner
{
    partial class FxShapeCtrl
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
            this.activate = new System.Windows.Forms.CheckBox();
            this.effect = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.strength = new System.Windows.Forms.TrackBar();
            this.label2 = new System.Windows.Forms.Label();
            this.depOnRP = new System.Windows.Forms.CheckBox();
            this.position = new System.Windows.Forms.PictureBox();
            this.label3 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.strength)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.position)).BeginInit();
            this.SuspendLayout();
            // 
            // activate
            // 
            this.activate.AutoSize = true;
            this.activate.Location = new System.Drawing.Point(3, 3);
            this.activate.Name = "activate";
            this.activate.Size = new System.Drawing.Size(65, 17);
            this.activate.TabIndex = 0;
            this.activate.Text = "Activate";
            this.activate.UseVisualStyleBackColor = true;
            this.activate.CheckedChanged += new System.EventHandler(this.activate_CheckedChanged);
            // 
            // effect
            // 
            this.effect.FormattingEnabled = true;
            this.effect.Items.AddRange(new object[] {
            "Floor-Mirror",
            "Wall-Mirror",
            "Particle-Barrier"});
            this.effect.Location = new System.Drawing.Point(3, 39);
            this.effect.Name = "effect";
            this.effect.Size = new System.Drawing.Size(99, 21);
            this.effect.TabIndex = 1;
            this.effect.SelectedIndexChanged += new System.EventHandler(this.effect_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 23);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(60, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Use Effect:";
            // 
            // strength
            // 
            this.strength.Location = new System.Drawing.Point(3, 79);
            this.strength.Maximum = 255;
            this.strength.Name = "strength";
            this.strength.Size = new System.Drawing.Size(99, 45);
            this.strength.TabIndex = 3;
            this.strength.TickFrequency = 10;
            this.strength.Scroll += new System.EventHandler(this.strength_Scroll);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(3, 63);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(50, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Strength:";
            // 
            // depOnRP
            // 
            this.depOnRP.AutoSize = true;
            this.depOnRP.Location = new System.Drawing.Point(6, 118);
            this.depOnRP.Name = "depOnRP";
            this.depOnRP.Size = new System.Drawing.Size(158, 17);
            this.depOnRP.TabIndex = 5;
            this.depOnRP.Text = "Depending on room position";
            this.depOnRP.UseVisualStyleBackColor = true;
            this.depOnRP.CheckedChanged += new System.EventHandler(this.depOnRP_CheckedChanged);
            // 
            // position
            // 
            this.position.BackColor = System.Drawing.Color.Silver;
            this.position.Location = new System.Drawing.Point(117, 37);
            this.position.Name = "position";
            this.position.Size = new System.Drawing.Size(100, 75);
            this.position.TabIndex = 6;
            this.position.TabStop = false;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(114, 21);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Position:";
            // 
            // FxShapeCtrl
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.Controls.Add(this.label3);
            this.Controls.Add(this.position);
            this.Controls.Add(this.depOnRP);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.strength);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.effect);
            this.Controls.Add(this.activate);
            this.Name = "FxShapeCtrl";
            this.Size = new System.Drawing.Size(246, 200);
            ((System.ComponentModel.ISupportInitialize)(this.strength)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.position)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.CheckBox activate;
        private System.Windows.Forms.ComboBox effect;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TrackBar strength;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox depOnRP;
        private System.Windows.Forms.PictureBox position;
        private System.Windows.Forms.Label label3;
    }
}
