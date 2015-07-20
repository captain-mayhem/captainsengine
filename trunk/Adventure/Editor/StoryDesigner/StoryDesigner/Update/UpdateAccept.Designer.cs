namespace StoryDesigner
{
    partial class UpdateAccept
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UpdateAccept));
            this.pictureBox = new System.Windows.Forms.PictureBox();
            this.lblUpdateAvail = new System.Windows.Forms.Label();
            this.lblNewVersion = new System.Windows.Forms.Label();
            this.btnYes = new System.Windows.Forms.Button();
            this.btnNo = new System.Windows.Forms.Button();
            this.btnDetails = new System.Windows.Forms.Button();
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
            // lblUpdateAvail
            // 
            this.lblUpdateAvail.AccessibleDescription = null;
            this.lblUpdateAvail.AccessibleName = null;
            resources.ApplyResources(this.lblUpdateAvail, "lblUpdateAvail");
            this.lblUpdateAvail.Name = "lblUpdateAvail";
            // 
            // lblNewVersion
            // 
            this.lblNewVersion.AccessibleDescription = null;
            this.lblNewVersion.AccessibleName = null;
            resources.ApplyResources(this.lblNewVersion, "lblNewVersion");
            this.lblNewVersion.Name = "lblNewVersion";
            // 
            // btnYes
            // 
            this.btnYes.AccessibleDescription = null;
            this.btnYes.AccessibleName = null;
            resources.ApplyResources(this.btnYes, "btnYes");
            this.btnYes.BackgroundImage = null;
            this.btnYes.Font = null;
            this.btnYes.Name = "btnYes";
            this.btnYes.UseVisualStyleBackColor = true;
            this.btnYes.Click += new System.EventHandler(this.btnYes_Click);
            // 
            // btnNo
            // 
            this.btnNo.AccessibleDescription = null;
            this.btnNo.AccessibleName = null;
            resources.ApplyResources(this.btnNo, "btnNo");
            this.btnNo.BackgroundImage = null;
            this.btnNo.Font = null;
            this.btnNo.Name = "btnNo";
            this.btnNo.UseVisualStyleBackColor = true;
            this.btnNo.Click += new System.EventHandler(this.btnNo_Click);
            // 
            // btnDetails
            // 
            this.btnDetails.AccessibleDescription = null;
            this.btnDetails.AccessibleName = null;
            resources.ApplyResources(this.btnDetails, "btnDetails");
            this.btnDetails.BackgroundImage = null;
            this.btnDetails.Font = null;
            this.btnDetails.Name = "btnDetails";
            this.btnDetails.UseVisualStyleBackColor = true;
            this.btnDetails.Click += new System.EventHandler(this.btnDetails_Click);
            // 
            // UpdateAccept
            // 
            this.AccessibleDescription = null;
            this.AccessibleName = null;
            resources.ApplyResources(this, "$this");
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = null;
            this.Controls.Add(this.btnDetails);
            this.Controls.Add(this.btnNo);
            this.Controls.Add(this.btnYes);
            this.Controls.Add(this.lblNewVersion);
            this.Controls.Add(this.lblUpdateAvail);
            this.Controls.Add(this.pictureBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = null;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "UpdateAccept";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox;
        private System.Windows.Forms.Label lblUpdateAvail;
        private System.Windows.Forms.Label lblNewVersion;
        private System.Windows.Forms.Button btnYes;
        private System.Windows.Forms.Button btnNo;
        private System.Windows.Forms.Button btnDetails;
    }
}