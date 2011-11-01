using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using ICSharpCode.SharpZipLib.Zip;
using ICSharpCode.SharpZipLib.Core;
using System.Windows.Forms;

namespace StoryDesigner
{
    class AdvFileWriter
    {
        public AdvFileWriter(AdvData data, TreeView gamepool, TreeView mediapool){
            ZipConstants.DefaultCodePage = 1252;
            mData = data;
            mGamepool = gamepool;
            mMediapool = mediapool;
        }

        public void writeGame(string path)
        {
            writeGameFile();
            writeFonts();
        }

        void writeGameFile()
        {
            FileStream fs = new FileStream("game.dat", FileMode.Create);
            ZipOutputStream zs = new ZipOutputStream(fs);
            zs.UseZip64 = UseZip64.Off;
            ZipEntry ze1 = new ZipEntry("game.001");
            zs.PutNextEntry(ze1);
            writeSettings(zs);
            zs.CloseEntry();
            ZipEntry ze2 = new ZipEntry("game.002");
            zs.PutNextEntry(ze2);
            writeObjects(zs);
            zs.CloseEntry();
            zs.Finish();
            fs.Close();
        }

        void writeSettings(Stream strm)
        {
            StreamWriter swr = new StreamWriter(strm, Encoding.GetEncoding("iso-8859-1"));
            swr.WriteLine("3.2 Point&Click Project File. DO NOT MODIFY!!");
            swr.WriteLine();
            swr.WriteLine(mData.Settings.Directory);
            swr.Write("Resolution X : ");
            swr.WriteLine(mData.Settings.Resolution.x);
            foreach (FontInfo fi in mData.Settings.Fonts)
            {
                swr.Write(fi.name); swr.Write(';');
                swr.Write(fi.bold ? -1 : 0); swr.Write(';');
                swr.Write(fi.italic ? -1 : 0); swr.Write(';');
                swr.Write(fi.size); swr.Write(';');
                swr.Write(fi.outline); swr.Write(';');
                swr.Write(fi.charset); swr.Write(';');
                swr.Write(fi.shadow); swr.Write(';');
                swr.Write(fi.fill); swr.Write(';');
                swr.WriteLine(fi.fading);
            }
            swr.WriteLine("GameFont : ");
            for (int i = 2; i <= 6; ++i)
            {
                swr.WriteLine("GameFont"+i+" : ");
            }
            swr.WriteLine("Gameicon : " + mData.Settings.GameIcon);
            swr.WriteLine(mData.Settings.LoadingImage);
            swr.WriteLine(mData.Settings.TsUseBgImage ? -1 : 0);
            swr.WriteLine(mData.Settings.TsBackground);
            swr.WriteLine("Startskript : " + mData.Settings.Startscript);
            swr.WriteLine(mData.Settings.Mainscript);
            swr.WriteLine(mData.Settings.AnywhereRoom);
            swr.WriteLine("Screenchange : " + (int)mData.Settings.ScreenChange);
            swr.Write("Checks : ");
            swr.Write(mData.Settings.TextOnOff ? 1 : 0);
            swr.Write(mData.Settings.DrawDraggedItemIcons ? 1 : 0);
            swr.Write(mData.Settings.ActionText ? 1 : 0);
            swr.Write(mData.Settings.ShowTaskbar ? 1 : 0);
            swr.Write(mData.Settings.NotAntialiased ? 1 : 0);
            swr.Write(mData.Settings.TaskbarFromTop ? 1 : 0);
            swr.Write(mData.Settings.GroupItems ? 1 : 0);
            swr.WriteLine(mData.Settings.ProtectGameFile ? 1 : 0);
            swr.WriteLine(mData.Settings.ActionTextHeight);
            swr.WriteLine(mData.Settings.CustomMenu ? -1 : 0);
            swr.WriteLine(mData.Settings.CustomMenuRoom);
            swr.WriteLine("Taskheight : " + mData.Settings.TaskHeight);
            swr.WriteLine("Taskroom : " + mData.Settings.TaskRoom);
            swr.WriteLine("Taskpopup : " + mData.Settings.TaskPopup);
            swr.Write("Setup : ");
            swr.Write(mData.Settings.SilentDelete ? 1 : 0);
            swr.Write(mData.Settings.InfoLine ? 1 : 0);
            swr.Write(mData.Settings.TaskHideCompletely ? ";hide;" : ";show;");
            swr.WriteLine(mData.Settings.RightClick);
            swr.WriteLine(mData.Settings.UseMouseWheel ? -1 : 0);
            swr.WriteLine(mData.Settings.MenuFading);
            swr.WriteLine(mData.Settings.TextSceneFading);
            swr.WriteLine(mData.Settings.AnywhereTransparency);
            swr.Write("Targacolor : ");
            swr.WriteLine(mData.Settings.TargaColor);
            swr.Write("Bordercolor : ");
            swr.WriteLine(mData.Settings.BorderColor);
            swr.Write("Backgroundcolor : ");
            swr.WriteLine(mData.Settings.BackgroundColor);
            swr.Write("Textcolor : ");
            swr.WriteLine(mData.Settings.TextColor);
            swr.Write("Offtextcolor : ");
            swr.Write(mData.Settings.OffspeechColor);
            swr.Write(';');
            swr.WriteLine(mData.Settings.InfotextColor);
            swr.WriteLine(mData.Settings.TsStyle + 1);
            swr.WriteLine(mData.Settings.TsBorderStyle + 1);
            swr.WriteLine(mData.Settings.TsBorderColor);
            swr.WriteLine(mData.Settings.TsAreaColor);
            swr.WriteLine(mData.Settings.TsSelectionColor);
            swr.WriteLine(mData.Settings.TsTextColor);
            swr.WriteLine(mData.Settings.TsUseSymbols ? -1 : 0);
            swr.WriteLine(mData.Settings.MuteMusicWhenSpeech ? -1 : 0);
            swr.WriteLine(mData.Settings.CoinActivated ? -1 : 0);
            swr.WriteLine(mData.Settings.CoinAutoPopup ? -1 : 0);
            swr.WriteLine(mData.Settings.CoinRoom);
            swr.WriteLine(mData.Settings.CoinFading);
            swr.WriteLine(mData.Settings.CoinCenter.x);
            swr.WriteLine(mData.Settings.CoinCenter.y);
            swr.Write("Linktext : ");
            swr.WriteLine(mData.Settings.LinkText);
            swr.Write("Givelink : ");
            swr.WriteLine(mData.Settings.GiveLink);
            swr.Write("Walktext : ");
            swr.WriteLine(mData.Settings.WalkText);
            swr.WriteLine("Booleans :");
            foreach (KeyValuePair<string, bool> boolvar in mData.Settings.Booleans)
            {
                swr.Write(boolvar.Key);
                swr.WriteLine(boolvar.Value ? -1 : 0);
            }
            swr.WriteLine("Commands :");
            foreach (KeyValuePair<string, string> cmd in mData.Settings.Commands)
            {
                swr.WriteLine(cmd.Key);
                swr.WriteLine(cmd.Value);
            }
            swr.WriteLine("Mediapool :");
            writeTreeEntry(swr, mMediapool.Nodes, 0);
            swr.WriteLine("Gamepool :");
            writeTreeEntry(swr, mGamepool.Nodes, 0);
            swr.WriteLine("Images :");
            foreach (KeyValuePair<string, string> image in mData.Images)
            {
                swr.WriteLine(image.Key);
                swr.WriteLine(image.Value);
            }
            swr.WriteLine("Sounds :");
            foreach (KeyValuePair<string, string> sound in mData.Sounds)
            {
                swr.WriteLine(sound.Key);
                swr.WriteLine(sound.Value);
            }
            swr.WriteLine("Music :");
            foreach (KeyValuePair<string, string> music in mData.Music)
            {
                swr.WriteLine(music.Key);
                swr.WriteLine(music.Value);
            }
            swr.WriteLine("Movie :");
            foreach (KeyValuePair<string, string> video in mData.Videos)
            {
                swr.WriteLine(video.Key);
                swr.WriteLine(video.Value);
            }
            swr.Flush();
        }

