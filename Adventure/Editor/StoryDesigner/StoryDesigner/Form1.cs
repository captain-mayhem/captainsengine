using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            mediaPool.NodeMouseDoubleClick += new TreeNodeMouseClickEventHandler(mediaPool_NodeMouseDoubleClick);
        }

        void mediaPool_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            ResourceID id = (ResourceID)e.Node.Tag;
            string name = e.Node.Text.ToLower();
            switch (id)
            {
                case ResourceID.IMAGE:
                    string filename = mData.Images[name];
                    System.Drawing.Bitmap bmp = mData.getImage(filename);
                    break;
                default:
                    Console.WriteLine("Clicked " + name + " " + id + "unhandled");
                    break;
            }
        }

        private void MainForm_Load(object sender, EventArgs e)
        {

        }

        private void menuStrip1_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {

        }

        private void beendenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Adventure game files (game.dat)|game.dat";
            fod.InitialDirectory = "D:\\pcdk\\Sinnlos im Weltraum - The First Adventure\\Sinnlos im Weltraum - The First Adventure\\data";
            fod.ShowDialog();
            if (fod.FileName.Length > 0)
            {
                AdvFileReader reader = new AdvFileReader(fod.FileName, mediaPool, gamePool);
                mData = reader.Data;
            }
        }

        private AdvData mData;
    }
}