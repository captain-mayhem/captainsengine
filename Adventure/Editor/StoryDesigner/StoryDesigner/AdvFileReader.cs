using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Drawing;
using ICSharpCode.SharpZipLib.Zip;

namespace StoryDesigner
{
    public class AdvFileReader
    {
        protected readonly float FPS_MAX = 50.0f;
        protected readonly int STATES_MAX = 10;
        protected readonly int CHAR_STATES_MAX = 36;
        protected readonly int FRAMES_MAX = 25;
        protected readonly int FRAMES2_MAX = 30;
        protected readonly int PARTS_MAX = 2;
        protected readonly int FXSHAPES_MAX = 3;

        public AdvFileReader(string filename, TreeView mediapool, TreeView gamepool)
        {
            mAdv = new AdvData(this);
            mMediaPool = mediapool;
            mGamePool = gamepool;

            if (System.IO.Path.GetExtension(filename) == ".dat")
            {
                mAdv.Settings.Projectname = "game";
                string dir = System.IO.Path.GetDirectoryName(filename);
                FileInfo[] files = System.IO.Directory.GetParent(dir).GetFiles("*.exe");
                foreach (FileInfo file in files)
                {
                    if (file.Name != "Setup.exe")
                    {
                        mAdv.Settings.Projectname = System.IO.Path.GetFileNameWithoutExtension(file.Name);
                    }
                }
            }

            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (entry.Name == "game.001")
                {
                    readSettings(zis);
                }
                else if (entry.Name == "game.002")
                {
                    readObjects(zis);
                }
                else if (entry.Name == "game.003")
                {
                    readScripts(zis);
                }
                else if (entry.Name == "game.005")
                {
                    readPassword(zis);
                }
            }
            zis.Close();
            mPath = System.IO.Path.GetDirectoryName(filename);
        }

