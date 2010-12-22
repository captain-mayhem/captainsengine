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
            this.actionTextHeight.Value = mData.Settings.ActionTextHeight;
            this.silentDelete.Checked = mData.Settings.SilentDelete;
            this.infoLine.Checked = mData.Settings.InfoLine;
            this.fitImages.Checked = true; //TODO
            this.createBackups.Checked = true; //TODO
            this.protectGamefiles.Checked = mData.Settings.ProtectGameFile;
            this.loadingImage.Text = mData.Settings.LoadingImage;
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
            this.taskbar.Checked = mData.Settings.ShowTaskbar;
            this.taskBarHeight.Value = mData.Settings.TaskHeight;
            this.taskbar_room.Text = mData.Settings.TaskRoom;
            switch (mData.Settings.TsStyle)
            {
                case 0:
                    this.ts_always.Checked = true;
                    break;
                case 1:
                    this.ts_popup.Checked = true;
                    break;
                case 2:
                    this.ts_scrolling.Checked = true;
                    break;
            }
            this.hideCompletely.Checked = mData.Settings.TaskHideCompletely;
            if (mData.Settings.TaskbarFromTop)
            {
                ts_appears_top.Checked = true;
            }
            else
            {
                ts_appear_bottom.Checked = true;
            }
            this.anywhere_room.Text = mData.Settings.AnywhereRoom;
            this.anywhere_transparency.Value = mData.Settings.AnywhereTransparency;
            //third page
            //this.border_color.BackColor = Color.FromArgb((int)(((System.UInt32)mData.Settings.BorderColor >> 8) & 0xffffff | ((System.UInt32)mData.Settings.BorderColor << 24) & 0xff000000));
            this.border_color.BackColor = Color.FromArgb(convertColor(mData.Settings.BorderColor));
            this.background_color.BackColor = Color.FromArgb(convertColor(mData.Settings.BackgroundColor));
            this.text_color.BackColor = Color.FromArgb(convertColor(mData.Settings.TextColor));
            this.useCustomMenu.Checked = mData.Settings.CustomMenu;
            this.customMenu.Text = mData.Settings.CustomMenuRoom;
            this.menu_fading.Value = mData.Settings.MenuFading;
            this.offspeech_color.BackColor = Color.FromArgb(convertColor(mData.Settings.OffspeechColor));
            this.infotext_color.BackColor = Color.FromArgb(convertColor(mData.Settings.InfotextColor));
            this.transparent_color.BackColor = Color.FromArgb(convertColor(mData.Settings.TargaColor));
            //fourth page
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

        private int convertColor(System.UInt32 color)
        {
            System.UInt32 ret = 0xFF000000;
            ret |= ((System.UInt32)color & 0xFF) << 16;
            ret |= (System.UInt32)color & 0xFF00;
            ret |= ((System.UInt32)color & 0xFF0000) >> 16;
            return (int)ret;
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

        private void taskbar_CheckedChanged(object sender, EventArgs e)
        {
            if (this.taskbar.Checked)
            {
                this.taskbar_panel.Enabled = true;
                this.tsstylebox.Enabled = true;
            }
            else
            {
                this.taskbar_panel.Enabled = false;
                this.tsstylebox.Enabled = false;
            }
        }

        private void useCustomMenu_CheckedChanged(object sender, EventArgs e)
        {
            if (this.useCustomMenu.Checked)
                this.customMenu.Enabled = true;
            else
                this.customMenu.Enabled = false;
        }

        private void menu_fading_ValueChanged(object sender, EventArgs e)
        {
            setFadingIndicator((NumericUpDown)sender, this.fading_indicator);
        }

        private void textscene_fading_ValueChanged(object sender, EventArgs e)
        {
            setFadingIndicator((NumericUpDown)sender, this.textscene_fading_indicator);
        }

        private void setFadingIndicator(NumericUpDown num, Label fading_indicator)
        {
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

        private void use_background_image_CheckedChanged(object sender, EventArgs e)
        {
            if (use_background_image.Checked)
                ts_background_image.Enabled = true;
            else
                ts_background_image.Enabled = false;
        }
    }
}