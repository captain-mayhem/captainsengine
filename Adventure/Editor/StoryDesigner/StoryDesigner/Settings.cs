using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class Settings : Form
    {
        public Settings(AdvData data)
        {
            InitializeComponent();
            mData = data;
            setControls();
        }

        private void OK_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void setControls()
        {
            //first page
            this.projetName.Text = mData.Settings.Projectname;
            if (mData.Settings.Resolution.y == 300)
                this.resolution.SelectedIndex = 0;
            else if (mData.Settings.Resolution.y == 320)
                this.resolution.SelectedIndex = 1;
            else if (mData.Settings.Resolution.x == 640)
                this.resolution.SelectedIndex = 2;
            else if (mData.Settings.Resolution.x == 800)
                this.resolution.SelectedIndex = 3;
            else if (mData.Settings.Resolution.x == 1024)
                this.resolution.SelectedIndex = 4;
            this.no_antialiasing.Checked = mData.Settings.NotAntialiased;
            this.mute_music.Checked = mData.Settings.MuteMusicWhenSpeech;
            if (mData.Settings.GameIcon.Length != 0)
            {
                Bitmap bmp = (Bitmap)Bitmap.FromFile(mData.Settings.GameIcon);
                this.symbol.Image = bmp;
            }
            else
            {
                this.symbol.Image = null;
            }
            this.text_on.Checked = mData.Settings.TextOnOff;
            this.drawDraggedItems.Checked = mData.Settings.NotAntialiased;
            this.groupItems.Checked = mData.Settings.GroupItems;
            this.actionText.Checked = mData.Settings.ActionText;
            //TODO
            //second page
            this.start_script.Text = mData.Settings.Startscript;
            this.main_script.Text = mData.Settings.Mainscript;
            switch (mData.Settings.ScreenChange)
            {
                case ScreenChange.SC_DIRECT:
                    this.sc_direct.Checked = true;
                    break;
                case ScreenChange.SC_FADEOUT:
                    this.sc_blackfade.Checked = true;
                    break;
                case ScreenChange.SC_RECTANGLE:
                    this.sc_blackfade.Checked = true;
                    break;
                case ScreenChange.SC_CIRCLE:
                    this.sc_circle.Checked = true;
                    break;
                case ScreenChange.SC_CLOCK:
                    this.sc_clock.Checked = true;
                    break;
                case ScreenChange.SC_SHUTTERS:
                    this.sc_shutters.Checked = true;
                    break;
                case ScreenChange.SC_BLEND:
                    this.sc_blend.Checked = true;
                    break;
                case ScreenChange.SC_BLEND_SLOW:
                    this.sc_blend_slow.Checked = true;
                    break;
            }
        }

        private void chooseColor(object sender)
        {
            ColorDialog cd = new ColorDialog();
            if (cd.ShowDialog() == DialogResult.OK)
            {
                Button btn = (Button)sender;
                btn.BackColor = cd.Color;
            }
        }

        AdvData mData;

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Symbol|*.ico";
            fod.ShowDialog();
            if (fod.FileName.Length > 0)
            {
            }
        }

        private void border_color_Click(object sender, EventArgs e)
        {
            chooseColor(sender);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            chooseColor(sender);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            chooseColor(sender);
        }
    }
}