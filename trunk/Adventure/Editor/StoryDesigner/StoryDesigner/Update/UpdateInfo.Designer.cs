namespace StoryDesigner
{
    partial class UpdateInfo
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UpdateInfo));
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.lblVersions = new System.Windows.Forms.Label();
            this.txtDescription = new System.Windows.Forms.RichTextBox();
            this.lblDescription = new System.Windows.Forms.Label();
            this.btnBack = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox
            // 
            this.pictureBox.AccessibleDescription = null;
            this.pictureBox.AccessibleName = null;
            resources.ApplyResources(this.pictureBox, "pictureBox");
            this.pictureBox.BackgroundImage = null;
            this.pictureBox.Font = null;
            this.pictureBox.Image = global::StoryDesigner.Properties.Resources.Update;
            this.pictureBox.ImageLocation = null;
            this.pictureBox.Name = "pictureBox";
            this.pictureBox.TabStop = false;
            // 
            // lblVersions
            // 
            this.lblVersions.AccessibleDescription = null;
            this.lblVersions.AccessibleName = null;
            resources.ApplyResources(this.lblVersions, "lblVersions");
            this.lblVersions.Name = "lblVersions";
            // 
            // txtDescription
            // 
            this.txtDescription.AccessibleDescription = null;
            this.txtDescription.AccessibleName = null;
            resources.ApplyResources(this.txtDescription, "txtDescription");
            this.txtDescription.BackColor = System.Drawing.SystemColors.Control;
            this.txtDescription.BackgroundImage = null;
            this.txtDescription.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.txtDescription.Cursor = System.Windows.Forms.Cursors.Default;
            this.txtDescription.Name = "txtDescription";
            this.txtDescription.ReadOnly = true;
            this.txtDescription.TabStop = false;
            this.txtDescription.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txtDescription_KeyDown);
            // 
            // lblDescription
            // 
            this.lblDescription.AccessibleDescription = null;
            this.lblDescription.AccessibleName = null;
            resources.ApplyResources(this.lblDescription, "lblDescription");
            this.lblDescription.Name = "lblDescription";
            // 
            // btnBack
            // 
            this.btnBack.AccessibleDescription = null;
            this.btnBack.AccessibleName = null;
            resources.ApplyResources(this.btnBack, "btnBack");
            this.btnBack.BackgroundImage = null;
            this.btnBack.Name = "btnBack";
            this.btnBack.TabStop = false;
            this.btnBack.UseVisualStyleBackColor = true;
            this.btnBack.Click += new System.EventHandler(this.btnBack_Click);
            // 
            // UpdateInfo
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add(this.btnBack);
            this.Controls.Add(this.lblDescription);
            this.Controls.Add(this.txtDescription);
            this.Controls.Add(this.lblVersions);
            this.Controls.Add(this.pictureBox);
            this.Font = null;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = null;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UpdateInfo";
            this.ShowInTaskbar = false;
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.Label lblVersions;
        private System.Windows.Forms.RichTextBox txtDescription;
        private System.Windows.Forms.Label lblDescription;
        private System.Windows.Forms.Button btnBack;
    }
}