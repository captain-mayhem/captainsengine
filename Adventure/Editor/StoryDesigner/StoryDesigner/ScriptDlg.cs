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
                case Script.Type.CHARACTER:
                    text = "Character";
                    break;
                case Script.Type.ROOM:
                    text = "Room";
                    break;
                case Script.Type.WALKMAP:
                    text = "Walkmap";
                    break;
            }
            string name = scr.Name;
            if (name.Contains(";")){
                name = name.Substring(0, name.IndexOf(';'));
            }
            if (scr.ScriptType == Script.Type.WALKMAP)
            {
                Text = String.Format("Script ({0} [{1}-{2}] / {3})", name.Substring(7), Convert.ToInt32(name.Substring(1,3)), Convert.ToInt32(name.Substring(4, 3)), text);
            }
            else
                this.Text = "Script (" + name + " / " + text + ")";
            scripttext.Text = scr.Text;
        }
    }
}