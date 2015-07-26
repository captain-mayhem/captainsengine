using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace InTeamPlanner
{
    public partial class Users : Form
    {
        public Users(Storage storage)
        {
            InitializeComponent();
            mStorage = storage;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            mStorage.addUser(login.Text, name.Text, show.Checked);
        }

        Storage mStorage;
    }
}