using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class Fonts : Form
    {
        public Fonts(AdvData data)
        {
            InitializeComponent();
            System.Drawing.Text.InstalledFontCollection fonts = new System.Drawing.Text.InstalledFontCollection();
            foreach (FontFamily fam in fonts.Families)
            {
                font.Items.Add(fam.Name);
            }
            mFonts = data.Settings.Fonts;
            fontlist.SelectedIndexChanged += new EventHandler(fontlist_SelectedIndexChanged);

            foreach (Object o in mFonts)
            {
                fontlist.Items.Add(((FontInfo)o).name);
            }

            charset.Items.Add("Western");
            charset.Items.Add("East european");
            charset.Items.Add("Russian");
            charset.Items.Add("Greek");
            charset.Items.Add("Turkish");
            charset.Items.Add("Arabic");
            charset.Items.Add("Hebrew");
            charset.Items.Add("Vietnamese");
            charset.Items.Add("Thai");
            charset.Items.Add("Baltic");
            charset.Items.Add("Japanese ShiftJIS");
            charset.Items.Add("Dos OEM");
            charset.Items.Add("Symbol");

            for (int i = 6; i <= 28; ++i)
            {
                fontsize.Items.Add(Convert.ToString(i));
            }

            fontlist.SelectedIndex = 0;
        }

        void fontlist_SelectedIndexChanged(object sender, EventArgs e)
        {
            FontInfo info = (FontInfo)mFonts[fontlist.SelectedIndex];
            font.SelectedItem = info.name;
            charset.SelectedIndex = info.charset;
            bold.Checked = info.bold;
            italic.Checked = info.italic;
            fontsize.SelectedItem = Convert.ToString(info.size);
            switch (info.outline)
            {
                case 0:
                    ol_none.Checked = true;
                    break;
                case 1:
                    ol_shadow_a.Checked = true;
                    break;
                case 2:
                    ol_shadow_b.Checked = true;
                    break;
                case 3:
                    ol_shadow_ab.Checked = true;
                    break;
            }
            switch (info.shadow)
            {
                case 0:
                    sh_none.Checked = true;
                    break;
                case 1:
                    sh_25.Checked = true;
                    break;
                case 2:
                    sh_50.Checked = true;
                    break;
                case 3:
                    sh_75.Checked = true;
                    break;
            }
            switch (info.fill)
            {
                case 0:
                    fill_normal.Checked = true;
                    break;
                case 1:
                    fill_falloff_1.Checked = true;
                    break;
                case 2:
                    fill_falloff_2.Checked = true;
                    break;
                case 3:
                    fill_scanlines.Checked = true;
                    break;
            }
            fading.Value = info.fading;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
        }

        System.Collections.ArrayList mFonts;

        private void fading_ValueChanged(object sender, EventArgs e)
        {
            NumericUpDown num = fading;
            if (num.Value > 1600)
                fading_indicator.Text = "really slow";
            else if (num.Value > 1200)
                fading_indicator.Text = "slow";
            else if (num.Value > 800)
                fading_indicator.Text = "average";
            else if (num.Value > 400)
                fading_indicator.Text = "fast";
            else if (num.Value > 0)
                fading_indicator.Text = "very fast";
            if (num.Value == 0)
                fading_indicator.Text = "<none>";
        }
    }
}