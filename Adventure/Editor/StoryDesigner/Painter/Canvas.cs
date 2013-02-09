using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Painter
{
    public partial class Canvas : Form
    {
        public Canvas(int width, int height)
        {
            InitializeComponent();
            initialize();
            this.ClientSize = new Size(width, height);
            mBitmap = new Bitmap(width, height);
            mGraphics = Graphics.FromImage(mBitmap);
            mPen = new Pen(Color.Yellow);
        }

        private void initialize()
        {
            this.BackColor = Color.Black;
            this.MouseDown += new MouseEventHandler(Canvas_MouseDown);
            this.MouseUp += new MouseEventHandler(Canvas_MouseUp);
            this.Paint += new PaintEventHandler(Canvas_Paint);
            this.MouseMove += new MouseEventHandler(Canvas_MouseMove);
        }

        void Canvas_MouseMove(object sender, MouseEventArgs e)
        {
            if (!mIsDrawing)
                return;
            Point newPoint = new Point(e.X, e.Y);
            drawLine(mLastPoint, newPoint);
            mLastPoint = newPoint;
        }

        void Canvas_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.DrawImage(mBitmap, 0, 0);
        }

        void Canvas_MouseUp(object sender, MouseEventArgs e)
        {
            mIsDrawing = false;
        }

        void Canvas_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button != MouseButtons.Left)
                return;
            mLastPoint = new Point(e.X, e.Y);
            mIsDrawing = true;
            setPixel(mLastPoint);
        }

        void setPixel(Point p)
        {
            mBitmap.SetPixel(p.X, p.Y, mPen.Color);
            this.Invalidate();
        }

        void drawLine(Point p1, Point p2)
        {
            mGraphics.DrawLine(mPen, p1, p2);
            this.Invalidate();
        }

        private Bitmap mBitmap;
        private Graphics mGraphics;
        private bool mIsDrawing;
        private Point mLastPoint;
        private Pen mPen;
    }
}
