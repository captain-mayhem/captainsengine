using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace StoryDesigner
{
    static class Utilities
    {
        public static int convertColor(System.UInt32 color)
        {
            System.UInt32 ret = 0xFF000000;
            ret |= ((System.UInt32)color & 0xFF) << 16;
            ret |= (System.UInt32)color & 0xFF00;
            ret |= ((System.UInt32)color & 0xFF0000) >> 16;
            return (int)ret;
        }

        public static System.UInt32 convertColorInverse(int color)
        {
            System.UInt32 ret = 0;
            ret |= ((System.UInt32)color & 0xFF) << 16;
            ret |= (System.UInt32)color & 0xFF00;
            ret |= ((System.UInt32)color & 0xFF0000) >> 16;
            return ret;
        }

        public static Color chooseColor(object sender)
        {
            ColorDialog cd = new ColorDialog();
            if (cd.ShowDialog() == DialogResult.OK)
            {
                Button btn = (Button)sender;
                btn.BackColor = cd.Color;
            }
            return ((Button)sender).BackColor;
        }

        public static void drawText(Graphics g, int x, int y, string text, Font font)
        {
            FontInfo info = new FontInfo();
            info.fill = 0;
            info.outline = 3;
            info.shadow = 0;
            StringFormat fmt = new StringFormat();
            drawText(g, x, y, text, font, info, fmt, false);
        }

        public static void drawText(Graphics g, int x, int y, string text, Font font, FontInfo info, StringFormat fmt, bool allwhite)
        {
            //g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            if (info.outline == 1 || info.outline == 3)
            {
                g.TranslateTransform(-1.0f, -1.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(1.0f, 1.0f);
                g.TranslateTransform(0.0f, -1.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(0.0f, 1.0f);
                g.TranslateTransform(-1.0f, 0.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(1.0f, 0.0f);
            }
            if (info.outline == 2 || info.outline == 3)
            {
                g.TranslateTransform(1.0f, 1.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(-1.0f, -1.0f);
                g.TranslateTransform(0.0f, 1.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(0.0f, -1.0f);
                g.TranslateTransform(1.0f, 0.0f);
                g.DrawString(text, font, allwhite ? Brushes.White : Brushes.Black, x, y, fmt);
                g.TranslateTransform(-1.0f, 0.0f);
            }
            if (info.shadow != 0)
            {
                g.TranslateTransform(3.0f, 3.0f);
                SolidBrush b = new SolidBrush(Color.FromArgb((int)(info.shadow * 0.25 * 255), allwhite ? Color.White : Color.Black));
                g.DrawString(text, font, b, x, y, fmt);
                g.TranslateTransform(-3.0f, -3.0f);
            }

            Brush brush = null;
            if (info.fill == 0 || allwhite)
                brush = Brushes.White;
            else if (info.fill == 1)
                brush = new LinearGradientBrush(new Point(x, y), new Point(x, y + font.Height), Color.White, Color.Gray);
            else if (info.fill == 2)
                brush = new LinearGradientBrush(new Point(x, y), new Point(x, y + font.Height), Color.Gray, Color.White);
            else if (info.fill == 3)
                brush = new HatchBrush(HatchStyle.NarrowHorizontal, Color.LightGray, Color.White);
            g.DrawString(text, font, brush, x, y, fmt);
        }

        public static void drawCircle(Graphics g, Pen p, int radius, Vec2i center)
        {
            Vec2i start = center-new Vec2i(radius, radius);
            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.DrawEllipse(p, start.x, start.y, 2 * radius, 2 * radius);
        }

        public static void drawCrosshair(Graphics g, Vec2i pos)
        {
            drawCrosshair(g, pos, Color.Red, 2.0f);
        }

        public static void drawCrosshair(Graphics g, Vec2i pos, Color color, float linewidth)
        {
            int width = 20;
            Pen p = new Pen(color, linewidth);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            g.DrawEllipse(p, pos.x - width / 2, pos.y - width / 2, width, width);
            g.DrawLine(p, pos.x, pos.y - linewidth, pos.x, pos.y - linewidth - width / 2 - linewidth);
            g.DrawLine(p, pos.x, pos.y + linewidth, pos.x, pos.y + linewidth + width / 2 + linewidth);
            g.DrawLine(p, pos.x - linewidth, pos.y, pos.x - linewidth - width / 2 - linewidth, pos.y);
            g.DrawLine(p, pos.x + linewidth, pos.y, pos.x + linewidth + width / 2 + linewidth, pos.y);
        }

        public static void drawDepthHandle(Graphics g, AdvData data, Vec2i position, float depth, Color color)
        {
            int ycenter = (int)(depth * data.WalkGridSize - data.WalkGridSize / 2);
            Pen cp = new Pen(color);
            g.DrawLine(cp, position.x, position.y, position.x, ycenter);
            g.FillRectangle(new SolidBrush(color), position.x - 2, ycenter - 2, 5, 5);
            g.DrawRectangle(cp, position.x - 5, ycenter - 5, 10, 10);
        }

        public static int ResourceIDToImageIndex(ResourceID id)
        {
            switch (id)
            {
                case ResourceID.FOLDER:
                    return 1;
            }
            return 0;
        }
    }

    class Resource
    {
        public Resource(TreeNode node)
        {
            mNode = node;
        }

        public ResourceID ID
        {
            get { return (ResourceID)mNode.Tag; }
        }

        public string Name
        {
            get { return mNode.Text; }
        }

        public TreeNode Node
        {
            get { return mNode; }
        }

        private TreeNode mNode;
    }

    public class InputField : Form
    {
        public InputField(string caption)
        {
            this.Size = new Size(200, 100);
            this.FormBorderStyle = FormBorderStyle.FixedToolWindow;
            this.Text = caption;
            box = new TextBox();
            box.Location = new Point(30, 10);
            box.Size = new Size(120, 20);
            this.Controls.Add(box);
            button = new Button();
            button.Text = "Enter";
            button.Location = new Point(50, 35);
            button.Click += new EventHandler(button_Click);
            this.Controls.Add(button);
        }

        void button_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
        }

        public string Input
        {
            get { return this.box.Text; }
            set { this.box.Text = value; }
        }

        private TextBox box;
        private Button button;
    }
}
