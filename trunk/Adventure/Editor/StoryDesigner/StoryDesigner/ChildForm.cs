using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ChildForm : Form
    {
        public ChildForm()
        {
            InitializeComponent();
            this.Resize += new EventHandler(ChildForm_Resize);
        }

        void ChildForm_Resize(object sender, EventArgs e)
        {
            if (this.WindowState != FormWindowState.Normal)
                this.WindowState = FormWindowState.Normal;
        }

    }
}