        protected bool readSettings(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding("iso-8859-1"));
            string str = rdr.ReadLine();
            int ver_major = Int32.Parse(str.Substring(0, 1));
            int ver_minor = Int32.Parse(str.Substring(2, 1));
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
            mAdv.Settings.Directory = rdr.ReadLine();
            str = rdr.ReadLine();
            if (str == "Resolution X : 640")
            {
                mAdv.Settings.Resolution = new Vec2i(640, 480);
            }
            else
            {
                throw new UnexpectedValueException("Resolution is invalid");
            }
            string font = rdr.ReadLine();
            while (font.Substring(0, 11) != "GameFont : ")
            {
                string[] fontparts = font.Split(';');
                FontInfo info = new FontInfo();
                info.name = fontparts[0];
                info.bold = Convert.ToInt32(fontparts[1]) == -1;
                info.italic = Convert.ToInt32(fontparts[2]) == -1;
                info.size = Convert.ToInt32(fontparts[3]);
                info.outline = Convert.ToInt32(fontparts[4]);
                info.charset = Convert.ToInt32(fontparts[5]);
                info.shadow = Convert.ToInt32(fontparts[6]);
                info.fill = Convert.ToInt32(fontparts[7]);
                info.fading = Convert.ToInt32(fontparts[8]);
                mAdv.Settings.Fonts.Add(info);
                font = rdr.ReadLine();
            } 
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.GameIcon = str.Substring(11);
            mAdv.Settings.LoadingImage = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.TsUseBgImage = str == "-1";
            mAdv.Settings.TsBackground = rdr.ReadLine();
            str = rdr.ReadLine();
            if (str.Substring(0, 14) != "Startskript : ")
            {
                throw new UnexpectedValueException("expected Startskript : ");
            }
            mAdv.Settings.Startscript = str.Substring(14);
            mAdv.Settings.Mainscript = rdr.ReadLine();
            mAdv.Settings.AnywhereRoom = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.ScreenChange = (ScreenChange)Convert.ToInt32(str.Substring(15));
            str = rdr.ReadLine().Substring(9);
            mAdv.Settings.TextOnOff = str[0] == '1';
            mAdv.Settings.DrawDraggedItemIcons = str[1] == '1';
            mAdv.Settings.ActionText = str[2] == '1';
            mAdv.Settings.ShowTaskbar = str[3] == '1';
            mAdv.Settings.NotAntialiased = str[4] == '1';
            mAdv.Settings.TaskbarFromTop = str[5] == '1';
            mAdv.Settings.GroupItems = str[6] == '1';
            mAdv.Settings.ProtectGameFile = str[7] == '1';
            str = rdr.ReadLine();
            mAdv.Settings.ActionTextHeight = Convert.ToInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.CustomMenu = str == "-1";
            mAdv.Settings.CustomMenuRoom = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.TaskHeight = Convert.ToInt32(str.Substring(13));
            str = rdr.ReadLine();
            mAdv.Settings.TaskRoom = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.TaskPopup = Convert.ToInt32(str.Substring(12));
            str = rdr.ReadLine();
            string addBools = str.Substring(8, 2);
            mAdv.Settings.SilentDelete = addBools[0] == '1';
            mAdv.Settings.InfoLine = addBools[1] == '1';
            string taskshow = str.Substring(11, 4);
            if (taskshow == "hide")
                mAdv.Settings.TaskHideCompletely = true;
            else
                mAdv.Settings.TaskHideCompletely = false;
            mAdv.Settings.RightClick = Convert.ToInt32(str.Substring(16));
            str = rdr.ReadLine();
            mAdv.Settings.UseMouseWheel = (str == "-1");
            str = rdr.ReadLine();
            mAdv.Settings.MenuFading = Convert.ToInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TextSceneFading = Convert.ToInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.AnywhereTransparency = Convert.ToInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TargaColor = Convert.ToUInt32(str.Substring(13));
            str = rdr.ReadLine();
            mAdv.Settings.BorderColor = Convert.ToUInt32(str.Substring(14));
            str = rdr.ReadLine();
            mAdv.Settings.BackgroundColor = Convert.ToUInt32(str.Substring(18));
            str = rdr.ReadLine();
            mAdv.Settings.TextColor = Convert.ToUInt32(str.Substring(12));
            str = rdr.ReadLine().Substring(15);
            string[] colors = str.Split(';');
            mAdv.Settings.OffspeechColor = Convert.ToUInt32(colors[0]);
            mAdv.Settings.InfotextColor = Convert.ToUInt32(colors[1]);
            str = rdr.ReadLine();
            mAdv.Settings.TsStyle = Convert.ToInt32(str) - 1;
            str = rdr.ReadLine();
            mAdv.Settings.TsBorderStyle = Convert.ToInt32(str) - 1;
            str = rdr.ReadLine();
            mAdv.Settings.TsBorderColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsAreaColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsSelectionColor = Convert.ToUInt32(str);
            str = rdr.ReadLine();
            mAdv.Settings.TsTextColor = Convert.ToUInt32(str);
            if (ver_major >= 3 || (ver_major >= 2 && ver_minor >= 7))
            {
                str = rdr.ReadLine();
                mAdv.Settings.TsUseSymbols = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.MuteMusicWhenSpeech = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.CoinActivated = str == "-1";
                str = rdr.ReadLine();
                mAdv.Settings.CoinAutoPopup = str == "-1";
                mAdv.Settings.CoinRoom = rdr.ReadLine();
                str = rdr.ReadLine();
                mAdv.Settings.CoinFading = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.CoinCenter.x = Convert.ToInt32(str);
                str = rdr.ReadLine();
                mAdv.Settings.CoinCenter.y = Convert.ToInt32(str);
            }
            str = rdr.ReadLine();
            mAdv.Settings.LinkText = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.GiveLink = str.Substring(11);
            str = rdr.ReadLine();
            mAdv.Settings.WalkText = str.Substring(11);
            str = rdr.ReadLine();
            if (str == "Booleans :")
            {
                mAdv.Settings.Booleans = new Dictionary<string, bool>();
                str = rdr.ReadLine();
                while (str != "Commands :")
                {
                    string name;
                    string val = str.Substring(str.Length - 1);
                    if (val == "1")
                        name = str.Substring(0, str.Length - 2);
                    else
                        name = str.Substring(0, str.Length - 1);
                    mAdv.Settings.Booleans.Add(name, val == "1");
                    str = rdr.ReadLine();
                }
            }
            if (str == "Commands :")
            {
                mAdv.Settings.Commands = new Dictionary<string, string>();
                str = rdr.ReadLine();
                while (str != "Mediapool :")
                {
                    string pretty_name = rdr.ReadLine();
                    mAdv.Settings.Commands.Add(str, pretty_name);
                    str = rdr.ReadLine();
                }
            }
            str = rdr.ReadLine();
            TreeNode root = new TreeNode("Mediapool");
            TreeNode node = root;
            mMediaPool.Nodes.Add(node);
            while (str != "Gamepool :")
            {
                node = insertTreeElement(mMediaPool.Nodes, str, node, node.Level-1);
                str = rdr.ReadLine();
            }
            TreeNode[] arr = new TreeNode[root.Nodes.Count];
            root.Nodes.CopyTo(arr, 0);
            mMediaPool.Nodes.Clear();
            mMediaPool.Nodes.AddRange(arr);

