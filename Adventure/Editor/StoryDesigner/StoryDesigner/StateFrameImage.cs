using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class StateFrameImage : UserControl
    {
        public StateFrameImage()
        {
            InitializeComponent();
            mStateButtons[0] = button1;
            mStateButtons[1] = button2;
            mStateButtons[2] = button3;
            mStateButtons[3] = button4;
            mStateButtons[4] = button5;
            mStateButtons[5] = button6;
            mStateButtons[6] = button7;
            mStateButtons[7] = button8;
            mStateButtons[8] = button9;
            mStateButtons[9] = button10;
            this.framecontrol.Paint += new PaintEventHandler(framecontrol_Paint);
            this.framecontrol.MouseClick += new MouseEventHandler(framecontrol_MouseClick);
            this.pictureBox.Paint += new PaintEventHandler(pictureBox_Paint);
            this.picturePanel.DragOver += new DragEventHandler(picturePanel_DragOver);
            this.picturePanel.DragDrop += new DragEventHandler(picturePanel_DragDrop);
            mOrigColor = mStateButtons[mState].BackColor;
            mStateButtons[mState].BackColor = Color.Turquoise;
        }

        void picturePanel_DragDrop(object sender, DragEventArgs e)
        {
            string name = (string)e.Data.GetData(DataFormats.StringFormat);
            mData.setFramePart(mState, mFrame, 0, name);
            this.pictureBox.Invalidate();
        }

        void picturePanel_DragOver(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(DataFormats.StringFormat))
                e.Effect = DragDropEffects.None;
            e.Effect = DragDropEffects.Copy;
        }

        void pictureBox_Paint(object sender, PaintEventArgs e)
        {
            if (mData == null)
                return;
            string[] pics = mData.getFrame(mState, mFrame);
            if (pics == null)
                return;
            for (int i = 0; i < pics.Length; ++i)
            {
                System.Drawing.Bitmap bmp = mData.getImage(pics[i]);
                if (mScaleImage)
                    e.Graphics.DrawImage(bmp, 0, 0, PictureBoxSize.Width, PictureBoxSize.Height);
                else
                    e.Graphics.DrawImage(bmp, 0, 0);
            }
        }

        public IStateFrameData Data
        {
            set { mData = value; }
            get { return mData; }
        }

        public Size PictureBoxSize
        {
            set {
                int heightDiff = value.Height - picturePanel.Size.Height;
                lower_group.Location = new Point(lower_group.Location.X, lower_group.Location.Y+heightDiff);
                int widthDiff = value.Width - picturePanel.Size.Width;
                picturePanel.Location = new Point(picturePanel.Location.X - widthDiff / 2, picturePanel.Location.Y);
                picturePanel.Size = value;
            }
            get { return picturePanel.Size; }
        }

        public bool ScaleImageToBox
        {
            set { mScaleImage = value; }
            get { return mScaleImage; }
        }

        void framecontrol_MouseClick(object sender, MouseEventArgs e)
        {
            PictureBox pb = (PictureBox)sender;
            int field = e.X / (pb.Size.Width / mFrames+1);
            mFrame = field;
            this.framecontrol.Invalidate();
        }

        void framecontrol_Paint(object sender, PaintEventArgs e)
        {
            PictureBox pb = (PictureBox)sender;
            Pen p = new Pen(Color.Black);
            Brush b = new SolidBrush(Color.Turquoise);
            for (int i = 0; i < mFrames; ++i)
            {
                Rectangle r = new Rectangle(i*(pb.Size.Width/mFrames+1), 0, pb.Size.Width / mFrames - 1, pb.Size.Height - 1);
                if (i == mFrame)
                    e.Graphics.FillRectangle(b, r);
                if (mData != null && mData.frameExists(mState, i))
                {
                    e.Graphics.DrawEllipse(p, r.X+r.Width/4+1, r.Y+r.Height/4, r.Width/2, r.Height/2);
                }
                e.Graphics.DrawRectangle(p, r);
            }
            this.pictureBox.Invalidate();
        }

        void changeState(int newState)
        {
            mStateButtons[mState].BackColor = mOrigColor;
            mState = newState;
            mStateButtons[mState].BackColor = Color.Turquoise;
            mFrame = 0;
            this.framecontrol.Invalidate();
        }

        private int mFrames = 25;
        private int mState = 0;
        private int mFrame = 0;
        private Button[] mStateButtons = new Button[10];
        private IStateFrameData mData;
        private Color mOrigColor;
        private bool mScaleImage = false;

        private void button1_Click(object sender, EventArgs e)
        {
            changeState(0);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            changeState(1);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            changeState(2);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            changeState(3);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            changeState(4);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            changeState(5);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            changeState(6);
        }

        private void button8_Click(object sender, EventArgs e)
        {
            changeState(7);
        }

        private void button9_Click(object sender, EventArgs e)
        {
            changeState(8);
        }

        private void button10_Click(object sender, EventArgs e)
        {
            changeState(9);
        }
    }
}