        void writeTreeEntry(StreamWriter swr, TreeNodeCollection nodes, int level)
        {
            foreach (TreeNode node in nodes)
            {
                for (int i = 0; i < level; ++i)
                    swr.Write('"');
                swr.Write(String.Format("{0:D2}",(int)node.Tag));
                swr.WriteLine(node.Text);
                writeTreeEntry(swr, node.Nodes, level + 1);
            }
        }

        void writeObjects(Stream strm)
        {
            StreamWriter swr = new StreamWriter(strm, Encoding.GetEncoding("iso-8859-1"));
            swr.WriteLine("3.2 Point&Click Project File. DO NOT MODIFY!!");
            swr.WriteLine();
            swr.Flush();
        }

        void writeFonts()
        {
            FileStream fs = new FileStream("fonts.dat", FileMode.Create);
            ZipOutputStream masterz = new ZipOutputStream(fs);
            masterz.UseZip64 = UseZip64.Off;
            for (int font = 1; font <= mData.Settings.Fonts.Count; ++font){
                Fonts fnts = new Fonts(mData);
                Bitmap[] images;
                Bitmap[] alphaimages;
                Vec2i dims;
                Vec2i numchars;
                int[] widths = fnts.createBitmapFont(font, out images, out alphaimages, out dims, out numchars);

                string fontname = String.Format("font.{0:D3}", font);
                ZipEntry mze = new ZipEntry(fontname);
                masterz.PutNextEntry(mze);
                ZipOutputStream zos = new ZipOutputStream(masterz);
                zos.UseZip64 = UseZip64.Off;
                for (int i = 0; i < images.Length; ++i)
                {
                    string imagename = String.Format("font{0}.bm{1}", font, i+1);
                    ZipEntry ze = new ZipEntry(imagename);
                    zos.PutNextEntry(ze);
                    images[i].Save(zos, ImageFormat.Bmp);
                    imagename = String.Format("font{0}.al{1}", font, i + 1);
                    zos.CloseEntry();
                    ze = new ZipEntry(imagename);
                    zos.PutNextEntry(ze);
                    alphaimages[i].Save(zos, ImageFormat.Bmp);
                    zos.CloseEntry();
                }
                string dataname = String.Format("fontdata.{0:D3}", font);
                ZipEntry zippe = new ZipEntry(dataname);
                zos.PutNextEntry(zippe);
                
                StreamWriter swr = new StreamWriter(zos, Encoding.GetEncoding("iso-8859-1"));
                swr.WriteLine(images.Length);
                swr.WriteLine(dims.x);
                swr.WriteLine(dims.y);
                swr.WriteLine(numchars.x);
                swr.WriteLine(numchars.y);
                foreach (int width in widths)
                {
                    swr.WriteLine(width);
                }
                swr.Flush();
                zos.CloseEntry();
                zos.Finish();

                masterz.CloseEntry();
            }
            masterz.Finish();
            fs.Close();
        }

        protected AdvData mData;
        TreeView mGamepool;
        TreeView mMediapool;
    }
}
