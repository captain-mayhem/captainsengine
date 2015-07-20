using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ItemDlg : ChildForm
    {
        public ItemDlg(Item it, MainForm parent)
        {
            InitializeComponent();
            stateFrameImage1.Data = it;
            mItem = it;
            mParent = parent;
            updateItem();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            mParent.showScript(Script.Type.ITEM, mItem.Name.ToLower());
        }

        public Item Item
        {
            get { return mItem; }
        }

        public void updateItem()
        {
            this.Text = "Item (" + mItem.Name + ")";
        }

        Item mItem;
        MainForm mParent;
    }
}