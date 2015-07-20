using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SupremeEdit
{
    public partial class TextView : Form
    {
        public TextView()
        {
            InitializeComponent();
            mFont = new Font("Arial", 12);
            mMainColor = Brushes.Black;
            mMargin = new Point(20, 20);
            mLogicalCursor = new Point(0, 0);
            mShowCursor = true;
            mCursorTimer = new Timer();
            mCursorTimer.Interval = 500;
            mCursorTimer.Tick += new EventHandler(mCursorTimer_Tick);
            mCursorTimer.Start();
            Paint += new PaintEventHandler(TextView_Paint);
            Resize += new EventHandler(TextView_Resize);
            this.KeyPress += new KeyPressEventHandler(TextView_KeyPress);
            this.KeyDown += new KeyEventHandler(TextView_KeyDown);
            this.MouseDown += new MouseEventHandler(TextView_MouseDown);
            setText("Hello World\nYes, man\nThe next line");
        }

        void TextView_MouseDown(object sender, MouseEventArgs e)
        {
            mLogicalCursor.Y = (int)((e.Y - mMargin.Y) / mFont.GetHeight());
            mLogicalCursor.Y = Math.Min(mLogicalCursor.Y, mText.Length - 1);
            mLogicalCursor.Y = Math.Max(mLogicalCursor.Y, 0);
            mShowCursor = false;
            //binary search for right x position
            Graphics g = CreateGraphics();
            int target = e.X -  mMargin.X;
            int istart = 0;
            int iend = mText[mLogicalCursor.Y].Length;
            if (target < 0)
                mLogicalCursor.X = istart;
            else
            {
                while (iend - istart > 1)
                {
                    int pos = istart + (iend - istart) / 2;
                    int width = calculateTextWidth(g, pos);
                    if (width > target)
                        iend = pos;
                    else
                        istart = pos;
                }
                int prevwidth = calculateTextWidth(g, istart);
                int nextwidth = calculateTextWidth(g, istart+1);
                mLogicalCursor.X = istart;
                if (target - prevwidth > nextwidth - target)
                    ++mLogicalCursor.X;
            }
            g.Dispose();
        }

        public void TextView_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Right:
                    if (mLogicalCursor.X < mText[mLogicalCursor.Y].Length)
                        ++mLogicalCursor.X;
                    else if (mLogicalCursor.Y < mText.Length - 1)
                    {
                        mLogicalCursor.X = 0;
                        ++mLogicalCursor.Y;
                    }
                    break;
                case Keys.Left:
                    if (mLogicalCursor.X > 0)
                        --mLogicalCursor.X;
                    else if (mLogicalCursor.Y > 0)
                    {
                        --mLogicalCursor.Y;
                        mLogicalCursor.X = mText[mLogicalCursor.Y].Length;
                    }
                    break;
                case Keys.Down:
                    if (mLogicalCursor.Y < mText.Length - 1)
                    {
                        ++mLogicalCursor.Y;
                        if (mLogicalCursor.X > mText[mLogicalCursor.Y].Length)
                            mLogicalCursor.X = mText[mLogicalCursor.Y].Length;
                    }
                    break;
                case Keys.Up:
                    if (mLogicalCursor.Y > 0)
                    {
                        --mLogicalCursor.Y;
                        if (mLogicalCursor.X > mText[mLogicalCursor.Y].Length)
                            mLogicalCursor.X = mText[mLogicalCursor.Y].Length;
                    }
                    break;
            }
            Invalidate();
        }

        void TextView_KeyPress(object sender, KeyPressEventArgs e)
        {
        }

        void mCursorTimer_Tick(object sender, EventArgs e)
        {
            mShowCursor = !mShowCursor;
            Invalidate();
        }

        void TextView_Resize(object sender, EventArgs e)
        {
            //WindowState = FormWindowState.Maximized;
        }

        void TextView_Paint(object sender, PaintEventArgs e)
        {
            Point p = mMargin;
            for (int i = 0; i < mText.Length; ++i)
            {
                e.Graphics.DrawString(mText[i], mFont, mMainColor, p);
                p.Y += (int)mFont.GetHeight();
            }
            if (mShowCursor)
            {   
                Point cursorStart = mMargin;
                cursorStart.X += calculateTextWidth(e.Graphics, mLogicalCursor.X);
                cursorStart.Y += (int)(mFont.GetHeight() * mLogicalCursor.Y);
                Point cursorEnd = new Point(cursorStart.X, (int)(cursorStart.Y + mFont.GetHeight()));
                Pen cursor = new Pen(Color.Black, 1.5f);
                e.Graphics.DrawLine(cursor, cursorStart, cursorEnd);
            }
        }

        public void setText(string text)
        {
            mText = text.Split('\n');
        }

        int calculateTextWidth(Graphics g, int to)
        {
            float x = 0.0F;
            float y = 0.0F;
            float width = 1350.0F;
            float height = 200.0F;
            RectangleF layoutRect = new RectangleF(x, y, width, height);
            int calcTo = to == 0 ? 1 : to;
            CharacterRange[] ranges = { new CharacterRange(0, calcTo) };
            StringFormat stringFormat = new StringFormat();
            stringFormat.SetMeasurableCharacterRanges(ranges);
            Region[] measure = g.MeasureCharacterRanges(mText[mLogicalCursor.Y], mFont, layoutRect, stringFormat);
            RectangleF rect = measure[0].GetBounds(g);
            float ret = rect.X;
            if (to > 0)
                ret += rect.Width;
            return (int)ret;
        }

        string[] mText;       
        Font mFont;
        Brush mMainColor;
        Point mLogicalCursor;
        Timer mCursorTimer;
        bool mShowCursor;
        Point mMargin;
    }
}
