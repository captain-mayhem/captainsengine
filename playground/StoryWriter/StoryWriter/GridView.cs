using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace StoryWriter
{
    class GridView : DataGridView
    {
        public GridView()
        {
            RowHeadersVisible = false;
            GridColor = SystemColors.Control;
            mGridColor = GridColor;
            this.Paint += GridView_Paint;
        }

        void GridView_Paint(object sender, PaintEventArgs e)
        {
            int rowHeight = this.RowTemplate.Height;
            int h = this.ColumnHeadersHeight + rowHeight * this.RowCount;
            int imgWidth = this.Width - 2;

            Rectangle rFrame = new Rectangle(0, 0, imgWidth, rowHeight);
            Rectangle rFill = new Rectangle(1, 1, imgWidth - 2, rowHeight);

            Pen pen = new Pen(this.GridColor, 1);
            Bitmap rowImg = new Bitmap(imgWidth, rowHeight);
            Graphics g = Graphics.FromImage(rowImg);
            g.DrawRectangle(pen, rFrame);
            g.FillRectangle(new SolidBrush(this.DefaultCellStyle.BackColor), rFill);

            int w = 0;
            for (int j = 0; j < this.ColumnCount; j++){
                g.DrawLine(pen, new Point(w, 0), new Point(w, rowHeight));
                w += this.Columns[j].Width;
            }

            int loop = (this.Height - h) / rowHeight;
            for (int j = 0; j < loop + 1; j++)
            {
                e.Graphics.DrawImage(rowImg, 1, h + j * rowHeight);
            }
        }

        public bool ShowGrid
        {
            get { return GridColor == mGridColor; }
            set
            {
                if (value)
                {
                    GridColor = mGridColor;
                }
                else
                    GridColor = DefaultCellStyle.BackColor;

            }
        }

        Color mGridColor;
    }
}
