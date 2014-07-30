using System;
using System.Windows.Forms;

namespace StoryDesigner
{
    internal partial class UpdateAccept : Form
    {
        private Updatable[] applicationInfos;
        private UpdateXml[] updateInfos;
        private UpdateInfo updateUnfoForm;

        internal UpdateAccept(Updatable[] applicationInfos, UpdateXml[] updateInfos)
        {
            InitializeComponent();

            int firstUpdatable = 0;
            for (int i = 0; i < updateInfos.Length; ++i)
            {
                if (updateInfos[i].ShouldUpdate)
                {
                    firstUpdatable = i;
                    break;
                }
            }

            this.applicationInfos = applicationInfos;
            this.updateInfos = updateInfos;

            this.Text = applicationInfos[firstUpdatable].ApplicationName+ Strings.updateAvail;

            if (applicationInfos[0].ApplicationIcon != null)
                this.Icon = applicationInfos[0].ApplicationIcon;

            lblNewVersion.Text = string.Format(Strings.newVersion, updateInfos[firstUpdatable].Version.ToString());
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
                updateUnfoForm = new UpdateInfo(applicationInfos, updateInfos);
            updateUnfoForm.ShowDialog(this);
        }
    }
}
