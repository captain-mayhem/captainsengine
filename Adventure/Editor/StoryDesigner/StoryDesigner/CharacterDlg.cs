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
        public CharacterDlg(AdvCharacter chr)
        {
            InitializeComponent();
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
            this.stateFrameImage1.setStateLables(labels);
            this.stateFrameImage1.Data = chr;

            this.text_color.BackColor = Color.FromArgb(Utilities.convertColor(chr.TextColor));

            mCharacter = chr;
        }

        AdvCharacter mCharacter;

        private void text_color_Click(object sender, EventArgs e)
        {
            Utilities.chooseColor(sender);
        }
    }
}