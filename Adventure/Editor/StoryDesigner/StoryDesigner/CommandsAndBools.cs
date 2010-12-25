using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class CommandsAndBools : Form
    {
        private class BoolAdder : Form
        {
            public BoolAdder()
            {
                this.Size = new Size(200,100);
                this.FormBorderStyle = FormBorderStyle.FixedToolWindow;
                this.Text = "Enter Name";
                box = new TextBox();
                box.Location = new Point(30, 10);
                box.Size = new Size(120, 20);
                this.Controls.Add(box);
                button = new Button();
                button.Text = "Enter";
                button.Location = new Point(50, 35);
                button.Click += new EventHandler(button_Click);
                this.Controls.Add(button);
            }

            void button_Click(object sender, EventArgs e)
            {
                this.DialogResult = DialogResult.OK;
            }

            public string BoolName
            {
                get { return this.box.Text; }
            }

            private TextBox box;
            private Button button;
        }

        public CommandsAndBools(AdvData data)
        {
            InitializeComponent();
            mData = data;
            setControls();
        }

        private void setControls()
        {
            foreach(KeyValuePair<string,string> cmd in mData.Settings.Commands)
            {
                this.command.DataGridView.Rows.Add(cmd.Key, cmd.Value);
            }
            foreach (KeyValuePair<string, bool> cmd in mData.Settings.Booleans)
            {
                this.bools.Items.Add(cmd.Key, cmd.Value);
            }
            this.linktext.Text = mData.Settings.LinkText;
            this.givelink.Text = mData.Settings.GiveLink;
            this.walktext.Text = mData.Settings.WalkText;
        }

        private void getControls()
        {
            mData.Settings.Commands.Clear();
            foreach (DataGridViewRow cmd in this.command.DataGridView.Rows)
            {
                string cmdtext = (string)cmd.Cells[0].Value;
                string texttext = (string)cmd.Cells[1].Value;
                if (cmdtext != null && texttext != null)
                    mData.Settings.Commands.Add(cmdtext, texttext);
            }
            mData.Settings.Booleans.Clear();
            foreach (string bol in this.bools.Items)
            {
                mData.Settings.Booleans.Add(bol, this.bools.CheckedItems.Contains(bol));
            }
            mData.Settings.LinkText = this.linktext.Text;
            mData.Settings.GiveLink = this.givelink.Text;
            mData.Settings.WalkText = this.walktext.Text;
        }

        private AdvData mData;

        private void button1_Click(object sender, EventArgs e)
        {
            getControls();
            this.DialogResult = DialogResult.OK;
        }

        private void bool_delete_Click(object sender, EventArgs e)
        {
            int selection = this.bools.SelectedIndex;
            if (selection >= 0)
                this.bools.Items.RemoveAt(selection);
        }

        private void add_bool_Click(object sender, EventArgs e)
        {
            BoolAdder ba = new BoolAdder();
            ba.ShowDialog(this);
            if (ba.DialogResult == DialogResult.OK && ba.BoolName.Length > 0)
            {
                this.bools.Items.Add(ba.BoolName);
            }
        }
    }
}