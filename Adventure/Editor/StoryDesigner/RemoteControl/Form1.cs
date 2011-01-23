using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace RemoteControl
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            mSettings = new Settings();
            this.Resize += new EventHandler(MainForm_Resize);
            modeTabs.TabPages["remoteTab"].MouseMove += new MouseEventHandler(MainForm_MouseMove);
            modeTabs.TabPages["remoteTab"].MouseClick += new MouseEventHandler(MainForm_MouseClick);
        }

        void MainForm_Resize(object sender, EventArgs e)
        {
            System.Windows.Forms.Form frm = (System.Windows.Forms.Form)sender;
            modeTabs.Size = frm.ClientSize;
        }

        void MainForm_MouseClick(object sender, MouseEventArgs e)
        {
            if (mSocket == null)
                return;
            System.Windows.Forms.TabPage frm = (System.Windows.Forms.TabPage)sender;
            if ((e.Button & MouseButtons.Left) != 0)
                mSockStream.WriteLine("mcl {0} {1}", e.X * mCGEResX / frm.ClientSize.Width, e.Y * mCGEResY / frm.ClientSize.Height);
            else if ((e.Button & MouseButtons.Right) != 0)
                mSockStream.WriteLine("mcr {0} {1}", e.X * mCGEResX / frm.ClientSize.Width, e.Y * mCGEResY / frm.ClientSize.Height);
            mSockStream.Flush();
        }

        void MainForm_MouseMove(object sender, MouseEventArgs e)
        {
            if (mSocket == null)
                return;
            System.Windows.Forms.TabPage frm = (System.Windows.Forms.TabPage)sender;
            try
            {
                mSockStream.WriteLine("mps {0} {1}", e.X * mCGEResX / frm.ClientSize.Width, e.Y *mCGEResY / frm.ClientSize.Height);
                mSockStream.Flush();
            }
            catch (System.IO.IOException)
            {
            }
        }

        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                mSocket = new System.Net.Sockets.TcpClient();
                mSocket.Connect(mSettings.Hostname, mSettings.Port);
                System.IO.StreamReader reader = new System.IO.StreamReader(mSocket.GetStream());
                string hello = reader.ReadLine();
                string[] data = hello.Split(' ');
                mCGEResX = Convert.ToInt32(data[1]);
                mCGEResY = Convert.ToInt32(data[2]);
                mSockStream = new System.IO.StreamWriter(mSocket.GetStream());
            }
            catch (System.Net.Sockets.SocketException)
            {
                mSocket = null;
                MessageBox.Show("Could not connect to CGE");
            }
        }

        System.Net.Sockets.TcpClient mSocket;
        System.IO.StreamWriter mSockStream;
        Settings mSettings;
        int mCGEResX;
        int mCGEResY;

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mSettings.ShowDialog();
        }

        private void disconnectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mSocket.Close();
            mSocket = null;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (mSocket == null)
                return;
            try
            {
                mSockStream.WriteLine("scr");
                mSockStream.WriteLine(this.script.Text);
                mSockStream.WriteLine("***");
                mSockStream.Flush();
            }
            catch (System.IO.IOException)
            {
            }
        }
    }
}