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
    public partial class MainWindow : Form
    {
        public MainWindow()
        {
            InitializeComponent();
            mFont = new Font("Arial", 12);
            mMainColor = Brushes.Black;
            mDocument = new TextView();
            mDocument.MdiParent = this;
            mDocument.Show();
            mDocument.WindowState = FormWindowState.Normal;
            mDocument.WindowState = FormWindowState.Maximized;
            this.KeyDown += new KeyEventHandler(MainWindow_KeyDown);
            this.PreviewKeyDown += new PreviewKeyDownEventHandler(MainWindow_PreviewKeyDown);
            RemoveCursorNavigation(this.Controls);
            Focus();
        }

        private void RemoveCursorNavigation(Control.ControlCollection controls)
        {
            foreach (Control ctrl in controls)
            {
                ctrl.PreviewKeyDown += new PreviewKeyDownEventHandler(MainWindow_PreviewKeyDown);
                RemoveCursorNavigation(ctrl.Controls);
            }
        }

        void MainWindow_KeyDown(object sender, KeyEventArgs e)
        {
            mDocument.TextView_KeyDown(sender, e);
            e.Handled = true;
        }

        void MainWindow_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Right:
                case Keys.Left:
                case Keys.Up:
                case Keys.Down:
                    e.IsInputKey = true;
                    break;
            }
        }

        Font mFont;
        Brush mMainColor;
        TextView mDocument;
    }
}
