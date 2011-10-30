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
            mediaPool.AfterLabelEdit += new NodeLabelEditEventHandler(gamePool_AfterLabelEdit);
            gamePool.NodeMouseDoubleClick +=new TreeNodeMouseClickEventHandler(mediaPool_NodeMouseDoubleClick);
            gamePool.MouseDown +=new MouseEventHandler(mediaPool_MouseDown);
            gamePool.AfterLabelEdit += new NodeLabelEditEventHandler(gamePool_AfterLabelEdit);
            newToolStripMenuItem_Click(null, null);
        }

        void gamePool_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            TreeView view = (TreeView)sender;
            view.LabelEdit = false;
            if (e.Label == null || e.Label.Length <= 0 || e.Node.Parent == null)
            {
                e.CancelEdit = true;
                return;
            }
            e.Node.EndEdit(false);
            ResourceID id = (ResourceID)e.Node.Tag;
            switch (id)
            {
                case ResourceID.ITEM:
                    {
                        Item it = mData.removeItem(e.Node.Text);
                        it.Name = e.Label;
                        mData.addItem(it);
                    }
                    break;
                case ResourceID.CHARACTER:
                    {
                        AdvCharacter chr = mData.removeCharacter(e.Node.Text);
                        chr.Name = e.Label;
                        mData.addCharacter(chr);
                    }
                    break;
                case ResourceID.SCRIPT:
                    {
                        Script scr = mData.removeScript(Script.Type.CUTSCENE, e.Node.Text);
                        scr.Name = e.Label;
                        mData.addScript(scr);
                    }
                    break;
                case ResourceID.OBJECT:
                    {
                        AdvObject obj = mData.removeObject(e.Node.Text);
                        obj.Name = e.Label;
                        mData.addObject(obj);
                    }
                    break;
                case ResourceID.ROOM:
                    {
                        Room rm = mData.removeRoom(e.Node.Text);
                        rm.Name = e.Label;
                        mData.addRoom(rm);
                    }
                    break;
            }
            e.Node.Text = e.Label;
            view.Sort();
            view.SelectedNode = e.Node;
        }

        public void showScript(Script.Type type, string name)
        {
            if (mScriptDlg != null)
                mScriptDlg.Close();
            Script scr = mData.getScript(type, name);
            if (scr == null)
            {
                scr = new Script(type);
                scr.Name = name;
                mData.addScript(scr);
            }
            mScriptDlg = new ScriptDlg(scr);
            mScriptDlg.Show(this);
        }

        void mediaPool_MouseDown(object sender, MouseEventArgs e)
        {
            TreeView pool = (TreeView)sender;
            TreeNode node = pool.GetNodeAt(e.Location);
            if (node == null)
                return;
            mSelectedView = pool;
            mSelectedNode = node;
            if (e.Button == MouseButtons.Right)
            {
                menuPool.Show(pool, e.X, e.Y);
            }
            if (e.Clicks > 1)
            {
                TreeNodeMouseClickEventArgs args = new TreeNodeMouseClickEventArgs(node, e.Button, e.Clicks, e.X, e.Y);
                mediaPool_NodeMouseDoubleClick(sender, args);
                return;
            }
            pool.SelectedNode = node;
            ResourceID id = (ResourceID)node.Tag;
            if (id == ResourceID.IMAGE)
            {
                string name = node.Text;
                pool.DoDragDrop(name, DragDropEffects.Copy);
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
            bool sameResource = mLastID == id && mLastName == name;
            mLastName = name;
            mLastID = id;
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
                case ResourceID.ITEM:
                    if (mItemDlg != null)
                        mItemDlg.Close();
                    Item it = mData.getItem(name);
                    if (it == null)
                    {
                        MessageBox.Show("Cannot find item " + name);
                        return;
                    }
                    mItemDlg = new ItemDlg(it);
                    mItemDlg.Show(this);
                    break;
                case ResourceID.OBJECT:
                    if (mObjectDlg != null)
                        mObjectDlg.Close();
                    AdvObject obj = mData.getObject(name);
                    if (obj == null)
                    {
                        MessageBox.Show("Cannot find object " + name);
                        return;
                    }
                    mObjectDlg = new ObjectDlg(obj);
                    mObjectDlg.Show(this);
                    break;
                case ResourceID.CHARACTER:
                    if (mCharacterDlg != null)
                        mCharacterDlg.Close();
                    AdvCharacter chr = mData.getCharacter(name);
                    if (chr == null)
                    {
                        MessageBox.Show("Cannot find character " + name);
                        return;
                    }
                    mCharacterDlg = new CharacterDlg(chr, mData);
                    mCharacterDlg.Show(this);
                    break;
                case ResourceID.ROOM:
                    Room room = mData.getRoom(name);
                    if (room == null)
                    {
                        MessageBox.Show("Cannot find room " + name);
                        return;
                    }
                    Point pos = new Point(200, 200);
                    if (mRoomDlg != null)
                    {
                        if (mRoomDlg.Room == room && mRoomDlg.Visible)
                        {
                            mRoomDlg.Show();
                            break;
                        }
                        pos = mRoomDlg.Location;
                        mRoomDlg.Close();
                    }
                    mRoomDlg = new RoomDlg(room, mData, mRoomView);
                    mRoomDlg.StartPosition = FormStartPosition.Manual;
                    mRoomDlg.Location = pos;
                    mRoomDlg.Show(this);
                    break;
                case ResourceID.SCRIPT:
                    showScript(Script.Type.CUTSCENE, name);
                    break;
                default:
                    Console.WriteLine("Clicked " + name + " " + id + " unhandled");
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
            fod.Filter = "Adventure project file|*.adv|Adventure game file|game.dat";
            fod.InitialDirectory = "D:\\pcdk\\Sinnlos im Weltraum - The First Adventure\\Sinnlos im Weltraum - The First Adventure\\data";
            fod.ShowDialog();
            if (fod.FileName.Length > 0)
            {
                loadFile(fod.FileName);
            }
        }

        private void loadFile(string filename)
        {
            try
            {
                AdvFileReader reader = new AdvFileReader(filename, mediaPool, gamePool);
                mData = reader.Data;
            }
            catch (System.IO.FileNotFoundException)
            {
                MessageBox.Show("Cannot load " + filename);
            }
        }

        private AdvData mData;
        private ImageViewer mImageViewer;
        private MouseIcons mMouseIcons;
        private ItemDlg mItemDlg;
        private ObjectDlg mObjectDlg;
        private CharacterDlg mCharacterDlg;
        private ScriptDlg mScriptDlg;
        private RoomDlg mRoomDlg;
        private string mLastName;
        private ResourceID mLastID;
        private RoomDlg.ViewMode mRoomView = RoomDlg.ViewMode.Objects;
        private TreeView mSelectedView;
        private TreeNode mSelectedNode;

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
            Fonts fnts = new Fonts(mData);
            fnts.ShowDialog(this);
        }

        private void mouseiconsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (mMouseIcons != null)
                mMouseIcons.Close();
            mMouseIcons = new MouseIcons(mData);
            mMouseIcons.Show(this);
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
                TreeNode parent;
                ResourceID res = determineTypeAndFolder(mediaPool, out parent);
                if (res != ResourceID.IMAGE)
                    parent = mediaPool.Nodes[0];
                TreeNode newnode = new TreeNode(file);
                newnode.Tag = ResourceID.IMAGE;
                parent.Nodes.Add(newnode);
            }
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mData = new AdvData();
            gamePool.Nodes.Clear();
            gamePool.Nodes.Add("CHARACTER");
            gamePool.Nodes.Add("SCRIPTS");
            gamePool.Nodes.Add("ITEMS");
            gamePool.Nodes.Add("OBJECTS");
            gamePool.Nodes.Add("ROOMS");
            mediaPool.Nodes.Clear();
            mediaPool.Nodes.Add("IMAGES");
            mediaPool.Nodes.Add("MUSIC");
            mediaPool.Nodes.Add("SOUNDS");
            mediaPool.Nodes.Add("VIDEO");
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sod = new SaveFileDialog();
            sod.Filter = "Adventure project files|*.adv";
            sod.InitialDirectory = mData.Settings.Directory;
            sod.ShowDialog();
        }

        private void createGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            /*SaveFileDialog sod = new SaveFileDialog();
            sod.Filter = "Adventure game|*.exe";
            sod.InitialDirectory = mData.Settings.Directory;
            sod.ShowDialog();*/

            AdvFileWriter writer = new AdvFileWriter(mData);
            writer.writeGame(".");
        }

        private void walkmapOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ToolStripDropDownItem item = (ToolStripDropDownItem)sender;
            if (mRoomView != RoomDlg.ViewMode.Walkmap)
            {
                item.Text = "Walkmap off";
                mRoomView = RoomDlg.ViewMode.Walkmap;
            }
            else
            {
                item.Text = "Walkmap on";
                mRoomView = RoomDlg.ViewMode.Objects;
            }
            if (mRoomDlg != null)
                mRoomDlg.Viewmode = mRoomView;
        }

        private void gamepool_add_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID id = determineTypeAndFolder(mSelectedView, out parent);
            switch (id)
            {
                case ResourceID.ITEM:
                    itemToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.CHARACTER:
                    characterToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.SCRIPT:
                    scriptToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.OBJECT:
                    objectToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.ROOM:
                    roomToolStripMenuItem_Click(null, null);
                    break;
                case ResourceID.IMAGE:
                    imageToolStripMenuItem_Click(null, null);
                    break;
            }
        }

        private ResourceID determineTypeAndFolder(TreeView view, out TreeNode parent)
        {
            TreeNode node = view.SelectedNode;
            if (node == null)
                node = view.Nodes[0];

            parent = node;
            if ((ResourceID)parent.Tag != ResourceID.FOLDER)
                parent = node.Parent;

            while (node.Parent != null)
                node = node.Parent;
            
            if (view == gamePool)
            {
                if (node == gamePool.Nodes[0])
                    return ResourceID.CHARACTER;
                else if (node == gamePool.Nodes[1])
                    return ResourceID.SCRIPT;
                else if (node == gamePool.Nodes[2])
                    return ResourceID.ITEM;
                else if (node == gamePool.Nodes[3])
                    return ResourceID.OBJECT;
                else if (node == gamePool.Nodes[4])
                    return ResourceID.ROOM;
            }
            else if (view == mediaPool)
            {
                if (node == mediaPool.Nodes[0])
                    return ResourceID.IMAGE;
                else if (node == mediaPool.Nodes[1])
                    return ResourceID.MUSIC;
                else if (node == mediaPool.Nodes[2])
                    return ResourceID.SOUND;
                else if (node == mediaPool.Nodes[3])
                    return ResourceID.VIDEO;
            }
            return ResourceID.INVALID;
        }

        private void renameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mSelectedView.LabelEdit = true;
            mSelectedNode.BeginEdit();
        }

        private void itemToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.ITEM)
                parent = gamePool.Nodes[2];
            Item it = new Item(mData, 10);
            mData.addItem(it);
            TreeNode node = new TreeNode(it.Name);
            node.Tag = ResourceID.ITEM;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void characterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.CHARACTER)
                parent = gamePool.Nodes[0];
            AdvCharacter chr = new AdvCharacter(mData, 36);
            mData.addCharacter(chr);
            TreeNode node = new TreeNode(chr.Name);
            node.Tag = ResourceID.CHARACTER;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void scriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.SCRIPT)
                parent = gamePool.Nodes[1];
            Script scr = new Script(Script.Type.CUTSCENE);
            scr.Name = "Script" + (mData.NumCutscenes + 1);
            mData.addScript(scr);
            TreeNode node = new TreeNode(scr.Name);
            node.Tag = ResourceID.SCRIPT;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void objectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.OBJECT)
                parent = gamePool.Nodes[3];
            AdvObject obj = new AdvObject(mData, 10);
            mData.addObject(obj);
            TreeNode node = new TreeNode(obj.Name);
            node.Tag = ResourceID.OBJECT;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void roomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(gamePool, out parent);
            if (res != ResourceID.ROOM)
                parent = gamePool.Nodes[4];
            Room rm = new Room(mData);
            mData.addRoom(rm);
            TreeNode node = new TreeNode(rm.Name);
            node.Tag = ResourceID.ROOM;
            parent.Nodes.Add(node);
            gamePool.Sort();
            gamePool.SelectedNode = node;
            gamePool.LabelEdit = true;
            node.BeginEdit();
        }

        private void newSubdirectoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode parent;
            ResourceID res = determineTypeAndFolder(mSelectedView, out parent);
            if (res == ResourceID.FOLDER)
                parent = mSelectedView.SelectedNode;
            int count = 0;
            foreach (TreeNode n in parent.Nodes)
            {
                if ((ResourceID)n.Tag == ResourceID.FOLDER)
                    ++count;
            }
            TreeNode node = new TreeNode("Folder"+(count+1));
            node.Tag = ResourceID.FOLDER;
            parent.Nodes.Add(node);
            mSelectedView.Sort();
            mSelectedView.SelectedNode = node;
            mSelectedView.LabelEdit = true;
            node.BeginEdit();
        }
    }
}