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
            mSocket = new System.Net.Sockets.TcpClient();
            this.MouseMove += new MouseEventHandler(MainForm_MouseMove);
            this.MouseClick += new MouseEventHandler(MainForm_MouseClick);
        }

        void MainForm_MouseClick(object sender, MouseEventArgs e)
        {
            if (!mSocket.Connected)
                return;
            if ((e.Button & MouseButtons.Left) != 0)
                mSockStream.WriteLine("mc {0} {1}", e.X, e.Y);
            else if ((e.Button & MouseButtons.Right) != 0)
                mSockStream.WriteLine("mr {0} {1}", e.X, e.Y);
            mSockStream.Flush();
        }

        void MainForm_MouseMove(object sender, MouseEventArgs e)
        {
            if (!mSocket.Connected)
                return;
            mSockStream.WriteLine("mp {0} {1}", e.X, e.Y);
            mSockStream.Flush();
        }

        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                mSocket.Connect(mSettings.Hostname, mSettings.Port);
                mSockStream = new System.IO.StreamWriter(mSocket.GetStream());
            }
            catch (System.Net.Sockets.SocketException)
            {
                MessageBox.Show("Could not connect to CGE");
            }
        }

        System.Net.Sockets.TcpClient mSocket;
        System.IO.StreamWriter mSockStream;
        Settings mSettings;

        private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mSettings.ShowDialog();
        }
    }
}