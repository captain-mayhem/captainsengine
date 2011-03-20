using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ItemDlg : Form
    {
        public ItemDlg(Item it)
        {
            InitializeComponent();
            stateFrameImage1.Data = it;
            this.Text = "Item ("+it.Name+")";
            mItem = it;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            MainForm form = (MainForm)this.Owner;
            form.showScript(Script.Type.ITEM, mItem.Name.ToLower());
        }

        Item mItem;
    }
}