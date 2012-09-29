using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Collections;

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
            this.stateFrameImage1.BackgroundColor = data.Persistence.CharBackColor;

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
            mData = data;
        }

        void stateFrameImage1_ClientSizeChanged(object sender, EventArgs e)
        {
            int height = stateFrameImage1.Location.Y + stateFrameImage1.Size.Height - 30;
            this.panel_down.Location = new Point(panel_down.Location.X, height);
            this.Size = new Size(this.Size.Width, height + this.panel_down.Size.Height+40);
        }

        AdvCharacter mCharacter;
        AdvData mData;
        Dictionary<int,string[]> mCopiedFrames;
        Dictionary<int, ArrayList> mCopiedOffsets;
        Vec2i mCopiedSize;
        Vec2i mCopiedHotspot;

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

        private void backgroundColorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ColorDialog cd = new ColorDialog();
            if (cd.ShowDialog() == DialogResult.OK)
            {
                mData.Persistence.CharBackColor = cd.Color;
                stateFrameImage1.BackgroundColor = cd.Color;
            }
        }

        private void copyActionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IStateFrameData data = stateFrameImage1.Data;
            int currState = stateFrameImage1.State;
            mCopiedFrames = new Dictionary<int, string[]>();
            mCopiedOffsets = new Dictionary<int, ArrayList>();
            for (int i = 0; i < stateFrameImage1.Frames; ++i)
            {
                if (data.frameExists(currState, i))
                {
                    mCopiedFrames[i] = (string[])data.getFrame(currState, i).Clone();
                    mCopiedOffsets[i] = new ArrayList();
                    for (int j = 0; j < stateFrameImage1.FrameParts; ++j)
                    {
                        Vec2i offset = data.getFramePartOffset(currState, i, j);
                        mCopiedOffsets[i].Add(offset);
                    }
                }
            }
            mCopiedSize = data.getSize(currState);
            mCopiedHotspot = data.getHotspot(currState);
            pasteActionToolStripMenuItem.Enabled = true;
        }

        private void pasteActionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IStateFrameData data = stateFrameImage1.Data;
            int currState = stateFrameImage1.State;
            data.setSize(currState, mCopiedSize);
            data.setHotspot(currState, mCopiedHotspot);
            foreach (KeyValuePair<int, string[]> frames in mCopiedFrames)
            {
                for (int i = 0; i < frames.Value.Length; ++i)
                {
                    data.setFramePart(currState, frames.Key, i, frames.Value[i]);
                    data.setFramePartOffset(currState, frames.Key, i, (Vec2i)mCopiedOffsets[frames.Key][i]);
                }
            }
            stateFrameImage1.Invalidate(true);
        }

        private void deleteActionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IStateFrameData data = stateFrameImage1.Data;
            int currState = stateFrameImage1.State;
            for (int i = 0; i < stateFrameImage1.Frames; ++i)
            {
                for (int j = 0; j < stateFrameImage1.FrameParts; ++j)
                {
                    data.setFramePart(currState, i, j, null);
                }
            }
            stateFrameImage1.Invalidate(true);
        }
    }
}