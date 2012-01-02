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
            Sound,
            Music,
            Video,
            VideoSwf,
        }

        public class ArgDef
        {
            public ArgDef(string name, ArgType type) : this(name, type, false) { }
            public ArgDef(string name, ArgType type, bool optional)
            {
                Name = name;
                Type = type;
                Optional = optional;
            }
            public string Name;
            public ArgType Type;
            public string[] AdditionalValues;
            public bool Optional;
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
            args[0] = new ArgDef("Red Color Value (0-255)", ArgType.Integer);
            args[1] = new ArgDef("Green Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Blue Color Value (0-255)", ArgType.Integer);
            addFunction("offtextcolor", args);
            args = new ArgDef[3];
            args[0] = new ArgDef("Red Color Value (0-255)", ArgType.Integer);
            args[1] = new ArgDef("Green Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Blue Color Value (0-255)", ArgType.Integer);
            addFunction("infotextcolor", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Transparency (0-100)", ArgType.Integer);
            addFunction("settransparency", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Font (1-99)", ArgType.Integer);
            addFunction("setfont", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Screenchange", ArgType.String);
            string[] screenchanges = new string[] {"direct", "fadeblack", "rectangle", "circle",
                "shutters", "clock", "blend", "blendslow"};
            args[0].AdditionalValues = screenchanges;
            addFunction("setscreenchange", args);
            args = new ArgDef[4];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("X (walkmap)", ArgType.Integer);
            args[2] = new ArgDef("Y (walkmap)", ArgType.Integer);
            args[3] = new ArgDef("Free to Walk (true/false)", ArgType.Boolean);
            addFunction("setwalkmap", args);
            args = new ArgDef[6];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("X1 (walkmap)", ArgType.Integer);
            args[2] = new ArgDef("Y1 (walkmap)", ArgType.Integer);
            args[3] = new ArgDef("X2 (walkmap)", ArgType.Integer);
            args[4] = new ArgDef("Y2 (walkmap)", ArgType.Integer);
            args[5] = new ArgDef("Free to Walk (true/false)", ArgType.Boolean);
            addFunction("set_rect_walkmap", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Variable name", ArgType.Variable);
            addFunction("sqrt", args);

            //sound and movie commands
            args = new ArgDef[2];
            args[0] = new ArgDef("Soundeffect...", ArgType.Sound);
            args[1] = new ArgDef("Volume (0-100)", ArgType.Integer, true);
            addFunction("playsound", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Musicfile...", ArgType.Music);
            args[1] = new ArgDef("Pattern", ArgType.String, true);
            addFunction("playmusic", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Volume 0-100", ArgType.Integer);
            addFunction("musicvolume", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Volume 0-100", ArgType.Integer);
            addFunction("speechvolume", args);
            args = new ArgDef[0];
            addFunction("stopmusic", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("1=Slow - 15=Fast OR Time in ms", ArgType.Integer);
            addFunction("fadespeed", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("EAX Preset", ArgType.String);
            args[0].AdditionalValues = new string[] {"paddedcell", "livingroom", "cave",
                "carpetedhallway", "alley", "mountains", "parkinglot", "drugged", "concerthall",
                "room", "stoneroom", "arena", "hallway", "forest", "quarry", "sewerpipe", "dizzy",
                "none", "bathroom", "auditorium", "hangar", "stonecorridor", "city", "plain",
                "underwater", "psychotic", "off"};
            addFunction("seteax", args);
            args = new ArgDef[6];
            args[0] = new ArgDef("Video...", ArgType.Video);
            args[1] = new ArgDef("pause script (true/false)", ArgType.Boolean);
            args[2] = new ArgDef("X (pixel)", ArgType.Integer, true);
            args[3] = new ArgDef("Y (pixel)", ArgType.Integer, true);
            args[4] = new ArgDef("Width (pixel)", ArgType.Integer, true);
            args[5] = new ArgDef("Height (pixel)", ArgType.Integer, true);
            addFunction("playvideo", args);
            addFunction("playavi", args);
            args = new ArgDef[0];
            addFunction("stopvideo", args);
            args = new ArgDef[5];
            args[0] = new ArgDef("Video...", ArgType.VideoSwf);
            args[1] = new ArgDef("X (pixel)", ArgType.Integer, true);
            args[2] = new ArgDef("Y (pixel)", ArgType.Integer, true);
            args[3] = new ArgDef("Width (pixel)", ArgType.Integer, true);
            args[4] = new ArgDef("Height (pixel)", ArgType.Integer, true);
            addFunction("playswf", args);
            args = new ArgDef[0];
            addFunction("stopswf", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Seconds", ArgType.Integer);
            addFunction("moviewait", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Soundeffect...", ArgType.Sound);
            args[1] = new ArgDef("Volume (0-100)", ArgType.Integer, true);
            addFunction("loopsound", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("Soundeffect...", ArgType.Sound);
            addFunction("loopstop", args);

            //object commands
            args = new ArgDef[12];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("State 0-10", ArgType.Integer);
            for (int i = 0; i < 10; ++i)
            {
                args[i+2] = new ArgDef("State 0-10", ArgType.Integer, true);
            }
            addFunction("setobj", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("State 0-10", ArgType.Integer);
            addFunction("instobj", args);
            args = new ArgDef[5];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("X (pixel)", ArgType.Integer);
            args[2] = new ArgDef("Y (pixel)", ArgType.Integer);
            args[3] = new ArgDef("Speed 0=Immediately 9=Slow OR Time in ms", ArgType.Integer);
            args[4] = new ArgDef("Optional: wait", ArgType.String, true);
            args[4].AdditionalValues = new string[] { "wait" };
            addFunction("moveobj", args);
            args = new ArgDef[21];
            args[0] = new ArgDef("Groupname", ArgType.String);
            for (int i = 0; i < 10; ++i)
            {
                args[i + 1] = new ArgDef("Object", ArgType.Object, true);
            }
            addFunction("group", args);
            args = new ArgDef[5];
            args[0] = new ArgDef("Object", ArgType.Object);
            args[1] = new ArgDef("Red Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Green Color Value (0-255)", ArgType.Integer);
            args[3] = new ArgDef("Blue Color Value (0-255)", ArgType.Integer);
            args[4] = new ArgDef("fade (Optional)", ArgType.String, true);
            args[4].AdditionalValues = new string[] { "fade" };
            addFunction("setobjlight", args);

            //room commands
            args = new ArgDef[2];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("Screenchange (Optional)", ArgType.String, true);
            args[1].AdditionalValues = screenchanges;
            addFunction("loadroom", args);
            args = new ArgDef[0];
            addFunction("unloadroom", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("Time in ms (Optional)", ArgType.Integer, true);
            addFunction("subroom", args);
            args = new ArgDef[0];
            addFunction("return", args);
            args = new ArgDef[5];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("Red Color Value (0-255)", ArgType.Integer);
            args[2] = new ArgDef("Green Color Value (0-255)", ArgType.Integer);
            args[3] = new ArgDef("Blue Color Value (0-255)", ArgType.Integer);
            args[4] = new ArgDef("fade (Optional)", ArgType.String, true);
            args[4].AdditionalValues = new string[] { "fade" };
            addFunction("setlight", args);
            args = new ArgDef[5];
            args[0] = new ArgDef("Room", ArgType.Room);
            args[1] = new ArgDef("X (walkmap)", ArgType.Integer);
            args[2] = new ArgDef("Y (walkmap)", ArgType.Integer);
            args[3] = new ArgDef("Don't scroll (true/false)", ArgType.Boolean);
            args[4] = new ArgDef("Optional: up/down/left/right", ArgType.String, true);
            args[4].AdditionalValues = new string[] { "up", "down", "left", "right" };
            addFunction("setpos", args);
            args = new ArgDef[1];
            args[0] = new ArgDef("1-20 (4 = Default) (+100 don't follow)", ArgType.Integer);
            addFunction("scrollspeed", args);
            args = new ArgDef[2];
            args[0] = new ArgDef("Seconds", ArgType.Integer);
            args[1] = new ArgDef("Strength of Vibration", ArgType.Integer);
            addFunction("jiggle", args);

            //character commands
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
