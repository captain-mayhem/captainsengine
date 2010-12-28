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
            string font;
            do
            {
                font = rdr.ReadLine();
            } while (font.Substring(0, 11) != "GameFont : ");
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
            rdr.ReadLine();
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
                cs.fps = FPS_MAX / Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.x = Convert.ToInt32(str);
                str = rdr.ReadLine();
                cs.highlight.y = Convert.ToInt32(str);
                mAdv.Cursor.Add(cs);
            }
            return true;
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
            ZipEntry entry;
            char[] arr = imagename.ToCharArray();
            for (int i = 0; i < imagename.Length; ++i)
            {
                if (arr[i] == '\xf6')
                    arr[i] = '\xf7';
                else if (arr[i] == '�')
                    arr[i] = '�';
                else if (arr[i] == '�')
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
    }
}
