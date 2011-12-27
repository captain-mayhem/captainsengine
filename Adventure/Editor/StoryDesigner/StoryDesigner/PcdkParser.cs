using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    class PcdkParser
    {
        public delegate void commentCB(int charpos, int length);
        public delegate void functionCB(Argument funcname, Object[] args, int startidx);

        public event commentCB Comment;
        public event functionCB Function;

        //syntax definition
        public void initSyntax()
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
        public void parseText(RichTextBox scripttext)
        {
            for (int i = 0; i < scripttext.Lines.Length; ++i)
            {
                parseLine(i, scripttext);
            }
        }

        void parseLine(int line, RichTextBox scripttext)
        {
            string linetext = scripttext.Lines[line];
            if (linetext.Trim().Length == 0)
                return;
            int charindex = scripttext.GetFirstCharIndexFromLine(line);
            parseFunction(charindex, linetext);
        }

        public class Argument
        {
            public Argument(int startidx, int stopidx, string text)
            {
                this.startidx = startidx;
                this.stopidx = stopidx;
                this.text = text;
            }
            public int startidx;
            public int stopidx;
            public string text;
        }

        void parseFunction(int startindex, string text)
        {
            string result;
            int idx = readUntil('(', text, 0, out result);
            if (idx != -1)
            {
                if (text[idx + 1] == '*')
                {
                    //Comment TODO make it right: multiline, part of a line
                    if (Comment != null)
                        Comment(startindex, text.Length);
                }
                else
                {
                    Argument func = new Argument(0, idx, result);
                    int lastidx = idx + 1;
                    //arguments
                    System.Collections.ArrayList arguments = new System.Collections.ArrayList();
                    do
                    {
                        int newidx = readUntil(';', text, lastidx, out result);
                        if (newidx != -1)
                        {
                            Argument arg = new Argument(lastidx, newidx, result);
                            arguments.Add(arg);
                            lastidx = newidx + 1;
                        }
                        else
                        {
                            newidx = readUntil(')', text, lastidx, out result);
                            if (newidx == -1)
                            {
                                throw new UnexpectedValueException("')' expected");
                            }
                            Argument arg = new Argument(lastidx, newidx, result);
                            arguments.Add(arg);
                            break;
                        }
                    }
                    while (idx != -1);
                    if (Function != null)
                        Function(func, arguments.ToArray(), startindex);
                }
                return;
            }
            idx = readUntil('{', text, 0, out result);
            if (idx != -1)
            {
                //block begin
                return;
            }
            idx = readUntil('}', text, 0, out result);
            if (idx != -1)
            {
                //block end
                return;
            }
            throw new UnexpectedValueException(text);
        }

        int readUntil(char stopchar, string text, int fromIdx, out string result)
        {
            StringBuilder builder = new StringBuilder();
            for (int i = fromIdx; i < text.Length; ++i)
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

        public bool IsKeyword(Argument funcname){
            return mKeywords.Contains(funcname.text);
        }

        System.Collections.Specialized.StringCollection mKeywords = new System.Collections.Specialized.StringCollection();
    }
}
