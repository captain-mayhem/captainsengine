using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;

namespace StoryDesigner
{
    public partial class Fonts : Form
    {
        public Fonts(AdvData data)
        {
            InitializeComponent();
            mData = data;
            System.Drawing.Text.InstalledFontCollection fonts = new System.Drawing.Text.InstalledFontCollection();
            foreach (FontFamily fam in fonts.Families)
            {
                if (fam.IsStyleAvailable(FontStyle.Regular))
                    font.Items.Add(fam.Name);
            }
            mFonts = data.Settings.Fonts;
            fontlist.SelectedIndexChanged += new EventHandler(fontlist_SelectedIndexChanged);
            preview.Paint += new PaintEventHandler(preview_Paint);
            font.DrawItem += new DrawItemEventHandler(font_DrawItem);

            for (int i = 0; i < mFonts.Count; ++i)
            {
                fontlist.Items.Add("");
                setListName(i, mFonts[i] as FontInfo);
            }

            charset.Items.Add("Western");
            charset.Items.Add("East european");
            charset.Items.Add("Russian");
            charset.Items.Add("Greek");
            charset.Items.Add("Turkish");
            charset.Items.Add("Arabic");
            charset.Items.Add("Hebrew");
            charset.Items.Add("Vietnamese");
            charset.Items.Add("Thai");
            charset.Items.Add("Baltic");
            charset.Items.Add("Japanese ShiftJIS");
            charset.Items.Add("Dos OEM");
            charset.Items.Add("Symbol");

            for (int i = 6; i <= 28; ++i)
            {
                fontsize.Items.Add(Convert.ToString(i));
            }

            if (fontlist.Items.Count > 0)
                fontlist.SelectedIndex = 0;
            else
            {
                //TODO disable controls
            }
        }

        void font_DrawItem(object sender, DrawItemEventArgs e)
        {
            if (e.Index == -1 || e.Index >= font.Items.Count)
                return;
            e.DrawBackground();
            if ((e.State & DrawItemState.Focus) != 0)
                e.DrawFocusRectangle();
            string fontname = font.Items[e.Index] as string;
            Font f = new Font(fontname, font.Font.Size);
            Brush b = new SolidBrush(e.ForeColor);
            e.Graphics.DrawString(fontname, f, b, e.Bounds);
            b.Dispose();
        }

