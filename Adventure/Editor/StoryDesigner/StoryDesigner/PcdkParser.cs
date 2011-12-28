using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    class PcdkParser
    {
        public enum Error
        {
            LINE_NOT_RECOGINZED,
            CLOSING_BRACE_EXPECTED
        }

        public enum ArgType{
            Unknown,
            Function,
            Event,
            String,
            Number,
            Integer,
            Object,
            Character,
            Item,
        }

        public class ArgDef
        {
            public ArgDef(string name, ArgType type)
            {
                Name = name;
                Type = type;
            }
            public string Name;
            public ArgType Type;
        }

        public class Argument
        {
            public Argument(int startidx, int stopidx, string text, ArgType type)
            {
                this.Startidx = startidx;
                this.Stopidx = stopidx;
                this.Text = text;
                this.Type = type;
            }
            public Argument(int startidx, int stopidx, string text) : this(startidx, stopidx, text, ArgType.Unknown)
            {
            }
            public int Startidx;
            public int Stopidx;
            public string Text;
            public ArgType Type;
        }

        public delegate void commentCB(int charpos, int length);
        public delegate void functionCB(Argument funcname, Object[] args, int startidx);
        public delegate void parseError(int charpos, int length, string text, Error error);

        public event commentCB Comment;
        public event functionCB Function;
        public event parseError ParseError;

        //syntax definition
        public void initSyntax()
        {
            //events
            ArgDef[] args = new ArgDef[1];
            args[0] = new ArgDef("Event", ArgType.Event);
            addFunction("on", args, true);
            //conditionals
            args = new ArgDef[2];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("State 0-10", ArgType.Integer);
            addConditional("obj", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("X (pixel)", ArgType.Integer);
            addConditional("xobj", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("Y (pixel)", ArgType.Integer);
            addConditional("yobj", args);
            addConditional("num", null);
            addConditional("hasitem", null);
            addConditional("room", null);
            addConditional("keydown", null);
        }

        public void addConditional(string name, ArgDef[] arguments)
        {
            addFunction("if_" + name, arguments, true);
            addFunction("ifnot_" + name, arguments, true);
        }

        public void addFunction(string name, ArgDef[] arguments)
        {
            addFunction(name, arguments, false);
        }

        public void addFunction(string name, ArgDef[] arguments, bool isKeyword)
        {
            mFunctions[name] = arguments;
            if (isKeyword)
                mKeywords.Add(name);
        }

        //parser
        public void parseText(RichTextBox scripttext)
        {
            for (int i = 0; i < scripttext.Lines.Length; ++i)
            {
                parseLine(i, scripttext);
            }
        }

        public void parseLine(int line, RichTextBox scripttext)
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
            int idx = readUntil('(', text, 0, out result);
            if (idx != -1)
            {
                if (text.Length > idx+1 && text[idx + 1] == '*')
                {
                    //Comment TODO make it right: multiline, part of a line
                    if (Comment != null)
                        Comment(startindex, text.Length);
                }
                else
                {
                    Argument func = new Argument(0, idx, result, ArgType.Function);
                    int lastidx = idx + 1;
                    //arguments
                    System.Collections.ArrayList arguments = new System.Collections.ArrayList();
                    do
                    {
                        int newidx = readUntil(';', text, lastidx, out result);
                        if (newidx != -1)
                        {
                            Argument arg = new Argument(lastidx, newidx, result);
                            addArgument(func, arg, arguments);
                            lastidx = newidx + 1;
                        }
                        else
                        {
                            newidx = readUntil(')', text, lastidx, out result);
                            if (newidx == -1)
                            {
                                OnError(startindex+lastidx, result.Length, result, Error.CLOSING_BRACE_EXPECTED);
                            }
                            Argument arg = new Argument(lastidx, newidx, result);
                            addArgument(func, arg, arguments);
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
            OnError(startindex, text.Length, text, Error.LINE_NOT_RECOGINZED);
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
            return mKeywords.Contains(funcname.Text);
        }

        void OnError(int charpos, int length, string text, Error error)
        {
            if (ParseError != null)
                ParseError(charpos, length, text, error);
        }

        bool addArgument(Argument func, Argument arg, System.Collections.ArrayList arguments)
        {
            if (arg.Stopidx == -1)
                arg.Stopidx = arg.Startidx;
            arguments.Add(arg);
            if (!mFunctions.ContainsKey(func.Text))
                return false;
            ArgDef[] args = mFunctions[func.Text];
            if (args == null || args.Length <= arguments.Count)
                return false;
            ArgDef curr = args[arguments.Count];
            arg.Type = curr.Type;
            return true;
        }

        System.Collections.Specialized.StringCollection mKeywords = new System.Collections.Specialized.StringCollection();
        System.Collections.Generic.Dictionary<string, ArgDef[]> mFunctions = new Dictionary<string,ArgDef[]>();
    }
}
