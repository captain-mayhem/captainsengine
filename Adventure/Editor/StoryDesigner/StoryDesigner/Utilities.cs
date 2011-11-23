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

        public static void chooseColor(object sender)
        {
            ColorDialog cd = new ColorDialog();
            if (cd.ShowDialog() == DialogResult.OK)
            {
                Button btn = (Button)sender;
                btn.BackColor = cd.Color;
            }
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
}
