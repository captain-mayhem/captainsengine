using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using ICSharpCode.SharpZipLib.Zip;

namespace StoryDesigner
{
    class AdvFileReader
    {
        public AdvFileReader(string filename, TreeView mediapool, TreeView gamepool)
        {
            mAdv = new AdvData();
            mMediaPool = mediapool;
            mGamePool = gamepool;

            ZipInputStream zis = new ZipInputStream(File.OpenRead(filename));
            ZipEntry entry;
            while ((entry = zis.GetNextEntry()) != null)
            {
                if (entry.Name == "game.001")
                {
                    readSettings(zis);
                }
            }
            zis.Close();
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
            str = rdr.ReadLine();
            str = rdr.ReadLine();
            mAdv.Settings.TsBackground = rdr.ReadLine();
            str = rdr.ReadLine();
            if (str.Substring(0, 14) != "Startskript : ")
            {
                throw new UnexpectedValueException("expected Startskript : ");
            }
            mAdv.Settings.Startscript = str.Substring(14);
            while (str != "Mediapool :")
            {
                str = rdr.ReadLine();
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

        protected AdvData mAdv;
        private TreeView mMediaPool;
        private TreeView mGamePool;
    }
}
