using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ImageViewer : ChildForm
    {
        public ImageViewer(System.Drawing.Bitmap bmp)
        {
            InitializeComponent();
            mImage = bmp;
            this.ClientSize = new Size(Math.Max(bmp.Width,50), Math.Max(bmp.Height,50));
            Paint += new PaintEventHandler(ImageViewer_Paint);
            this.Resize += new EventHandler(ImageViewer_Resize);
        }

        void ImageViewer_Resize(object sender, EventArgs e)
        {
            Invalidate();
        }

        void ImageViewer_Paint(object sender, PaintEventArgs e)
        {
            float scalex = e.ClipRectangle.Width / (float)mImage.Width;
            float scaley = e.ClipRectangle.Height / (float)mImage.Height;
            float scale = System.Math.Min(scalex, scaley);
            e.Graphics.ScaleTransform(scale, scale);
            e.Graphics.DrawImage(mImage, 0, 0, mImage.Width, mImage.Height);
        }

        public Bitmap Image
        {
            get { return mImage; }
            set { mImage = value; Invalidate(); }
        }

        System.Drawing.Bitmap mImage;
    }
}