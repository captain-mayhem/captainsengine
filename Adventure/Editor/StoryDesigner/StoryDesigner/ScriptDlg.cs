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
            initSyntax();
            mKeywordFont = new Font(scripttext.Font, FontStyle.Bold);
            parseText();
        }

        //syntax definition
        void initSyntax()
        {
            addConditional("num");
            addConditional("hasitem");
            addConditional("obj");
            addConditional("room");
            addConditional("keydown");
        }

        public void addConditional(string name)
        {
            mKeywords.Add("if_" + name);
            mKeywords.Add("ifnot_" + name);
        }

        //parser
        void parseText()
        {
            for (int i = 0; i < scripttext.Lines.Length; ++i)
            {
                parseLine(i);
            }
        }

        void parseLine(int line)
        {
            string linetext = scripttext.Lines[line];
            if (linetext.Trim().Length == 0)
                return;
            int charindex = scripttext.GetFirstCharIndexFromLine(line);
            parseFunction(charindex, linetext);
        }

        void parseFunction(int startindex, string text)
        {
            string result;
            int idx = readUntil('(', text, out result);
            if (idx != -1)
            {
                if (text[idx + 1] == '*')
                {
                    //Comment TODO make it right: multiline, part of a line
                    comment(startindex, text.Length);
                }
                else
                    functionName(result, startindex, idx);
                return;
            }
            idx = readUntil('{', text, out result);
            if (idx != -1)
            {
                //block begin
                return;
            }
            idx = readUntil('}', text, out result);
            if (idx != -1)
            {
                //block end
                return;
            }
            throw new UnexpectedValueException(text);
        }

        void comment(int charpos, int length)
        {
            scripttext.SelectionStart = charpos;
            scripttext.SelectionLength = length;
            scripttext.SelectionColor = Color.Green;
        }

        void functionName(string name, int charpos, int length)
        {
            scripttext.SelectionStart = charpos;
            scripttext.SelectionLength = length;
            //scripttext.SelectionFont = new Font(this.Font, FontStyle.Bold);
            //scripttext.SelectionColor = Color.Brown;
            scripttext.SelectionFont = mKeywordFont;
            if (mKeywords.Contains(name))
                scripttext.SelectionColor = Color.Blue;
            //else
            //    scripttext.SelectionFont = mKeywordFont;
        }

        int readUntil(char stopchar, string text, out string result)
        {
            StringBuilder builder = new StringBuilder();
            for (int i = 0; i < text.Length; ++i)
            {
                if (text[i] == stopchar)
                {
                    result = builder.ToString();
                    return i;
                }
                if (!Char.IsWhiteSpace(text[i]))
                    builder.Append(text[i]);
            }
            result = builder.ToString();
            return -1;
        }

        System.Collections.Specialized.StringCollection mKeywords = new System.Collections.Specialized.StringCollection();
        Font mKeywordFont;
    }
}