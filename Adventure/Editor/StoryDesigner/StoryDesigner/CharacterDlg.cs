using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class CharacterDlg : Form
    {
        public CharacterDlg(AdvCharacter chr, AdvData data)
        {
            InitializeComponent();
            this.stateFrameImage1.ClientSizeChanged += new EventHandler(stateFrameImage1_ClientSizeChanged);
            this.Text = "Character (" + chr.Name + ")";
            string[] labels = new string[36];
            labels[0] = "Stand front";
            labels[1] = "Stand back";
            labels[2] = "Stand right";
            labels[3] = "Walk front";
            labels[4] = "Walk back";
            labels[5] = "Walk right";
            labels[6] = "Talk, Stand front";
            labels[7] = "Talk, Stand back";
            labels[8] = "Talk, Stand right";
            labels[9] = "Talk, Walk front";
            labels[10] = "Talk, Walk back";
            labels[11] = "Talk, Walk right";
            labels[12] = "Bored 1";
            labels[13] = "Bored 2";
            labels[14] = "Pickup front";
            labels[15] = "Pickup right";
            for (int i = 16; i < 36; ++i)
            {
                labels[i] = chr.getStateName(i);
            }
            this.font.Items.Add("Default");
            for (int i = 0; i < data.Settings.Fonts.Count; ++i)
            {
                FontInfo fi = (FontInfo)data.Settings.Fonts[i];
                this.font.Items.Add(fi.name+ " ("+fi.size+")");
            }
            for (int i = 1; i < 10; ++i)
            {
                this.walkspeed.Items.Add(i);
            }
            this.stateFrameImage1.setStateLables(labels);
            this.stateFrameImage1.Data = chr;

            this.text_color.BackColor = Color.FromArgb(Utilities.convertColor(chr.TextColor));
            this.font.SelectedIndex = chr.Font;
            this.walksound.Text = chr.Walksound;
            this.walkspeed.SelectedIndex = chr.WalkSpeed - 1;
            this.nozoom.Checked = chr.NoZoom;
            this.leftanim.Checked = chr.RealLeftAnimations;
            this.memory_resistant.Checked = chr.MemoryReistent;
            this.ghostmode.Checked = chr.Ghost;
            this.startzoom.Value = chr.Zoom;

            mCharacter = chr;
        }

        void stateFrameImage1_ClientSizeChanged(object sender, EventArgs e)
        {
            int height = stateFrameImage1.Location.Y + stateFrameImage1.Size.Height - 30;
            this.panel_down.Location = new Point(panel_down.Location.X, height);
            this.Size = new Size(this.Size.Width, height + this.panel_down.Size.Height+40);
        }

        AdvCharacter mCharacter;

        private void text_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }

        private void customize_Click(object sender, EventArgs e)
        {
            customizeMenu.Show(this, customize.Location.X, customize.Location.Y + customize.Size.Height);
        }

        private void customizeMenu_Opening(object sender, CancelEventArgs e)
        {

        }
    }
}