        public int[] createBitmapFont(int idx, out Bitmap[] images, out Bitmap[] alphaimages, out Vec2i dims, out Vec2i numchars)
        {
            //font setup
            mCurrInfo = (FontInfo)mData.Settings.Fonts[idx - 1];
            mCurrFont = new Font(mCurrInfo.name, mCurrInfo.size);
            if (mCurrInfo.bold)
                mCurrFont = new Font(mCurrFont, FontStyle.Bold);
            if (mCurrInfo.italic)
                mCurrFont = new Font(mCurrFont, FontStyle.Italic);
            int outlinethickness = 0;
            int shadowthickness = 0;
            if (mCurrInfo.outline == 1 || mCurrInfo.outline == 2)
                outlinethickness = 1;
            else if (mCurrInfo.outline == 3)
                outlinethickness = 2;
            if (mCurrInfo.shadow != 0)
                shadowthickness = 3;

            //character dimension measuring
            Bitmap bmp = new Bitmap(256, 256);
            Graphics g = Graphics.FromImage(bmp);
            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
            int maxwidth = 0, maxheight = 0;
            int[] charwidths = new int[224];
            StringFormat fmt = new StringFormat();
            fmt.FormatFlags = StringFormatFlags.NoClip;
            CharacterRange[] range = new CharacterRange[1];
            range[0] = new CharacterRange(0,1);
            fmt.SetMeasurableCharacterRanges(range);
            Encoding enc = Encoding.GetEncoding(1252);
            byte [] b = new byte[1];
            for (int i = 0; i < 224; ++i)
            {
                b[0] = (byte)(i + 0x20);
                char[] chs = enc.GetChars(b);
                char ch = chs[0];
                Region[] reg = g.MeasureCharacterRanges(new string(ch, 1), mCurrFont, new RectangleF(0,0,256,256), fmt);
                RectangleF rect = reg[0].GetBounds(g);
                int width = (int)rect.Width;///*Math.Ceiling(*/ f.Width;//);
                if (width == 0)
                {
                    SizeF f = g.MeasureString(new string(ch, 1), mCurrFont);
                    width = (int)f.Width;
                }
                charwidths[i] = width+outlinethickness;
                maxwidth = Math.Max(maxwidth, (int)(rect.Width+rect.X));
                int height = (int)(rect.Height + rect.Y);
                if (height > 256)
                    continue;
                maxheight = Math.Max(maxheight, height);
            }
            if (maxheight == 0)
                maxheight = maxwidth;//a crappy workaround to compensate for crappy fonts
            int charwidth = maxwidth+outlinethickness+shadowthickness;
            int charheight = maxheight+2+outlinethickness+shadowthickness;
            int charsperrow = 256 / charwidth;
            int numrows = 256 / charheight;
            int numtextures = (int)Math.Ceiling(224.0 / (charsperrow * numrows));

            //allocate bitmaps
            images = new Bitmap[numtextures];
            alphaimages = new Bitmap[numtextures];
            int currtexture = -1;
            Graphics g2 = null;

            //render bitmap font
            for (int i = 0; i < 224; ++i)
            {
                b[0] = (byte)(i + 0x20);
                char[] chs = enc.GetChars(b);
                char ch = chs[0];
                int texture = i / (numrows * charsperrow);
                if (texture != currtexture)
                {
                    images[texture] = new Bitmap(256, 256, PixelFormat.Format16bppRgb565);
                    g.Dispose();
                    g = Graphics.FromImage(images[texture]);
                    g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;
                    g.SmoothingMode = SmoothingMode.AntiAlias;
                    g.Clear(Color.Black);
                    alphaimages[texture] = new Bitmap(256, 256, PixelFormat.Format16bppRgb565);
                    if (g2 != null)
                        g2.Dispose();
                    g2 = Graphics.FromImage(alphaimages[texture]);
                    g2.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;
                    g2.SmoothingMode = SmoothingMode.AntiAlias;
                    g2.Clear(Color.Black);
                    currtexture = texture;
                }
                int row = i / charsperrow - texture*numrows;
                int column = i % charsperrow;
                drawText(g, column * charwidth, row * charheight, new string(ch, 1), fmt);
                drawText(g2, column * charwidth, row * charheight, new string(ch, 1), fmt, true); 
            }
            g.Dispose();
            g2.Dispose();
            dims = new Vec2i(charwidth, charheight);
            numchars = new Vec2i(charsperrow, numrows);
            return charwidths;
        }

        void preview_Paint(object sender, PaintEventArgs e)
        {
            if (mCurrInfo == null)
                return;
            PictureBox obj = (PictureBox)sender;
            StringFormat fmt = new StringFormat();
            fmt.Alignment = StringAlignment.Center;
            drawText(e.Graphics, obj.ClientSize.Width / 2, 0, "AaBbCcXxYyZz", fmt);
            drawText(e.Graphics, obj.ClientSize.Width / 2, obj.ClientSize.Height / 2 - mCurrFont.Height / 2, "0123456789", fmt);
            drawText(e.Graphics, obj.ClientSize.Width / 2, obj.ClientSize.Height - mCurrFont.Height, "!?ÄÖÜ$%§ÓÍ+", fmt);
        }

        void drawText(Graphics g, int x, int y, string text, StringFormat fmt)
        {
            drawText(g, x, y, text, fmt, false);
        }

        void drawText(Graphics g, int x, int y, string text, StringFormat fmt, bool allwhite)
        {
            Utilities.drawText(g, x, y, text, mCurrFont, mCurrInfo, fmt, allwhite);
        }

        void drawPreview(Graphics g, int width, int height, Brush brush)
        {
            StringFormat fmt = new StringFormat();
            fmt.Alignment = StringAlignment.Center;
            if (brush is LinearGradientBrush)
            {
                LinearGradientBrush lb = (LinearGradientBrush)brush;
                lb.Transform.Translate(0, 0);
                
            }
            g.DrawString("AaBbCcXxYyZz", mCurrFont, brush, width / 2, 0, fmt);
            //fmt.LineAlignment = StringAlignment.Center;
            if (brush is LinearGradientBrush)
            {
                LinearGradientBrush lb = (LinearGradientBrush)brush;
                brush = new LinearGradientBrush(new Point(0, height / 2 - mCurrFont.Height / 2), new Point(0, height / 2 + mCurrFont.Height / 2), lb.LinearColors[0], lb.LinearColors[1]);
            }
            g.DrawString("0123456789", mCurrFont, brush, width / 2, height / 2 - mCurrFont.Height / 2, fmt);
            //fmt.LineAlignment = StringAlignment.Far;
            if (brush is LinearGradientBrush)
            {
                LinearGradientBrush lb = (LinearGradientBrush)brush;
                brush = new LinearGradientBrush(new Point(0, height - mCurrFont.Height), new Point(0, height), lb.LinearColors[0], lb.LinearColors[1]);
            }
            g.DrawString("!?ÄÖÜ$%§ÓÍ+", mCurrFont, brush, width / 2, height - mCurrFont.Height, fmt);
        }

