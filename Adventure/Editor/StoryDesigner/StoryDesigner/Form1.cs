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
            mediaPool.MouseDown += new MouseEventHandler(mediaPool_MouseDown);
        }

        void mediaPool_MouseDown(object sender, MouseEventArgs e)
        {
            TreeNode node = mediaPool.GetNodeAt(e.Location);
            if (node == null)
                return;
            mediaPool.SelectedNode = node;
            ResourceID id = (ResourceID)node.Tag;
            if (id == ResourceID.IMAGE)
            {
                string name = node.Text;
                mediaPool.DoDragDrop(name, DragDropEffects.Copy);
            }
        }

        public MainForm(string filename) : this()
        {
            loadFile(filename);
        }

        void mediaPool_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            ResourceID id = (ResourceID)e.Node.Tag;
            string name = e.Node.Text.ToLower();
            switch (id)
            {
                case ResourceID.IMAGE:
                    System.Drawing.Bitmap bmp = mData.getImage(name);
                    if (bmp == null)
                    {
                        MessageBox.Show("Cannot find image " + name);
                        return;
                    }
                    mImageViewer = new ImageViewer(bmp);
                    mImageViewer.Show(this);
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
                loadFile(fod.FileName);
            }
        }

        private void loadFile(string filename)
        {
            AdvFileReader reader = new AdvFileReader(filename, mediaPool, gamePool);
            mData = reader.Data;
        }

        private AdvData mData;
        private ImageViewer mImageViewer;

        private void projectSetupToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Settings settings = new Settings(mData);
            settings.ShowDialog(this);
        }

        private void commandsAndBoolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CommandsAndBools cabs = new CommandsAndBools(mData);
            cabs.ShowDialog(this);
        }

        private void fontsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Fonts fnts = new Fonts();
            fnts.ShowDialog(this);
        }

        private void mouseiconsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MouseIcons icons = new MouseIcons(mData);
            icons.Show(this);
        }

        private void imageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog fod = new OpenFileDialog();
            fod.Filter = "Image files|*.jpg;*.jpeg;*.png;*.bmp,*.tga;*.gif";
            fod.InitialDirectory = "D:\\pcdk\\Sinnlos im Weltraum - The First Adventure\\Sinnlos im Weltraum - The First Adventure\\data";
            if (fod.ShowDialog() == DialogResult.OK)
            {
                string filename = fod.FileName;
                string file = System.IO.Path.GetFileNameWithoutExtension(filename);
                try
                {
                    mData.Images.Add(file, filename);
                }
                catch (ArgumentException)
                {
                    MessageBox.Show("Image with same name already added");
                    return;
                }
                TreeNode node = mediaPool.SelectedNode;
                if (node == null)
                    node = mediaPool.Nodes[0];
                else
                {
                    TreeNode parent = node;
                    while (parent.Parent != null)
                        parent = parent.Parent;
                    if (parent != mediaPool.Nodes[0])
                        node = mediaPool.Nodes[0];
                }
                if ((ResourceID)node.Tag != ResourceID.FOLDER)
                    node = node.Parent;
                TreeNode newnode = new TreeNode(file);
                newnode.Tag = ResourceID.IMAGE;
                node.Nodes.Add(newnode);
            }
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mData = new AdvData();
        }
    }
}