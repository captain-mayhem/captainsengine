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
        }

        public DocumentForm(string filename)
        {
            InitializeComponent();
            mDoc = new Document(filename);
            this.Text = mDoc.Title + " " + Properties.Resources.by + " " + mDoc.AuthorName;
        }

        Document mDoc;
    }
}