        void fontlist_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (fontlist.SelectedIndex == -1)
                return;
            FontInfo info = (FontInfo)mFonts[fontlist.SelectedIndex];
            mCurrInfo = info;
            mCurrFont = new Font(info.name, info.size);
            if (info.bold)
                mCurrFont = new Font(mCurrFont, FontStyle.Bold);
            if (info.italic)
                mCurrFont = new Font(mCurrFont, FontStyle.Italic);
            font.SelectedItem = info.name;
            charset.SelectedIndex = info.charset;
            bold.Checked = info.bold;
            italic.Checked = info.italic;
            fontsize.SelectedItem = Convert.ToString(info.size);
            switch (info.outline)
            {
                case 0:
                    ol_none.Checked = true;
                    break;
                case 1:
                    ol_shadow_a.Checked = true;
                    break;
                case 2:
                    ol_shadow_b.Checked = true;
                    break;
                case 3:
                    ol_shadow_ab.Checked = true;
                    break;
            }
            switch (info.shadow)
            {
                case 0:
                    sh_none.Checked = true;
                    break;
                case 1:
                    sh_25.Checked = true;
                    break;
                case 2:
                    sh_50.Checked = true;
                    break;
                case 3:
                    sh_75.Checked = true;
                    break;
            }
            switch (info.fill)
            {
                case 0:
                    fill_normal.Checked = true;
                    break;
                case 1:
                    fill_falloff_1.Checked = true;
                    break;
                case 2:
                    fill_falloff_2.Checked = true;
                    break;
                case 3:
                    fill_scanlines.Checked = true;
                    break;
            }
            fading.Value = info.fading;
            preview.Invalidate();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
        }

        System.Collections.ArrayList mFonts;
        FontInfo mCurrInfo;
        Font mCurrFont;
        AdvData mData;

        private void fading_ValueChanged(object sender, EventArgs e)
        {
            NumericUpDown num = fading;
            if (num.Value > 1600)
                fading_indicator.Text = "really slow";
            else if (num.Value > 1200)
                fading_indicator.Text = "slow";
            else if (num.Value > 800)
                fading_indicator.Text = "average";
            else if (num.Value > 400)
                fading_indicator.Text = "fast";
            else if (num.Value > 0)
                fading_indicator.Text = "very fast";
            if (num.Value == 0)
                fading_indicator.Text = "<none>";
            mCurrInfo.fading = (int)num.Value;
        }

        private void fontsize_SelectedIndexChanged(object sender, EventArgs e)
        {
            mCurrInfo.size = Convert.ToInt32(fontsize.SelectedItem);
            mCurrFont = new Font(mCurrFont.FontFamily, mCurrInfo.size, mCurrFont.Style);
            preview.Invalidate();
            setListName(fontlist.SelectedIndex, mCurrInfo);
        }

        private void ol_none_CheckedChanged(object sender, EventArgs e)
        {
            if (ol_none.Checked)
            {
                mCurrInfo.outline = 0;
                preview.Invalidate();
            }
        }

        private void ol_shadow_a_CheckedChanged(object sender, EventArgs e)
        {
            if (ol_shadow_a.Checked)
            {
                mCurrInfo.outline = 1;
                preview.Invalidate();
            }
        }

        private void ol_shadow_b_CheckedChanged(object sender, EventArgs e)
        {
            if (ol_shadow_b.Checked)
            {
                mCurrInfo.outline = 2;
                preview.Invalidate();
            }
        }

        private void ol_shadow_ab_CheckedChanged(object sender, EventArgs e)
        {
            if (ol_shadow_ab.Checked)
            {
                mCurrInfo.outline = 3;
                preview.Invalidate();
            }
        }

        private void sh_none_CheckedChanged(object sender, EventArgs e)
        {
            if (sh_none.Checked)
            {
                mCurrInfo.shadow = 0;
                preview.Invalidate();
            }
        }

        private void sh_25_CheckedChanged(object sender, EventArgs e)
        {
            if (sh_25.Checked)
            {
                mCurrInfo.shadow = 1;
                preview.Invalidate();
            }
        }

