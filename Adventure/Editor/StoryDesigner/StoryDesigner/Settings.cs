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
            getControls();
            this.DialogResult = DialogResult.OK;
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
                mIcon = mData.Settings.GameIcon;
                try
                {
                    Bitmap bmp = (Bitmap)Bitmap.FromFile(mIcon);
                    this.symbol.Image = bmp;
                }
                catch (System.IO.FileNotFoundException)
                {
                    MessageBox.Show("Cannot find icon " + mIcon);
                    this.symbol.Image = null;
                }
            }
            else
            {
                mIcon = new string('a', 0);
                this.symbol.Image = null;
            }
            this.text_on.Checked = mData.Settings.TextOnOff;
            this.drawDraggedItems.Checked = mData.Settings.DrawDraggedItemIcons;
            this.groupItems.Checked = mData.Settings.GroupItems;
            this.actionText.Checked = mData.Settings.ActionText;
            this.actionTextHeight.Value = mData.Settings.ActionTextHeight;
            this.silentDelete.Checked = mData.Settings.SilentDelete;
            this.infoLine.Checked = mData.Settings.InfoLine;
            this.fitImages.Checked = mData.Persistence.FitImages;
            this.createBackups.Checked = mData.Persistence.CreateBackups;
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
            switch (mData.Settings.TaskPopup)
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
            this.border_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.BorderColor));
            this.background_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.BackgroundColor));
            this.text_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TextColor));
            this.useCustomMenu.Checked = mData.Settings.CustomMenu;
            this.customMenu.Text = mData.Settings.CustomMenuRoom;
            this.menu_fading.Value = mData.Settings.MenuFading;
            this.offspeech_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.OffspeechColor));
            this.infotext_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.InfotextColor));
            this.transparent_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TargaColor));
            //fourth page
            switch (mData.Settings.TsStyle)
            {
                case 0:
                    this.ts_solid.Checked = true;
                    break;
                case 1:
                    this.ts_transparent.Checked = true;
                    break;
                case 2:
                    this.ts_additive.Checked = true;
                    break;
                case 3:
                    this.ts_none.Checked = true;
                    break;
            }
            switch (mData.Settings.TsBorderStyle)
            {
                case 0:
                    this.ts_rectangles.Checked = true;
                    break;
                case 1:
                    this.ts_lines.Checked = true;
                    break;
                case 2:
                    this.ts_box.Checked = true;
                    break;
                case 3:
                    this.ts_border_none.Checked = true;
                    break;
            }
            this.textscene_fading.Value = mData.Settings.TextSceneFading;
            this.ts_area_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TsAreaColor));
            this.ts_border_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TsBorderColor));
            this.ts_text_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TsTextColor));
            this.ts_selection_color.BackColor = Color.FromArgb(Utilities.convertColor(mData.Settings.TsSelectionColor));
            if (mData.Settings.TsUseSymbols)
                this.ts_symbol.Checked = true;
            else
                this.ts_text.Checked = true;
            this.use_background_image.Checked = mData.Settings.TsUseBgImage;
            this.ts_background_image.Text = mData.Settings.TsBackground;
            //fifth page
            this.coin_interface.Checked = mData.Settings.CoinActivated;
            this.coin_room.Text = mData.Settings.CoinRoom;
            this.coin_autopopup.Checked = mData.Settings.CoinAutoPopup;
            this.coin_fading.Value = mData.Settings.CoinFading;
            //TODO crosshair
            //sixth page
            this.noPngToJpeg.Checked = mData.Settings.NoPngToJpeg;
        }

        private void getControls()
        {
            //first page
            mData.Settings.Projectname = this.projetName.Text;
            switch (this.resolution.SelectedIndex)
            {
                case 0:
                    mData.Settings.Resolution = new Vec2i(300, 200);
                    break;
                case 1:
                    mData.Settings.Resolution = new Vec2i(300, 240);
                    break;
                case 2:
                    mData.Settings.Resolution = new Vec2i(640, 480);
                    break;
                case 3:
                    mData.Settings.Resolution = new Vec2i(800, 600);
                    break;
                case 4:
                    mData.Settings.Resolution = new Vec2i(1024, 768);
                    break;
            }
            mData.Settings.NotAntialiased = this.no_antialiasing.Checked;
            mData.Settings.MuteMusicWhenSpeech = this.mute_music.Checked;
            mData.Settings.GameIcon = mIcon;
            mData.Settings.TextOnOff = this.text_on.Checked;
            mData.Settings.DrawDraggedItemIcons = this.drawDraggedItems.Checked;
            mData.Settings.GroupItems = this.groupItems.Checked;
            mData.Settings.ActionText = this.actionText.Checked;
            mData.Settings.ActionTextHeight = (int)this.actionTextHeight.Value;
            mData.Settings.SilentDelete = this.silentDelete.Checked;
            mData.Settings.InfoLine = this.infoLine.Checked;
            mData.Persistence.FitImages = this.fitImages.Checked;
            mData.Persistence.CreateBackups = this.createBackups.Checked;
            mData.Settings.ProtectGameFile = this.protectGamefiles.Checked;
            mData.Settings.LoadingImage = this.loadingImage.Text;
            //second page
            mData.Settings.Startscript = this.start_script.Text;
            mData.Settings.Mainscript = this.main_script.Text;
            if (sc_direct.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_DIRECT;
            else if (sc_blackfade.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_FADEOUT;
            else if (sc_rectangle.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_RECTANGLE;
            else if (sc_circle.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_CIRCLE;
            else if (sc_clock.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_CLOCK;
            else if (sc_shutters.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_SHUTTERS;
            else if (sc_blend.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_BLEND;
            else if (sc_blend_slow.Checked)
                mData.Settings.ScreenChange = ScreenChange.SC_BLEND_SLOW;
            mData.Settings.ShowTaskbar = this.taskbar.Checked;
            mData.Settings.TaskHeight = (int)this.taskBarHeight.Value;
            mData.Settings.TaskRoom = this.taskbar_room.Text;
            if (this.ts_always.Checked)
                mData.Settings.TaskPopup = 0;
            else if (this.ts_popup.Checked)
                mData.Settings.TaskPopup = 1;
            else if (this.ts_scrolling.Checked)
                mData.Settings.TaskPopup = 2;
            mData.Settings.TaskHideCompletely = this.hideCompletely.Checked;
            if (this.ts_appears_top.Checked)
                mData.Settings.TaskbarFromTop = true;
            else
                mData.Settings.TaskbarFromTop = false;
            mData.Settings.AnywhereRoom = this.anywhere_room.Text;
            mData.Settings.AnywhereTransparency = (int)this.anywhere_transparency.Value;
            //third page
            mData.Settings.BorderColor = Utilities.convertColorInverse(this.border_color.BackColor.ToArgb());
            mData.Settings.BackgroundColor = Utilities.convertColorInverse(this.background_color.BackColor.ToArgb());
            mData.Settings.TextColor = Utilities.convertColorInverse(this.text_color.BackColor.ToArgb());
            mData.Settings.CustomMenu = this.useCustomMenu.Checked;
            mData.Settings.CustomMenuRoom = this.customMenu.Text;
            mData.Settings.MenuFading = (int)this.menu_fading.Value;
            mData.Settings.OffspeechColor = Utilities.convertColorInverse(this.offspeech_color.BackColor.ToArgb());
            mData.Settings.InfotextColor = Utilities.convertColorInverse(this.infotext_color.BackColor.ToArgb());
            mData.Settings.TargaColor = Utilities.convertColorInverse(this.transparent_color.BackColor.ToArgb());
            //fourth page
            if (this.ts_solid.Checked)
                mData.Settings.TsStyle = 0;
            else if (this.ts_transparent.Checked)
                mData.Settings.TsStyle = 1;
            else if (this.ts_additive.Checked)
                mData.Settings.TsStyle = 2;
            else if (this.ts_none.Checked)
                mData.Settings.TsStyle = 3;
            if (this.ts_rectangles.Checked)
                mData.Settings.TsBorderStyle = 0;
            else if (this.ts_lines.Checked)
                mData.Settings.TsBorderStyle = 1;
            else if (this.ts_box.Checked)
                mData.Settings.TsBorderStyle = 2;
            else if (this.ts_border_none.Checked)
                mData.Settings.TsBorderStyle = 3;
            mData.Settings.TextSceneFading = (int)this.textscene_fading.Value;
            mData.Settings.TsAreaColor = Utilities.convertColorInverse(this.ts_area_color.BackColor.ToArgb());
            mData.Settings.TsBorderColor = Utilities.convertColorInverse(this.ts_border_color.BackColor.ToArgb());
            mData.Settings.TsTextColor = Utilities.convertColorInverse(this.ts_text_color.BackColor.ToArgb());
            mData.Settings.TsSelectionColor = Utilities.convertColorInverse(this.ts_selection_color.BackColor.ToArgb());
            if (this.ts_symbol.Checked)
                mData.Settings.TsUseSymbols = true;
            else
                mData.Settings.TsUseSymbols = false;
            mData.Settings.TsUseBgImage = this.use_background_image.Checked;
            mData.Settings.TsBackground = this.ts_background_image.Text;
            //fifth page
            mData.Settings.CoinActivated = this.coin_interface.Checked;
            mData.Settings.CoinRoom = this.coin_room.Text;
            mData.Settings.CoinAutoPopup = this.coin_autopopup.Checked;
            mData.Settings.CoinFading = (int)this.coin_fading.Value;
            //TODO crosshair
            //sixth page
            mData.Settings.NoPngToJpeg = this.noPngToJpeg.Checked;
        }

        AdvData mData;
        string mIcon;

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Symbol|*.ico";
            fod.ShowDialog();
            if (fod.FileName.Length > 0)
            {
                mIcon = fod.FileName;
                Bitmap bmp = (Bitmap)Bitmap.FromFile(mIcon);
                this.symbol.Image = bmp;
            }
        }

        private void border_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
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

        private void coin_interface_CheckedChanged(object sender, EventArgs e)
        {
            this.coin_interface_panel.Enabled = this.coin_interface.Checked;
        }

        private void numericUpDown1_ValueChanged(object sender, EventArgs e)
        {
            setFadingIndicator((NumericUpDown)sender, this.coin_fading_indicator);
        }

        private void ts_area_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void ts_border_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void ts_text_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void ts_selection_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }
    }
}