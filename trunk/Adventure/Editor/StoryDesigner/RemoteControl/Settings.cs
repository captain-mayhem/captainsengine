using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace RemoteControl
{
    public partial class Settings : Form
    {
        public Settings()
        {
            InitializeComponent();
            hostname.Text = "127.0.0.1";
            port.Text = "28406";
        }

        public int Port
        {
            get { return Convert.ToInt32(port.Text); }
        }

        public string Hostname
        {
            get { return hostname.Text; }
        }
    }
}