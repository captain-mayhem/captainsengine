namespace StoryDesigner
{
    partial class ObjectDlg
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
            this.stateFrameImage1 = new StoryDesigner.StateFrameImage();
            this.SuspendLayout();
            // 
            // stateFrameImage1
            // 
            this.stateFrameImage1.Data = null;
            this.stateFrameImage1.Hotspot = false;
            this.stateFrameImage1.HotspotCoordScale = 1F;
            this.stateFrameImage1.Location = new System.Drawing.Point(12, 12);
            this.stateFrameImage1.Name = "stateFrameImage1";
            this.stateFrameImage1.PictureBoxSize = new System.Drawing.Size(100, 100);
            this.stateFrameImage1.ScaleImageToBox = false;
            this.stateFrameImage1.ShowAnimationButton = true;
            this.stateFrameImage1.ShowImageNames = true;
            this.stateFrameImage1.Size = new System.Drawing.Size(331, 296);
            this.stateFrameImage1.TabIndex = 0;
            // 
            // ObjectDlg
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(357, 338);
            this.Controls.Add(this.stateFrameImage1);
            this.Name = "ObjectDlg";
            this.Text = "ObjectDlg";
            this.ResumeLayout(false);

        }

        #endregion

        private StateFrameImage stateFrameImage1;
    }
}