using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Collections;

namespace StoryDesigner
{
    public partial class ScriptDlg : Form
    {
        public ScriptDlg(Script scr, AdvData data)
        {
            mData = data;
            InitializeComponent();
            linenumberbox.Paint += new PaintEventHandler(linenumberbox_Paint);
            scripttext.VScroll += new EventHandler(scripttext_VScroll);
            scripttext.TextChanged += new EventHandler(scripttext_TextChanged);
            scripttext.MouseDown += new MouseEventHandler(scripttext_MouseDown);
            scripttext.KeyUp += new KeyEventHandler(scripttext_KeyUp);
            //scripttext
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
            //parsing stuff
            mParser = new PcdkParser();
            mParser.initSyntax();
            mParser.Comment += new PcdkParser.commentCB(colorComment);
            mParser.Function += new PcdkParser.functionCB(colorFunction);
            mParser.ParseError += new PcdkParser.parseError(mParser_ParseError);
            mKeywordFont = new Font(scripttext.Font, FontStyle.Bold);
            parseScript();
        }

        void scripttext_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Up || e.KeyCode == Keys.Down || e.KeyCode == Keys.Left || e.KeyCode == Keys.Right)
            {
                int line = scripttext.GetLineFromCharIndex(scripttext.SelectionStart);
                parseLine(line);
            }
            else if (e.KeyCode == Keys.Enter)
            {
                int line = scripttext.GetLineFromCharIndex(scripttext.SelectionStart);
                string prevline = scripttext.Lines[line - 1];
                int oldidx = scripttext.SelectionStart;
                int i;
                for (i = 0; i < prevline.Length; ++i)
                {
                    if (!Char.IsWhiteSpace(prevline[i]))
                        break;
                }
                scripttext.Select(scripttext.GetFirstCharIndexFromLine(line-1), i);
                scripttext.Copy();
                scripttext.SelectionStart = oldidx;
                scripttext.SelectionLength = 0;
                scripttext.Paste();
            }
        }

        void scripttext_MouseDown(object sender, MouseEventArgs e)
        {
            int charidx = scripttext.GetCharIndexFromPosition(new Point(e.X, e.Y));
            int line = scripttext.GetLineFromCharIndex(charidx);
            parseLine(line);
        }

        void scripttext_TextChanged(object sender, EventArgs e)
        {
            int charidx = scripttext.GetFirstCharIndexOfCurrentLine();
            int line = scripttext.GetLineFromCharIndex(charidx);
            parseLine(line);
        }

        void scripttext_VScroll(object sender, EventArgs e)
        {
            linenumberbox.Invalidate();
        }

        void linenumberbox_Paint(object sender, PaintEventArgs e)
        {
            //as c# 2.0 has no way to determine scroll offset without p/invoke, do the following workaround
            int y = 1;
            int idx = 0;
            int idx2 = scripttext.GetCharIndexFromPosition(new Point(1, 1));
            for (int i = 2; i < scripttext.Font.Height; ++i){
                idx = scripttext.GetCharIndexFromPosition(new Point(2, i));
                if (idx != idx2){
                    y = i;
                    break;
                }
            }
            int line = scripttext.GetLineFromCharIndex(idx);
            y -= scripttext.Font.Height;
            //workaround end
            while (y < linenumberbox.Size.Height)
            {
                e.Graphics.DrawString(line.ToString(), scripttext.Font, Brushes.Black, 0, y);
                ++line;
                y += scripttext.Font.Height;
            }
        }

        void parseScript()
        {
            if (mLayoutPerformed)
                return;
            mLayoutPerformed = true;
            scripttext.SuspendLayout();
            int oldoffset = scripttext.SelectionStart;
            mCursorPos = oldoffset;
            int oldlength = scripttext.SelectionLength;
            mParser.parseText(scripttext);
            scripttext.SelectionStart = oldoffset;
            scripttext.SelectionLength = oldlength;
            scripttext.SelectionFont = scripttext.Font;
            scripttext.SelectionColor = Color.Black;
            scripttext.ResumeLayout();
            mLayoutPerformed = false;
        }

        void parseLine(int line)
        {
            if (mLayoutPerformed)
                return;
            mLayoutPerformed = true;
            scripttext.SuspendLayout();
            int oldoffset = scripttext.SelectionStart;
            mCursorPos = oldoffset;
            int oldlength = scripttext.SelectionLength;
            mParser.parseLine(line, scripttext);
            scripttext.SelectionStart = oldoffset;
            scripttext.SelectionLength = oldlength;
            scripttext.SelectionFont = scripttext.Font;
            scripttext.SelectionColor = Color.Black;
            scripttext.ResumeLayout();
            mLayoutPerformed = false;
        }

        void colorComment(int charpos, int length)
        {
            scripttext.SelectionStart = charpos;
            scripttext.SelectionLength = length;
            scripttext.SelectionColor = Color.Green;
        }

        void colorFunction(PcdkParser.Argument funcname, Object[] args, int startidx)
        {
            scripttext.SelectionStart = startidx+funcname.Startidx;
            scripttext.SelectionLength = funcname.Stopidx-funcname.Startidx;
            scripttext.SelectionFont = mKeywordFont;
            if (mParser.IsKeyword(funcname))
                scripttext.SelectionColor = Color.Blue;
            else
                scripttext.SelectionColor = Color.Black;
            if (scripttext.SelectionStart <= mCursorPos && scripttext.SelectionStart + scripttext.SelectionLength >= mCursorPos)
                fillMatchList(funcname);
            foreach (PcdkParser.Argument arg in args){
                scripttext.SelectionStart = startidx + arg.Startidx;
                scripttext.SelectionLength = arg.Stopidx - arg.Startidx;
                scripttext.SelectionFont = scripttext.Font;
                scripttext.SelectionColor = Color.Brown;
                if (scripttext.SelectionStart <= mCursorPos && scripttext.SelectionStart + scripttext.SelectionLength >= mCursorPos)
                    fillMatchList(arg);
            }
        }

        void mParser_ParseError(int charpos, int length, string text, PcdkParser.Error error)
        {
            scripttext.SelectionStart = charpos;
            scripttext.SelectionLength = length;
            scripttext.SelectionColor = Color.Black;
            if (error == PcdkParser.Error.LINE_NOT_RECOGINZED)
            {
                PcdkParser.Argument arg = new PcdkParser.Argument(charpos, charpos + length, text.Trim(),
                new PcdkParser.ArgDef("Command", PcdkParser.ArgType.Function));
                if (mParser.IsKeyword(arg))
                    scripttext.SelectionColor = Color.Blue;
                scripttext.SelectionFont = mKeywordFont;
                fillMatchList(arg);
            }
        }

        void fillMatchList(PcdkParser.Argument arg)
        {
            info.Text = arg.Definition.Name;
            string match = scripttext.Text.Substring(scripttext.SelectionStart, mCursorPos - scripttext.SelectionStart).Trim();
            matches.Items.Clear();
            if (arg.Definition.AdditionalValues != null)
            {
                foreach (string val in arg.Definition.AdditionalValues)
                {
                    addMatch(val, match);
                }
            }
            switch (arg.Definition.Type)
            {
                case PcdkParser.ArgType.Boolean:
                    addMatch("true", match);
                    addMatch("false", match);
                    break;
                case PcdkParser.ArgType.Character:
                    addMatch("self", match);
                    foreach (KeyValuePair<string, ArrayList> pair in mData.CharacterInstances)
                    {
                        foreach (CharacterInstance chr in pair.Value){
                            addMatch(chr.Name, match);
                        }
                    }
                    break;
                case PcdkParser.ArgType.Command:
                    foreach (KeyValuePair<string, string> pair in mData.Settings.Commands)
                    {
                        addMatch(pair.Key, match);
                    }
                    break;
                case PcdkParser.ArgType.Event:
                    {
                        foreach (KeyValuePair<string, string> pair in mData.Settings.Commands)
                        {
                            addMatch(pair.Key, match);
                            addMatch("cant" + pair.Key, match);
                        }
                        break;
                    }
                case PcdkParser.ArgType.Function:
                    foreach (KeyValuePair<string, PcdkParser.ArgDef[]> func in mParser.Functions)
                    {
                        addMatch(func.Key, match);
                    }
                    break;
            }
        }

        void addMatch(string str, string match)
        {
            if (str.StartsWith(match, StringComparison.OrdinalIgnoreCase))
                matches.Items.Add(str);
        }

        AdvData mData;
        PcdkParser mParser;
        Font mKeywordFont;
        bool mLayoutPerformed;
        int mCursorPos;
    }
}