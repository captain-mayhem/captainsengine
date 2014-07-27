using System;
using System.Windows.Forms;

namespace StoryDesigner
{
    internal partial class UpdateAccept : Form
    {
        private Updatable applicationInfo;
        private UpdateXml updateInfo;
        private UpdateInfo updateUnfoForm;

        internal UpdateAccept(Updatable applicationInfo, UpdateXml updateInfo)
        {
            InitializeComponent();

            this.applicationInfo = applicationInfo;
            this.updateInfo = updateInfo;

            this.Text = applicationInfo.ApplicationName+ " - Update Available";

            if (applicationInfo.ApplicationIcon != null)
                this.Icon = applicationInfo.ApplicationIcon;

            lblNewVersion.Text = string.Format("New Version: {0}", updateInfo.Version.ToString());
        }

        private void btnYes_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Yes;
            this.Close();
        }

        private void btnNo_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.No;
            this.Close();
        }

        private void btnDetails_Click(object sender, EventArgs e)
        {
            if (updateUnfoForm == null)
                updateUnfoForm = new UpdateInfo(applicationInfo, updateInfo);
            updateUnfoForm.ShowDialog(this);
        }
    }
}
