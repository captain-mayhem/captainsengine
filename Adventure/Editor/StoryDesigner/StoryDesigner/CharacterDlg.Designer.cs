namespace StoryDesigner
{
    partial class CharacterDlg
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
            this.text_color = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.stateFrameImage1 = new StoryDesigner.StateFrameImage();
            this.SuspendLayout();
            // 
            // text_color
            // 
            this.text_color.BackColor = System.Drawing.Color.Lime;
            this.text_color.Location = new System.Drawing.Point(308, 26);
            this.text_color.Name = "text_color";
            this.text_color.Size = new System.Drawing.Size(23, 23);
            this.text_color.TabIndex = 2;
            this.text_color.UseVisualStyleBackColor = false;
            this.text_color.Click += new System.EventHandler(this.text_color_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(250, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "TextColor";
            // 
            // stateFrameImage1
            // 
            this.stateFrameImage1.AutoSize = true;
            this.stateFrameImage1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.stateFrameImage1.Data = null;
            this.stateFrameImage1.Hotspot = true;
            this.stateFrameImage1.HotspotCoordScale = 1F;
            this.stateFrameImage1.Location = new System.Drawing.Point(12, 12);
            this.stateFrameImage1.Name = "stateFrameImage1";
            this.stateFrameImage1.PictureBoxSize = new System.Drawing.Size(120, 200);
            this.stateFrameImage1.ScaleImageToBox = false;
            this.stateFrameImage1.ShowAnimationButton = true;
            this.stateFrameImage1.ShowImageNames = true;
            this.stateFrameImage1.ShowStateDropDown = true;
            this.stateFrameImage1.Size = new System.Drawing.Size(330, 424);
            this.stateFrameImage1.TabIndex = 0;
            // 
            // CharacterDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(355, 463);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.text_color);
            this.Controls.Add(this.stateFrameImage1);
            this.Name = "CharacterDlg";
            this.Text = "CharacterDlg";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private StateFrameImage stateFrameImage1;
        private System.Windows.Forms.Button text_color;
        private System.Windows.Forms.Label label1;
    }
}