using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    internal partial class UpdateInfo : Form
    {
        public UpdateInfo(Updatable[] applicationInfos, UpdateXml[] updateInfos)
        {
            InitializeComponent();

            if (applicationInfos[0].ApplicationIcon != null)
                this.Icon = applicationInfos[0].ApplicationIcon;

            int firstUpdatable = 0;
            for (int i = 0; i < updateInfos.Length; ++i)
            {
                if (updateInfos[i].ShouldUpdate)
                {
                    firstUpdatable = i;
                    break;
                }
            }

            this.Text = applicationInfos[firstUpdatable].ApplicationName + " - Update Info";
            this.lblVersions.Text = String.Format("Current Version: {0}\n Update Version: {1}",
                applicationInfos[firstUpdatable].ApplicationVersion.ToString(),
                updateInfos[firstUpdatable].Version.ToString());
            this.txtDescription.Text = "";
            for (int i = 0; i < updateInfos.Length; ++i){
                UpdateXml updateInfo = updateInfos[i];
                if (updateInfo.ShouldUpdate)
                {
                    this.txtDescription.Text += applicationInfos[i].ApplicationName + "\n";
                    this.txtDescription.Text += updateInfo.Description + "\n\n";
                }
            }
        }

        private void btnBack_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void txtDescription_KeyDown(object sender, KeyEventArgs e)
        {
            if (!(e.Control && e.KeyCode == Keys.C))
                e.SuppressKeyPress = true;
        }
    }
}