        private void sh_50_CheckedChanged(object sender, EventArgs e)
        {
            if (sh_50.Checked)
            {
                mCurrInfo.shadow = 2;
                preview.Invalidate();
            }
        }

        private void sh_75_CheckedChanged(object sender, EventArgs e)
        {
            if (sh_75.Checked)
            {
                mCurrInfo.shadow = 3;
                preview.Invalidate();
            }
        }

        private void bold_CheckedChanged(object sender, EventArgs e)
        {
            mCurrInfo.bold = bold.Checked;
            if (bold.Checked)
            {
                mCurrFont = new Font(mCurrFont, FontStyle.Bold | (mCurrInfo.italic ? FontStyle.Italic : FontStyle.Regular));
            }
            else
            {
                mCurrFont = new Font(mCurrFont, mCurrInfo.italic ? FontStyle.Italic : FontStyle.Regular);
            }
            preview.Invalidate();
        }

        private void italic_CheckedChanged(object sender, EventArgs e)
        {
            mCurrInfo.italic = italic.Checked;
            if (italic.Checked)
            {
                mCurrFont = new Font(mCurrFont, FontStyle.Italic | (mCurrInfo.bold ? FontStyle.Bold : FontStyle.Regular));
            }
            else
            {
                mCurrFont = new Font(mCurrFont, mCurrInfo.bold ? FontStyle.Bold : FontStyle.Regular);
            }
            preview.Invalidate();
        }

        private void font_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (mCurrInfo == null)
                return;
            mCurrInfo.name = (string)font.SelectedItem;
            mCurrFont = new Font(mCurrInfo.name, mCurrInfo.size, mCurrFont.Style);
            preview.Invalidate();
            setListName(fontlist.SelectedIndex, mCurrInfo);
        }

        private void fill_normal_CheckedChanged(object sender, EventArgs e)
        {
            if (fill_normal.Checked)
            {
                mCurrInfo.fill = 0;
                preview.Invalidate();
            }
        }

        private void fill_falloff_1_CheckedChanged(object sender, EventArgs e)
        {
            if (fill_falloff_1.Checked)
            {
                mCurrInfo.fill = 1;
                preview.Invalidate();
            }
        }

        private void fill_falloff_2_CheckedChanged(object sender, EventArgs e)
        {
            if (fill_falloff_2.Checked)
            {
                mCurrInfo.fill = 2;
                preview.Invalidate();
            }
        }

        private void fill_scanlines_CheckedChanged(object sender, EventArgs e)
        {
            if (fill_scanlines.Checked)
            {
                mCurrInfo.fill = 3;
                preview.Invalidate();
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            FontInfo info = new FontInfo();
            info.name = "Arial";
            info.size = 12;
            mFonts.Add(info);
            fontlist.Items.Add(info.name);
            fontlist.SelectedIndex = fontlist.Items.Count - 1;
        }

        private void setListName(int idx, FontInfo info)
        {
            if (idx < 0)
                return;
            fontlist.Items[idx] = (idx+1)+" - "+info.name + " ("+info.size+")";
        }

        private void charset_SelectedIndexChanged(object sender, EventArgs e)
        {
            mCurrInfo.charset = charset.SelectedIndex;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            int idx = fontlist.SelectedIndex;
            mFonts.RemoveAt(idx);
            fontlist.Items.RemoveAt(idx);
            if (idx >= fontlist.Items.Count)
            {
                --idx;
            }
            if (idx >= 0)
            {
                fontlist.SelectedIndex = idx;
            }
        }

        private void font_up_Click(object sender, EventArgs e)
        {
            int idx = fontlist.SelectedIndex;
            if (idx <= 0)
                return;
            FontInfo info = mFonts[idx] as FontInfo;
            mFonts.RemoveAt(idx);
            --idx;
            mFonts.Insert(idx, info);
            for (int i = idx; i < mFonts.Count; ++i)
            {
                setListName(i, mFonts[i] as FontInfo);
            }
            fontlist.SelectedIndex = idx;
        }

        private void button_down_Click(object sender, EventArgs e)
        {
            int idx = fontlist.SelectedIndex;
            if (idx >= fontlist.Items.Count-1)
                return;
            FontInfo info = mFonts[idx] as FontInfo;
            mFonts.RemoveAt(idx);
            ++idx;
            mFonts.Insert(idx, info);
            for (int i = idx-1; i < mFonts.Count; ++i)
            {
                setListName(i, mFonts[i] as FontInfo);
            }
            fontlist.SelectedIndex = idx;
        }
    }
}