namespace StoryDesigner
{
    partial class MouseIcons
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
            this.stateFrameImage1.Location = new System.Drawing.Point(12, 12);
            this.stateFrameImage1.Name = "stateFrameImage1";
            this.stateFrameImage1.Size = new System.Drawing.Size(331, 238);
            this.stateFrameImage1.TabIndex = 0;
            // 
            // MouseIcons
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(355, 262);
            this.Controls.Add(this.stateFrameImage1);
            this.Name = "MouseIcons";
            this.Text = "MouseIcons";
            this.ResumeLayout(false);

        }

        #endregion

        private StateFrameImage stateFrameImage1;

    }
}