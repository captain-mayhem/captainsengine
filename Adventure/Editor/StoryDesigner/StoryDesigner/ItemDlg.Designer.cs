namespace StoryDesigner
{
    partial class ItemDlg
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
            this.stateFrameImage1 = new StoryDesigner.StateFrameImage();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(138, 264);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 1;
            this.button1.Text = "Edit script";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // stateFrameImage1
            // 
            this.stateFrameImage1.AdaptFirstDrop = false;
            this.stateFrameImage1.AutoSize = true;
            this.stateFrameImage1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.stateFrameImage1.BackgroundColor = System.Drawing.SystemColors.Desktop;
            this.stateFrameImage1.Data = null;
            this.stateFrameImage1.FrameParts = 1;
            this.stateFrameImage1.Frames = 25;
            this.stateFrameImage1.Hotspot = false;
            this.stateFrameImage1.HotspotCoordScale = 1F;
            this.stateFrameImage1.Location = new System.Drawing.Point(12, 12);
            this.stateFrameImage1.Name = "stateFrameImage1";
            this.stateFrameImage1.PictureBoxSize = new System.Drawing.Size(50, 50);
            this.stateFrameImage1.ScaleImageToBox = true;
            this.stateFrameImage1.ShowAnimationButton = true;
            this.stateFrameImage1.ShowImageNames = true;
            this.stateFrameImage1.ShowStateDropDown = false;
            this.stateFrameImage1.Size = new System.Drawing.Size(339, 274);
            this.stateFrameImage1.TabIndex = 0;
            // 
            // ItemDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(356, 316);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.stateFrameImage1);
            this.Name = "ItemDlg";
            this.Text = "Item (none)";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private StateFrameImage stateFrameImage1;
        private System.Windows.Forms.Button button1;
    }
}