using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace StoryWriter
{
    public partial class DocumentForm : Form
    {
        public DocumentForm()
        {
            InitializeComponent();
            mDoc = new Document();
            chapters.Columns[3].DefaultCellStyle.ForeColor = Color.Green;
            chapters.Columns[1].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleRight;
            chapters.SelectionChanged += chapters_SelectionChanged;
        }

        void chapters_SelectionChanged(object sender, EventArgs e)
        {
            int idx = chapters.CurrentCell.RowIndex;
            Chapter c = mDoc.getChapter(idx+1);
            sceneView.Rows.Clear();
            foreach (Scene s in c.Scenes)
            {
                sceneView.Rows.Add(s.toArray());
            }
            return;
        }

        public DocumentForm(string filename) : this()
        {
            mDoc = new Document(filename);
            this.Text = mDoc.Title + " " + Properties.Resources.by + " " + mDoc.AuthorName;
            int waccu = 0;
            foreach (Chapter c in mDoc.Chapters)
            {
                chapters.Rows.Add(c.toArray(ref waccu));
                chapters.Rows[chapters.Rows.Count - 1].Cells[0].Style.ForeColor = c.TextColor;
            }
        }

        Document mDoc;
    }
}
