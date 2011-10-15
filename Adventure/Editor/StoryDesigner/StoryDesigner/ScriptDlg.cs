using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class ScriptDlg : Form
    {
        public ScriptDlg(Script scr)
        {
            InitializeComponent();
            string text = scr.ScriptType.ToString();
            switch (scr.ScriptType)
            {
                case Script.Type.CUTSCENE:
                    text = "Script";
                    break;
                case Script.Type.ITEM:
                    text = "Item";
                    break;
            }
            string name = scr.Name;
            if (name.Contains(";")){
                name = name.Substring(0, name.IndexOf(';'));
            }
            this.Text = "Script (" + name + " / " + text + ")";
            scripttext.Text = scr.Text;
        }
    }
}