            str = rdr.ReadLine();
            root = new TreeNode("Gamepool");
            node = root;
            mGamePool.Nodes.Add(node);
            while (str != "Images :")
            {
                node = insertTreeElement(mGamePool.Nodes, str, node, node.Level - 1);
                str = rdr.ReadLine();
            }
            arr = new TreeNode[root.Nodes.Count];
            root.Nodes.CopyTo(arr, 0);
            mGamePool.Nodes.Clear();
            mGamePool.Nodes.AddRange(arr);

            if (str == "Images :")
            {
                str = rdr.ReadLine();
                while (str != "Sounds :")
                {
                    string filename = rdr.ReadLine();
                    mAdv.Images.Add(str.ToLower(), filename);
                    str = rdr.ReadLine();
                }
            }
            return true;
        }

        protected bool readObjects(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding("iso-8859-1"));
            string str = rdr.ReadLine();
            int ver_major = Int32.Parse(str.Substring(0, 1));
            int ver_minor = Int32.Parse(str.Substring(2, 1));
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
            for (int state = 0; state < STATES_MAX; ++state)
            {
                CursorState cs;
                cs.frames = new System.Collections.ArrayList();
                for (int frames = 0; frames < FRAMES_MAX; ++frames)
                {
                    str = rdr.ReadLine();
                    if (str.Length > 0)
                        cs.frames.Add(str);
                }
                str = rdr.ReadLine();
                cs.command = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.fpsDivider = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.x = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.y = Convert.ToInt32(str);
                mAdv.Cursor.Add(cs);
            }
            while (!rdr.EndOfStream)
            {
                str = rdr.ReadLine();
                if (rdr.EndOfStream)
                    return true;
                if (str.Length < 2)
                    continue;
                string rest = str.Substring(2);
                //TODO check
                string[] typename = rest.Split(' ');
                //ITEM
                if (typename[0] == "Item")
                {
                    Item it = new Item(mAdv);
                    it.Name = typename[1];
                    int numStates = STATES_MAX;
                    int delim = 1;
                    if (ver_major == 2 || (ver_major == 3 && ver_minor == 0))
                    {
                        numStates = 1;
                        delim = 1;
                    }
                    for (int state = 0; state < numStates; ++state)
                    {
                        ItemState ist;
                        ist.frames = new System.Collections.ArrayList();
                        for (int frames = 0; frames < FRAMES_MAX; ++frames)
                        {
                            str = rdr.ReadLine();
                            if (str != ";" && str.Length > 0)
                            {
                                ist.frames.Add(str.Substring(0, str.Length + 1 - delim));
                            }
                        }
                        str = rdr.ReadLine();
                        ist.fpsDivider = Convert.ToInt32(str);
                        it.Add(ist);
                    }
                    mAdv.addItem(it);
                }
                //OBJECT
                else if (typename[0] == "Object")
                {
                    AdvObject obj = new AdvObject(mAdv);
                    obj.Name = typename[1];
                    str = rdr.ReadLine();
                    int x = Convert.ToInt32(str);
                    str = rdr.ReadLine();
                    int y = Convert.ToInt32(str);
                    obj.setSize(0, new Vec2i(x, y));
                    str = rdr.ReadLine();
                    x = Convert.ToInt32(str);
                    obj.Lighten = x != 0;
                    for (int state = 0; state < STATES_MAX; ++state)
                    {
                        ObjectState os = new ObjectState();
                        os.fpsDivider = readExtendedFrames(rdr, os.frames);
                        obj.Add(os);
                    }
                    mAdv.addObject(obj);
                }
                //CHARACTER
                else if (typename[0] == "Character")
                {
                    AdvCharacter chr = new AdvCharacter(mAdv);
                    chr.Name = typename[1];
                    str = rdr.ReadLine();
                    chr.TextColor = Convert.ToUInt32(str);
                    chr.WalkSpeed = Convert.ToInt32(rdr.ReadLine());
                    int tmp = Convert.ToInt32(rdr.ReadLine());
                    chr.NoZoom = tmp != 0;
                    if (ver_major > 3 || (ver_major == 3 && ver_minor > 0))
                    {
                        tmp = Convert.ToInt32(rdr.ReadLine());
                        chr.RealLeftAnimations = tmp != 0;
                    }
                    tmp = Convert.ToInt32(rdr.ReadLine());
                    chr.MemoryReistent = tmp != 0;
                    tmp = Convert.ToInt32(rdr.ReadLine());
                    chr.Ghost = tmp != 0;
                    chr.Walksound = rdr.ReadLine();
                    if (ver_major >= 3)
                    {
                        str = rdr.ReadLine();
                        string[] names = str.Split(';');
                        for (int i = 0; i < names.Length - 1; ++i)
                        {
                            chr.setStateName(16 + i, names[i]);
                        }
                    }
                    chr.Font = Convert.ToInt32(rdr.ReadLine());
                    chr.Zoom = Convert.ToInt32(rdr.ReadLine());
                    for (int state = 0; state < CHAR_STATES_MAX; ++state)
                    {
                        CharacterState cs = new CharacterState();
                        cs.size.x = Convert.ToInt32(rdr.ReadLine());
                        cs.size.y = Convert.ToInt32(rdr.ReadLine());
                        cs.basepoint.x = Convert.ToInt32(rdr.ReadLine());
                        cs.basepoint.y = Convert.ToInt32(rdr.ReadLine());
                        cs.fpsDivider = readExtendedFrames(rdr, cs.frames);
                        chr.Add(cs);
                    }
                    mAdv.addCharacter(chr);
                }
                //CHARACTER INSTANCE
                else if (typename[0] == "Rcharacter")
                {
                }
                //ROOM
                else if (typename[0] == "Room")
                {
                    Room room = new Room();
                    room.Name = typename[1];
                    room.Size.x = Convert.ToInt32(rdr.ReadLine());
                    room.Size.y = Convert.ToInt32(rdr.ReadLine());
                    room.ScrollOffset.x = Convert.ToInt32(rdr.ReadLine());
                    room.ScrollOffset.y = Convert.ToInt32(rdr.ReadLine());
                    room.Depthmap.x = Convert.ToInt32(rdr.ReadLine());
                    room.Depthmap.y = Convert.ToInt32(rdr.ReadLine());
                    room.Zoom = Convert.ToInt32(rdr.ReadLine());
                    room.Background = rdr.ReadLine();
                    room.ParallaxBackground = rdr.ReadLine();
                    int tmp = Convert.ToInt32(rdr.ReadLine());
                    room.DoubleWalkmap = tmp != 0;
                    if (ver_major >= 3)
                    {
                        for (int i = 0; i < FXSHAPES_MAX; ++i)
                        {
                            //TODO
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                            rdr.ReadLine();
                        }
                        //TODO unknown
                        rdr.ReadLine();
                    }
                    //inventory
                    str = rdr.ReadLine();
                    string[] inventory = str.Split(';');
                    room.InvPos.x = Convert.ToInt32(inventory[0]);
                    room.InvPos.y = Convert.ToInt32(inventory[1]);
                    room.InvSize.x = Convert.ToInt32(inventory[2]);
                    room.InvSize.y = Convert.ToInt32(inventory[3]);
                    room.InvScale.x = (float)Convert.ToDouble(inventory[4]);
                    room.InvScale.y = (float)Convert.ToDouble(inventory[5]);
                    //walkmap
                    str = rdr.ReadLine();
                    //TODO
                    mAdv.addRoom(room);
                }
                //OBJECT INSTANCE
                else if (typename[0] == "Robject")
                {
                }
            }
            return true;
        }

        protected bool readScripts(Stream strm)
        {
            StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding("iso-8859-1"));
            string str = rdr.ReadLine();
            int ver_major = Int32.Parse(str.Substring(0, 1));
            int ver_minor = Int32.Parse(str.Substring(2, 1));
            if (str.Substring(4) != "Point&Click Project File. DO NOT MODIFY!!")
                return false;
            str = rdr.ReadLine();
            mLastScript = null;
            while (!rdr.EndOfStream)
            {
                str = rdr.ReadLine();
                if (str.Length >= 2 && str.StartsWith("//"))
                {
                    string rest = str.Substring(2);
                    int idx = rest.IndexOf(' ');
                    string[] typename = new string[2];
                    typename[0] = rest.Substring(0, idx);
                    typename[1] = rest.Substring(idx + 1);
                    Script.Type scrType;
                    if (typename[0] == "Cutscene")
                        scrType = Script.Type.CUTSCENE;
                    else if (typename[0] == "Item")
                        scrType = Script.Type.ITEM;
                    else if (typename[0] == "Character")
                        scrType = Script.Type.CHARACTER;
                    else if (typename[0] == "Room")
                        scrType = Script.Type.ROOM;
                    else if (typename[0] == "Object")
                        scrType = Script.Type.OBJECT;
                    else if (typename[0] == "Walkmap")
                        scrType = Script.Type.WALKMAP;
                    else
                        return false;
                    Script scr = new Script(scrType);
                    scr.Name = typename[1];
                    if (scrType == Script.Type.WALKMAP)
                    {
                        string roomname;
                        Vec2i pos;
                        if (ver_major >= 3)
                        {
                            roomname = scr.Name.Substring(7);
                            pos.x = Convert.ToInt32(scr.Name.Substring(1, 3));
                            pos.y = Convert.ToInt32(scr.Name.Substring(4, 3));
                        }
                        else
                        {
                            roomname = scr.Name.Substring(4);
                            pos.x = Convert.ToInt32(scr.Name.Substring(0, 2));
                            pos.y = Convert.ToInt32(scr.Name.Substring(2, 2));
                        }
                        mAdv.addWalkmapScript(scr, pos, roomname);
                    }
                    else
                    {
                        mAdv.addScript(scr);
                    }
                    mLastScript = scr;
                }
                else
                {
                    mLastScript.Text += str + "\n";
                }
            }
            return true;
        }

        protected bool readPassword(Stream strm){
          StreamReader rdr = new StreamReader(strm, Encoding.GetEncoding("iso-8859-1"));
          string str = rdr.ReadLine();
          string pwd = "";
          for (int i = 0; i < 20; ++i){
            int val = Convert.ToInt32(str.Substring(i*3, 3));
            char curr;
            if (i >= 15)
              curr = (char)(val/3);
            else if (i >= 10)
              curr = (char)(val/6);
            else if (i >= 5)
              curr = (char)(val/4);
            else
              curr = (char)(val/5);
            pwd += curr;
          }
          mZipPwd = pwd;
          return true;
        }

        protected int readExtendedFrames(StreamReader rdr, System.Collections.ArrayList frames)
        {
            string str;
            for (int frms = 0; frms < FRAMES2_MAX; ++frms)
            {
                ExtendedFrame frm = new ExtendedFrame();
                bool set = false;
                for (int parts = 0; parts < PARTS_MAX; ++parts)
                {
                    str = rdr.ReadLine();
                    if (str.Length > 0)
                    {
                        set = true;
                        frm.names.Add(str);
                    }
                }
                //read offsets
                str = rdr.ReadLine();
                string[] split = str.Split(';');
                for (int i = 0; i < frm.names.Count; ++i)
                {
                    Vec2i offset = new Vec2i(Convert.ToInt32(split[2*i]), Convert.ToInt32(split[2*i+1]));
                    frm.offsets.Add(offset);
                }
                if (set)
                    frames.Add(frm);
            }
            str = rdr.ReadLine();
            return Convert.ToInt32(str);
        }

        TreeNode insertTreeElement(TreeNodeCollection nodes, string name, TreeNode current, int curr_level)
        {
            int level = 0;
            for (int i = 0; i < name.Length; ++i)
            {
                if (name[i] == '"')
                    ++level;
                else
                    break;
            }
            string idstr = name.Substring(level, 2);
            string label = name.Substring(level + 2);
            int leveldiff = level - curr_level;
            while (leveldiff <= 0)
            {
                current = current.Parent;
                ++leveldiff;
            }
            TreeNode node = new TreeNode(label);
            node.Tag =  (ResourceID)Convert.ToInt32(idstr);
            current.Nodes.Add(node);
            return node;
        }

        public System.Drawing.Bitmap getImage(string filename)
        {
            string zipname = mPath + System.IO.Path.DirectorySeparatorChar + "gfx.dat";
            string imagename = System.IO.Path.GetFileName(filename);
            ZipInputStream zis = new ZipInputStream(File.OpenRead(zipname));
            if (mZipPwd.Length > 0)
                zis.Password = mZipPwd;
            ZipEntry entry;
            char[] arr = imagename.ToCharArray();
            for (int i = 0; i < imagename.Length; ++i)
            {
                if (arr[i] == '\xf6')
                    arr[i] = '\xf7';
                else if (arr[i] == 'ä')
                    arr[i] = 'õ';
                else if (arr[i] == 'ü')
                    arr[i] = '³';
                else if (arr[i] == 'ß')
                    arr[i] = '\x2580';
            }
            imagename = new string(arr);
            /*Encoder enc = Encoding.GetEncoding("iso-8859-1").GetEncoder();
            //char[] arr = imagename.ToCharArray();
            byte[] conv = new byte[arr.Length];
            int charsUsed;
            int bytesUsed;
            bool completed;
            enc.Convert(arr, 0, arr.Length, conv, 0, conv.Length, true, out charsUsed, out bytesUsed, out completed);
            Decoder dec = new System.Text.UTF8Encoding().GetDecoder();
            char[] convchars = new char[conv.Length];
            dec.Convert(conv, 0, conv.Length, convchars, 0, convchars.Length, true, out charsUsed, out bytesUsed, out completed);
            for (int i = 0; i < convchars.Length; ++i)
            {
                convchars[i] = (char)(convchars[i] % 256);
            }
            string convname = new String(convchars);*/
            System.Drawing.Bitmap img = null;
            while ((entry = zis.GetNextEntry()) != null)
            {
                /*if (entry.Name[1] == 'c' && entry.Name[2] == 'h')
                {
                    char[] tmp = entry.Name.ToCharArray();
                    //break;
                }*/
                if (entry.Name == imagename)
                {
                    img = (Bitmap)System.Drawing.Bitmap.FromStream(zis);
                    if (System.IO.Path.GetExtension(imagename) == ".pnj")
                    {
                        System.Drawing.Bitmap alpha = getImage(System.IO.Path.ChangeExtension(filename, ".pna"));
                        System.Drawing.Bitmap newImage = new System.Drawing.Bitmap(img.Width, img.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                        for (int i = 0; i < img.Width; ++i){
                            for (int j = 0; j < img.Height; ++j)
                            {
                                newImage.SetPixel(i,j,Color.FromArgb(alpha.GetPixel(i,j).R, img.GetPixel(i,j)));
                            }
                        }
                        img = newImage;
                    }
                    break;
                }
            }
            zis.Close();
            return img;
        }

        private AdvData mAdv;

        public AdvData Data
        {
            get { return mAdv; }
        }
        private TreeView mMediaPool;
        private TreeView mGamePool;
        private string mPath;
        private Script mLastScript;
        private string mZipPwd;
    }
}
