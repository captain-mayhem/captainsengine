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
        public class StateEventArgs : EventArgs
        {
            public StateEventArgs(int oldstate, int newstate)
            {
                OldState = oldstate;
                NewState = newstate;
            }
            public int OldState;
            public int NewState;
        }
        public delegate void StateEventHandler(object sender, StateEventArgs e);
        public event EventHandler PictureChanged;
        public event StateEventHandler StateChanged;

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
            this.fps.ValueChanged += new EventHandler(fps_ValueChanged);
            this.pictureBox.MouseDown += new MouseEventHandler(pictureBox_MouseDown);
            this.pictureBox.MouseMove += new MouseEventHandler(pictureBox_MouseMove);
            this.pictureBox.MouseUp += new MouseEventHandler(pictureBox_MouseUp);
            this.picbox_width.Leave += new EventHandler(picbox_width_Leave);
            this.picbox_width.KeyPress += new KeyPressEventHandler(picbox_width_KeyPress);
            this.picbox_height.Leave += new EventHandler(picbox_height_Leave);
            this.picbox_height.KeyPress += new KeyPressEventHandler(picbox_height_KeyPress);
            mTimer = new Timer();
            mTimer.Tick += new EventHandler(mTimer_Tick);
        }

        void mTimer_Tick(object sender, EventArgs e)
        {
            if (!mData.frameExists(mState, ++mFrame))
                mFrame = 0;
            pictureBox.Invalidate();
        }

        void picbox_height_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 0xd)
                picbox_height_Leave(sender, e);
        }

        void picbox_width_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 0xd)
                picbox_width_Leave(sender, e);
        }

        void picbox_height_Leave(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            int height = Convert.ToInt32(tb.Text);
            PictureBoxSize = new Size(PictureBoxSize.Width, height);
            mData.setSize(mState, new Vec2i(PictureBoxSize.Width, height));
        }

        void pictureBox_MouseUp(object sender, MouseEventArgs e)
        {
            mPictureDragging = false;
        }

        void pictureBox_MouseMove(object sender, MouseEventArgs e)
        {
            if (!mPictureDragging)
                return;
            Vec2i mouse = new Vec2i(e.X, e.Y);
            if (mouse.x < 0)
                mouse.x = 0;
            if (mouse.x > pictureBox.Size.Width)
                mouse.x = pictureBox.Size.Width;
            if (mouse.y < 0)
                mouse.y = 0;
            if (mouse.y > pictureBox.Size.Height)
                mouse.y = pictureBox.Size.Height;
            mData.setHotspot(mState, mouse / mHotspotScale);
            pictureBox.Invalidate();
        }

        void pictureBox_MouseDown(object sender, MouseEventArgs e)
        {
            Vec2i center = mData.getHotspot(mState)*mHotspotScale;
            Vec2i mouse = new Vec2i(e.X, e.Y);
            if ((mouse-center).length() < 10)
                mPictureDragging = true;
        }

        void fps_ValueChanged(object sender, EventArgs e)
        {
            string str = "Speed " + 50.0f / fps.Value + " frames/second";
            fps_label.Text = str;
            mData.setFPSDivider(mState, fps.Value);
        }

        void picturePanel_DragDrop(object sender, DragEventArgs e)
        {
            string name = (string)e.Data.GetData(DataFormats.StringFormat);
            mData.setFramePart(mState, mFrame, 0, name);
            //this.pictureBox.Invalidate();
            this.framecontrol.Invalidate();
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
            {
                imageNames.Text = "none";
                if (mDrawHotspot)
                    drawCrosshair(e.Graphics, mData.getHotspot(mState)*mHotspotScale);
                if (PictureChanged != null)
                    PictureChanged(this, new EventArgs());
                return;
            }
            if (!mTimer.Enabled)
                imageNames.Text = "";
            for (int i = 0; i < pics.Length; ++i)
            {
                if (!mTimer.Enabled)
                    imageNames.Text += pics[i] + " ";
                System.Drawing.Bitmap bmp = mData.getImage(pics[i]);
                if (mScaleImage)
                    e.Graphics.DrawImage(bmp, 0, 0, PictureBoxSize.Width, PictureBoxSize.Height);
                else
                    e.Graphics.DrawImage(bmp, 0, 0, bmp.Width, bmp.Height);
            }
            if (mDrawHotspot)
                drawCrosshair(e.Graphics, mData.getHotspot(mState)*mHotspotScale);
            if (PictureChanged != null)
                PictureChanged(this, new EventArgs());
        }

        public IStateFrameData Data
        {
            set { mData = value; updateStateValues(mState); }
            get { return mData; }
        }

        public Size PictureBoxSize
        {
            set {
                int heightDiff = value.Height - picturePanel.Size.Height;
                lower_group.Location = new Point(lower_group.Location.X, Math.Max(171, lower_group.Location.Y+heightDiff));
                int widthDiff = value.Width - picturePanel.Size.Width;
                int leftxmove = 0;
                if (height_label.Location.X - widthDiff / 2 < 0)
                    leftxmove = - height_label.Location.X + widthDiff / 2;
                //int addxsize = 0;
                //if (animation.Location.X + animation.Size.Width + widthDiff / 2 + leftxmove < value.Width + leftxmove)
                //    addxsize = -(animation.Location.X + animation.Size.Width + widthDiff / 2 + leftxmove) + (value.Width + leftxmove);
                
                picturePanel.Location = new Point(picturePanel.Location.X - widthDiff / 2 + leftxmove, picturePanel.Location.Y);
                picturePanel.ClientSize = value;
                imageNames.Location = new Point(imageNames.Location.X + widthDiff / 2 + leftxmove, imageNames.Location.Y);
                animation.Location = new Point(animation.Location.X + widthDiff / 2 + leftxmove, animation.Location.Y);
                width_label.Location = new Point(width_label.Location.X - widthDiff / 2 + leftxmove, width_label.Location.Y);
                height_label.Location = new Point(height_label.Location.X - widthDiff / 2 + leftxmove, height_label.Location.Y);
                picbox_width.Location = new Point(picbox_width.Location.X - widthDiff / 2 + leftxmove, picbox_width.Location.Y);
                picbox_height.Location = new Point(picbox_height.Location.X - widthDiff / 2 + leftxmove, picbox_height.Location.Y);
                picbox_height.Text = value.Height.ToString();
                picbox_width.Text = value.Width.ToString();
                //int clientwidth = Math.Max(value.Width+331-50, 331);
                //int clientheight = Math.Max(value.Height+296-50, 296);
                //this.ClientSize = new Size(clientwidth, clientheight);
                /*
                if (value.Width+leftxmove > this.ClientSize.Width && value.Height > this.ClientSize.Height)
                    this.ClientSize = new Size(value.Width+Math.Max(imageNames.Width, 40), value.Height);
                else if (value.Width + leftxmove > this.ClientSize.Width)
                    this.ClientSize = new Size(value.Width + leftxmove, this.ClientSize.Height);
                else if (value.Height > this.ClientSize.Height)
                    this.ClientSize = new Size(this.ClientSize.Width, value.Height);*/
            }
            get { return picturePanel.Size; }
        }

        public bool ScaleImageToBox
        {
            set { mScaleImage = value;
                width_label.Visible = !mScaleImage;
                height_label.Visible = !mScaleImage;
                picbox_width.Visible = !mScaleImage;
                picbox_height.Visible = !mScaleImage;
            }
            get { return mScaleImage; }
        }

        public Bitmap getBitmap()
        {
            Bitmap bmp = new Bitmap(pictureBox.Width, pictureBox.Height);
            Rectangle r = new Rectangle(0, 0, pictureBox.Width, pictureBox.Height);
            bool hotspot = mDrawHotspot;
            mDrawHotspot = false;
            pictureBox.DrawToBitmap(bmp, r);
            mDrawHotspot = hotspot;
            return bmp;
        }

        public void setStateLables(string[] lables)
        {
            for (int i = 0; i < Math.Min(lables.Length, mStateButtons.Length); ++i)
            {
                mStateButtons[i].Text = lables[i];
            }
        }

        public int State
        {
            get { return mState; }
        }

        public bool Hotspot
        {
            get { return mDrawHotspot; }
            set { mDrawHotspot = value; }
        }

        public float HotspotCoordScale
        {
            get { return mHotspotScale; }
            set { mHotspotScale = value; }
        }

        public bool ShowImageNames
        {
            get { return imageNames.Visible; }
            set { imageNames.Visible = value; }
        }

        public bool ShowAnimationButton
        {
            get { return animation.Visible; }
            set { animation.Visible = value; }
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
            int oldstate = mState;
            mState = newState;
            mStateButtons[mState].BackColor = Color.Turquoise;
            mFrame = 0;
            updateStateValues(oldstate);
            this.framecontrol.Invalidate();
        }

        void updateStateValues(int oldstate)
        {
            if (mData == null)
                return;
            fps.Value = mData.getFPSDivider(mState);
            Vec2i newsize = mData.getSize(mState) * mHotspotScale;
            Vec2i oldsize = new Vec2i(pictureBox.Size.Width, pictureBox.Size.Height);
            if (newsize != oldsize)
            {
                PictureBoxSize = new Size(newsize.x, newsize.y);
            }
            if (StateChanged != null)
                StateChanged(this, new StateEventArgs(oldstate, mState));
        }

        void drawCrosshair(Graphics g, Vec2i pos)
        {
            int width = 20;
            float linewidth = 2.0f;
            Pen p = new Pen(Color.Red, linewidth);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            g.DrawEllipse(p, pos.x-width/2, pos.y-width/2, width, width);
            g.DrawLine(p, pos.x, pos.y - linewidth, pos.x, pos.y - linewidth - width/2 - linewidth);
            g.DrawLine(p, pos.x, pos.y + linewidth, pos.x, pos.y + linewidth + width / 2 + linewidth);
            g.DrawLine(p, pos.x - linewidth, pos.y, pos.x - linewidth - width / 2 - linewidth, pos.y);
            g.DrawLine(p, pos.x + linewidth, pos.y, pos.x + linewidth + width / 2 + linewidth, pos.y);
        }

        private int mFrames = 25;
        private int mState = 0;
        private int mFrame = 0;
        private Button[] mStateButtons = new Button[10];
        private IStateFrameData mData;
        private Color mOrigColor;
        private bool mScaleImage = false;
        private bool mDrawHotspot = false;
        private float mHotspotScale = 1.0f;
        private bool mPictureDragging = false;
        private Timer mTimer;

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

        private void picbox_width_Leave(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            int width = Convert.ToInt32(tb.Text);
            PictureBoxSize = new Size(width, PictureBoxSize.Height);
            mData.setSize(mState, new Vec2i(width, PictureBoxSize.Height));
        }

        private void animation_Click(object sender, EventArgs e)
        {
            if (mTimer.Enabled)
            {
                mTimer.Stop();
                animation.Text = "|>";
            }
            else
            {
                mTimer.Interval = mData.getFPSDivider(mState) * 1000 / 50;
                mTimer.Start();
                animation.Text = "| |";
            }
        }
    }
}