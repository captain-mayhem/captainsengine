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
            Command,
            Event,
            String,
            Number,
            Integer,
            Boolean,
            Variable,
            PresetVariable,
            Object,
            Character,
            Item,
            Script,
            Room,
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
            public string[] AdditionalValues;
        }

        public class Argument
        {
            public Argument(int startidx, int stopidx, string text, ArgDef def)
            {
                this.Startidx = startidx;
                this.Stopidx = stopidx;
                this.Text = text;
                this.Definition = def;
            }
            public Argument(int startidx, int stopidx, string text) : this(startidx, stopidx, text, new ArgDef("", ArgType.Unknown))
            {
            }
            public int Startidx;
            public int Stopidx;
            public string Text;
            public ArgDef Definition;
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
            args[0].AdditionalValues = new string[] { "mouse", "mouseout", "click", "release", 
            "rightclick", "doubleclick", "enter", "exit", "loop1", "loop2", "link", "givelink",
            "cantall"};
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
            args = new ArgDef[1];
            args[0] = new ArgDef("Character", ArgType.Character);
            addConditional("focus", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Character", ArgType.Character);
            args[1] = new ArgDef("Room", ArgType.Room);
            addConditional("charin", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Character", ArgType.Character);
            addConditional("ischar", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Character", ArgType.Character);
            args[1] = new ArgDef("Item", ArgType.Item);
            addConditional("hasitem", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Variable Name", ArgType.Variable);
            args[1] = new ArgDef("Number Value", ArgType.Number);
            addConditional("num", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("String Name", ArgType.Variable);
            args[1] = new ArgDef("Textline", ArgType.String);
            addConditional("string", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Bool Name", ArgType.Variable);
            args[1] = new ArgDef("true/false", ArgType.Boolean);
            addConditional("bool", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Key", ArgType.String);
            addConditional("keydown", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Key", ArgType.String);
            addConditional("keypressed", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("up OR down", ArgType.String);
            addConditional("mousewheel", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Gamecommand", ArgType.Command);
            addConditional("command", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Linkname", ArgType.String);
            addConditional("link", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Item", ArgType.Item);
            addConditional("givelink", args);
            args = new ArgDef[4];
            args[0] = new ArgDef("Script", ArgType.Script);
            args[1] = new ArgDef("Level 1-9", ArgType.Integer);
            args[2] = new ArgDef("Row 1-9", ArgType.Integer);
            args[3] = new ArgDef("Activate (true/false)", ArgType.Boolean);
            addConditional("textscene", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Room", ArgType.Room);
            addConditional("room", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Item", ArgType.Item);
            args[1] = new ArgDef("State 0-10", ArgType.Integer);
            addConditional("item", args);
            //set commands
            args = new ArgDef[1];
            args[0] = new ArgDef("Gamecommand", ArgType.Command);
            addFunction("command", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Mouseicons (1-8)", ArgType.Integer);
            addFunction("instmouse", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Character", ArgType.Character);
            args[0].AdditionalValues = new string[] { "none", "last" };
            addFunction("setfocus", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Bool Name", ArgType.PresetVariable);
            args[1] = new ArgDef("true / false", ArgType.Boolean);
            addFunction("setbool", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Variable Name", ArgType.Variable);
            args[1] = new ArgDef("Number Value", ArgType.Number);
            addFunction("setnum", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("String Name", ArgType.Variable);
            args[1] = new ArgDef("Textline", ArgType.String);
            addFunction("setstring", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Variable Name", ArgType.Variable);
            args[1] = new ArgDef("Range", ArgType.Integer);
            addFunction("randomnum", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Variable Name", ArgType.Variable);
            addFunction("savenum", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Variable Name", ArgType.Variable);
            addFunction("loadnum", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("String Name", ArgType.Variable);
            addFunction("savestring", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("String Name", ArgType.Variable);
            addFunction("loadstring", args);
            args = new ArgDef[3];
            args[0] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            args[1] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            addFunction("offtextcolor", args);
            args = new ArgDef[3];
            args[0] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            args[1] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Color Value (0-255)", ArgType.Integer);
            addFunction("infotextcolor", args);
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
                    Argument func = new Argument(0, idx, result, new ArgDef("Command", ArgType.Function));
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
                arg.Stopidx = arg.Startidx+arg.Text.Length;
            arguments.Add(arg);
            if (!mFunctions.ContainsKey(func.Text))
                return false;
            ArgDef[] args = mFunctions[func.Text];
            if (args == null || args.Length <= arguments.Count-1)
                return false;
            ArgDef curr = args[arguments.Count-1];
            arg.Definition = curr;
            return true;
        }

        public Dictionary<string, ArgDef[]> Functions
        {
            get {return mFunctions;}
        }

        System.Collections.Specialized.StringCollection mKeywords = new System.Collections.Specialized.StringCollection();
        System.Collections.Generic.Dictionary<string, ArgDef[]> mFunctions = new Dictionary<string,ArgDef[]>();
    }
}
