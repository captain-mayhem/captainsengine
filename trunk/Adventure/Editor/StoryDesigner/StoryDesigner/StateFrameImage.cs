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
            for (int i = 0; i < 10; ++i)
            {
                stateDropDown.Items.Add(i + 1);
            }
            this.framecontrol.Paint += new PaintEventHandler(framecontrol_Paint);
            this.framecontrol.MouseClick += new MouseEventHandler(framecontrol_MouseClick);
            this.framecontrol.DoubleClick += new EventHandler(framecontrol_DoubleClick);
            this.pictureBox.Paint += new PaintEventHandler(pictureBox_Paint);
            this.picturePanel.DragOver += new DragEventHandler(picturePanel_DragOver);
            this.picturePanel.DragDrop += new DragEventHandler(picturePanel_DragDrop);
            mOrigColor = mStateButtons[mState].BackColor;
            mStateButtons[mState].BackColor = Color.Turquoise;
            this.fps.ValueChanged += new EventHandler(fps_ValueChanged);
            this.pictureBox.MouseDown += new MouseEventHandler(pictureBox_MouseDown);
            this.pictureBox.MouseMove += new MouseEventHandler(pictureBox_MouseMove);
            this.pictureBox.MouseUp += new MouseEventHandler(pictureBox_MouseUp);
            this.pictureBox.KeyDown += new KeyEventHandler(StateFrameImage_KeyDown);
            this.picbox_width.Leave += new EventHandler(picbox_width_Leave);
            this.picbox_width.KeyPress += new KeyPressEventHandler(picbox_width_KeyPress);
            this.picbox_height.Leave += new EventHandler(picbox_height_Leave);
            this.picbox_height.KeyPress += new KeyPressEventHandler(picbox_height_KeyPress);
            mTimer = new Timer();
            mTimer.Tick += new EventHandler(mTimer_Tick);
            this.stateDropDown.KeyDown += new KeyEventHandler(stateDropDown_KeyDown);
            this.framecontrol.AllowDrop = true;
            this.framecontrol.MouseDown += new MouseEventHandler(framecontrol_MouseDown);
            this.framecontrol.MouseUp += new MouseEventHandler(framecontrol_MouseUp);
            this.framecontrol.MouseMove += new MouseEventHandler(framecontrol_MouseMove);
            this.framecontrol.DragOver += new DragEventHandler(framecontrol_DragOver);
            this.framecontrol.DragDrop += new DragEventHandler(framecontrol_DragDrop);
        }

        void framecontrol_DoubleClick(object sender, EventArgs e)
        {
            if (!mAddScripts)
                return;
            string title = String.Format("Script for {0}/{1} ({2})", stateDropDown.Items[mState], mFrame+1, "Test");
            InputField script = new InputField(title);
            script.Input = mData.getScript(mState, mFrame);
            script.ShowDialog(this);
            if (script.DialogResult == DialogResult.OK)
            {
                mData.setScript(mState, mFrame, script.Input);
            }
            this.framecontrol.Invalidate();
        }

        void StateFrameImage_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete && mSelectedPart != -1)
            {
                removeToolStripMenuItem_Click(null, null);
                e.Handled = true;
            }
        }

        void framecontrol_MouseMove(object sender, MouseEventArgs e)
        {
            if (mMouseDown && e.Button == MouseButtons.Left)
            {
                mMouseDown = false;
                PictureBox pb = (PictureBox)sender;
                int field = e.X / (pb.Size.Width / mFrames);
                pb.DoDragDrop(field, DragDropEffects.Copy);
            }
        }

        void framecontrol_MouseUp(object sender, MouseEventArgs e)
        {
            mMouseDown = false;
        }

        void framecontrol_MouseDown(object sender, MouseEventArgs e)
        {
            mMouseDown = true;
        }

        void framecontrol_DragDrop(object sender, DragEventArgs e)
        {
            mFromFrame = (int)e.Data.GetData(typeof(int));
            PictureBox pb = (PictureBox)sender;
            Point p = pb.PointToClient(new Point(e.X, e.Y));
            mToFrame = p.X / (pb.Size.Width / mFrames);
            copyMoveMenu.Show(new Point(e.X, e.Y));
        }

        void framecontrol_DragOver(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(int)))
                e.Effect = DragDropEffects.None;
            else
            {
                PictureBox pb = (PictureBox)sender;
                Point p = pb.PointToClient(new Point(e.X, e.Y));
                int toFrame = p.X / (pb.Size.Width / mFrames);
                string[] images = mData.getFrame(mState, toFrame);
                if (images == null){
                    e.Effect = DragDropEffects.Copy;
                    return;
                }
                for (int i = images.Length-1; i >= 0; --i){
                    if (images[i] != null){
                        e.Effect = DragDropEffects.None;
                        return;
                    }
                }
                e.Effect = DragDropEffects.Copy;
            }
        }

        void stateDropDown_KeyDown(object sender, KeyEventArgs e)
        {
            if (!mEditStateText)
                return;
            if (e.KeyCode == Keys.Enter)
            {
                string text = stateDropDown.Text;
                mEditStateText = false;
                stateDropDown.DropDownStyle = ComboBoxStyle.DropDownList;
                mData.setStateName(mState, text);
                stateDropDown.Items[mState] = text;
                stateDropDown.SelectedItem = text;
            }
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
            {
                picbox_height_Leave(sender, e);
                e.Handled = true;
            }
        }

        void picbox_width_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 0xd)
            {
                picbox_width_Leave(sender, e);
                e.Handled = true;
            }
        }

        void picbox_height_Leave(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            int height = Convert.ToInt32(tb.Text);
            PictureBoxSize = new Size(PictureBoxSize.Width, height);
            mData.setSize(mState, new Vec2i(PictureBoxSize.Width, height));
            if (mData.getHotspot(mState).y > height)
                mData.setHotspot(mState, new Vec2i(mData.getHotspot(mState).x, height));
            pictureBox.Invalidate();
        }

        void pictureBox_MouseUp(object sender, MouseEventArgs e)
        {
            mPictureDragging = 0;
        }

        void pictureBox_MouseMove(object sender, MouseEventArgs e)
        {
            if (mPictureDragging == 0)
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
            if (mPictureDragging == 100)
                mData.setHotspot(mState, (mouse-mDraggingOffset) / mHotspotScale);
            else
            {
                mData.setFramePartOffset(mState, mFrame, mPictureDragging - 1, mouse-mDraggingOffset);
            }
            pictureBox.Invalidate();
        }

        void pictureBox_MouseDown(object sender, MouseEventArgs e)
        {
            pictureBox.Focus();
            Vec2i center = mData.getHotspot(mState)*mHotspotScale;
            Vec2i mouse = new Vec2i(e.X, e.Y);
            if ((mouse - center).length() < 10)
            {
                mDraggingOffset = mouse - center;
                mPictureDragging = 100;
                return;
            }
            string[] pics = mData.getFrame(mState, mFrame);
            if (pics == null)
                return;
            for (int i = pics.Length-1; i >= 0; --i)
            {
                if (pics[i] == null)
                    continue;
                System.Drawing.Bitmap bmp = mData.getImage(pics[i]);
                Vec2i offset = mData.getFramePartOffset(mState, mFrame, i);
                if (mouse.x >= offset.x && mouse.x <= offset.x + bmp.Width)
                {
                    if (mouse.y >= offset.y && mouse.y <= offset.y + bmp.Height)
                    {
                        mDraggingOffset = mouse - offset;
                        mPictureDragging = i + 1;
                        mSelectedPart = i;
                        pictureBox.Invalidate();
                        if (e.Button == MouseButtons.Right)
                        {
                            Point p = pictureBox.PointToScreen(e.Location);
                            removeMenu.Show(p);
                        }
                        return;
                    }
                }
            }
            mSelectedPart = -1;
            pictureBox.Invalidate();
        }

        void fps_ValueChanged(object sender, EventArgs e)
        {
            string str = "Speed " + 50.0f / fps.Value + " frames/second";
            fps_label.Text = str;
            mData.setFPSDivider(mState, fps.Value);
        }

        void picturePanel_DragDrop(object sender, DragEventArgs e)
        {
            Resource res = (Resource)e.Data.GetData(typeof(Resource));
            string name = res.Name;
            Point p = pictureBox.PointToClient(new Point(e.X, e.Y));
            string[] frames = mData.getFrame(mState, mFrame);
            int part = 0;
            bool keep_offset = false;
            if (frames != null && frames.Length < mFrameParts)
            {
                part = frames.Length;
            }
            else if (frames != null)
            {
                for (int i = 0; i < frames.Length; ++i)
                {
                    if (frames[i] == null)
                    {
                        part = i;
                        continue;
                    }
                    System.Drawing.Bitmap bmp = mData.getImage(frames[i]);
                    Vec2i offset = mData.getFramePartOffset(mState, mFrame, i);
                    if (p.X >= offset.x && p.X <= offset.x + bmp.Width)
                    {
                        if (p.Y >= offset.y && p.Y <= offset.y + bmp.Height)
                        {
                            part = i;
                            keep_offset = true;
                        }
                    }
                }
            }
            if (mAdaptFirstDrop && isObjectEmpty())
            {
                System.Drawing.Bitmap dropimg = mData.getImage(name);
                picbox_width.Text = dropimg.Size.Width.ToString();
                picbox_width_Leave(picbox_width, null);
                picbox_height.Text = dropimg.Size.Height.ToString();
                picbox_height_Leave(picbox_height, null);
                p.X = 0;
                p.Y = 0;
            }
            else if (mAdaptFirstDrop && isFrameEmpty(mState, mFrame))
            {
                p.X = 0;
                p.Y = 0;
            }
            mData.setFramePart(mState, mFrame, part, name);
            if (!keep_offset)
                mData.setFramePartOffset(mState, mFrame, part, new Vec2i(p.X, p.Y));
            this.framecontrol.Invalidate();
        }

        private bool isObjectEmpty()
        {
            for (int i = 0; i < 10; ++i)
            {
                for (int j = 0; j < Frames; ++j)
                {
                    if (!isFrameEmpty(i, j))
                        return false;
                }
            }
            return true;
        }

        private bool isFrameEmpty(int state, int frameNum)
        {
            string[] frames = mData.getFrame(state, frameNum);
            if (frames == null)
                return true;
            foreach (string frame in frames)
            {
                if (frame != null)
                    return false;
            }
            return true;
        }

        void picturePanel_DragOver(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(typeof(Resource)))
                e.Effect = DragDropEffects.None;
            else
            {
                Resource res = (Resource)e.Data.GetData(typeof(Resource));
                if (res.ID != ResourceID.IMAGE)
                    e.Effect = DragDropEffects.None;
                else
                    e.Effect = DragDropEffects.Copy;
            }
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
                    Utilities.drawCrosshair(e.Graphics, mData.getHotspot(mState)*mHotspotScale);
                if (PictureChanged != null)
                    PictureChanged(this, new EventArgs());
                return;
            }
            if (!mTimer.Enabled && mPictureDragging == 0)
                imageNames.Text = "";
            for (int i = 0; i < pics.Length; ++i)
            {
                if (pics[i] == null)
                    continue;
                if (!mTimer.Enabled && mPictureDragging == 0)
                    imageNames.Text += pics[i] + " ";
                System.Drawing.Bitmap bmp = mData.getImage(pics[i]);
                Vec2i offset = mData.getFramePartOffset(mState, mFrame, i);
                Pen pen;
                if (i == mSelectedPart)
                    pen = new Pen(Color.Yellow);
                else
                    pen = new Pen(Color.Red);
                pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Custom;
                pen.DashPattern = new float[] { 3, 3 };
                pen.DashOffset = 3.0f;
                if (mScaleImage)
                {
                    e.Graphics.DrawImage(bmp, 0, 0, PictureBoxSize.Width, PictureBoxSize.Height);
                    e.Graphics.DrawRectangle(pen, -1, -1, PictureBoxSize.Width+1, PictureBoxSize.Height+1);
                }
                else
                {
                    e.Graphics.DrawImage(bmp, offset.x, offset.y, bmp.Width, bmp.Height);
                    e.Graphics.DrawRectangle(pen, offset.x-1, offset.y-1, bmp.Width+1, bmp.Height+1);
                }
            }
            if (mDrawHotspot)
                Utilities.drawCrosshair(e.Graphics, mData.getHotspot(mState)*mHotspotScale);
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
                int val = height_label.Location.X - widthDiff / 2;
                if (mFrameParts>1 && (val < 0 || val > 10))
                    leftxmove = - val;
                
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

        public void setStateLables(string[] labels)
        {
            for (int i = 0; i < Math.Min(labels.Length, mStateButtons.Length); ++i)
            {
                mStateButtons[i].Text = labels[i];
            }
            stateDropDown.Items.Clear();
            stateDropDown.Items.AddRange(labels);
            stateDropDown.SelectedIndex = mState;
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

        public bool ShowStateDropDown
        {
            get { return mShowStateDropdown; }
            set
            {
                mShowStateDropdown = value;
                for (int i = 0; i < mStateButtons.Length; ++i)
                {
                    mStateButtons[i].Visible = !value;
                }
                stateDropDown.Visible = value;
                stateDropDown.SelectedIndex = mState;
            }
        }

        public int Frames
        {
            get { return mFrames; }
            set { mFrames = value; this.framecontrol.Invalidate(); }
        }

        void framecontrol_MouseClick(object sender, MouseEventArgs e)
        {
            PictureBox pb = (PictureBox)sender;
            int field = e.X / (pb.Size.Width / mFrames);
            mFrame = field;
            mSelectedPart = -1;
            this.framecontrol.Invalidate();
        }

        void framecontrol_Paint(object sender, PaintEventArgs e)
        {
            PictureBox pb = (PictureBox)sender;
            Pen p = new Pen(Color.Black);
            Brush b = new SolidBrush(Color.Turquoise);
            for (int i = 0; i < mFrames; ++i)
            {
                Rectangle r = new Rectangle(i*(pb.Size.Width/mFrames+0), 0, pb.Size.Width / (mFrames + 0)-2, pb.Size.Height - 1);
                if (i == mFrame)
                    e.Graphics.FillRectangle(b, r);
                if (mData != null)
                {
                    if (mData.frameExists(mState, i) && !isFrameEmpty(mState, i))
                        e.Graphics.DrawEllipse(p, r.X+r.Width/4+1, r.Y+r.Height/2-2, r.Width/2, r.Height/2);
                    if (mData.getScript(mState, i) != null && mData.getScript(mState, i).Length > 0)
                        e.Graphics.DrawEllipse(p, r.X + r.Width / 4 + 1, r.Y + 2, r.Width / 2, r.Width / 2);
                }
                e.Graphics.DrawRectangle(p, r);
            }
            this.pictureBox.Invalidate();
        }

        void changeState(int newState)
        {
            if (mState < 10)
                mStateButtons[mState].BackColor = mOrigColor;
            int oldstate = mState;
            mState = newState;
            if (mState < 10)
                mStateButtons[mState].BackColor = Color.Turquoise;
            mFrame = 0;
            mSelectedPart = -1;
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

        public bool AdaptFirstDrop
        {
            get { return mAdaptFirstDrop; }
            set { mAdaptFirstDrop = value; }
        }

        private int mFrames = 25;
        private int mState = 0;
        private int mFrame = 0;
        private int mFrameParts = 1;
        private int mSelectedPart = -1;
        private Button[] mStateButtons = new Button[10];
        private IStateFrameData mData;
        private Color mOrigColor;
        private bool mScaleImage = false;
        private bool mDrawHotspot = false;
        private float mHotspotScale = 1.0f;
        private int mPictureDragging = 0;
        private Vec2i mDraggingOffset = new Vec2i();
        private Timer mTimer;
        private bool mShowStateDropdown = false;
        private bool mEditStateText = false;
        private bool mMouseDown = false;
        private int mFromFrame = 0;
        private int mToFrame = 0;
        private bool mAdaptFirstDrop = false;
        private bool mAddScripts = true;

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
            if (mData.getHotspot(mState).x > width)
                mData.setHotspot(mState, new Vec2i(width, mData.getHotspot(mState).y));
            pictureBox.Invalidate();
        }

        private void animation_Click(object sender, EventArgs e)
        {
            if (mTimer.Enabled)
            {
                mTimer.Stop();
                animation.Text = "|>";
                mFrame = 0;
                pictureBox.Invalidate();
            }
            else
            {
                mTimer.Interval = mData.getFPSDivider(mState) * 1000 / 50;
                mTimer.Start();
                animation.Text = "| |";
            }
        }

        private void stateDropDown_SelectedIndexChanged(object sender, EventArgs e)
        {
            changeState(stateDropDown.SelectedIndex);
        }

        public int FrameParts
        {
            get { return mFrameParts; }
            set { mFrameParts = value; }
        }

        public Color BackgroundColor
        {
            get { return pictureBox.BackColor; }
            set { pictureBox.BackColor = value; }
        }

        public void renameState()
        {
            stateDropDown.DropDownStyle = ComboBoxStyle.Simple;
            mEditStateText = true;
            stateDropDown.Focus();
        }

        public bool AddScripts
        {
            get {return mAddScripts;}
            set {mAddScripts = value;}
        }

        private void copyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string[] from = mData.getFrame(mState, mFromFrame);
            for (int i = 0; i < from.Length; ++i)
            {
                mData.setFramePart(mState, mToFrame, i, from[i]);
                Vec2i offset = mData.getFramePartOffset(mState, mFromFrame, i);
                mData.setFramePartOffset(mState, mToFrame, i, offset);
            }
            mFrame = mToFrame;
            framecontrol.Invalidate();
        }

        private void moveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string[] from = mData.getFrame(mState, mFromFrame);
            for (int i = 0; i < from.Length; ++i)
            {
                mData.setFramePart(mState, mToFrame, i, from[i]);
                Vec2i offset = mData.getFramePartOffset(mState, mFromFrame, i);
                mData.setFramePartOffset(mState, mToFrame, i, offset);
                mData.setFramePart(mState, mFromFrame, i, null);
            }
            mFrame = mToFrame;
            framecontrol.Invalidate();
        }

        private void removeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mData.setFramePart(mState, mFrame, mSelectedPart, null);
            framecontrol.Invalidate();
        }
    }
}
