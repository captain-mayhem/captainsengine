using System;
using System.Collections;
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
        protected readonly float FPS_MAX = 50.0f;
        protected readonly int STATES_MAX = 10;
        protected readonly int CHAR_STATES_MAX = 36;
        protected readonly int FRAMES_MAX = 25;
        protected readonly int FRAMES2_MAX = 30;
        protected readonly int PARTS_MAX = 2;
        protected readonly int FXSHAPES_MAX = 3;

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
            try
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

                ZipEntry ze3 = new ZipEntry("game.003");
                zs.PutNextEntry(ze3);
                writeScripts(zs);
                zs.CloseEntry();

                zs.Finish();
                fs.Close();
            }
            catch (Exception)
            {
                MessageBox.Show("Failed to write game.dat. Please make sure that it is a writable file.");
            }
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
            for (int i = 0; i < STATES_MAX; ++i)
            {
                for (int frames = 0; frames < FRAMES_MAX; ++frames)
                {
                    string[] data = mData.Cursor.getFrame(i, frames);
                    if (data != null)
                        swr.WriteLine(data[0]);
                    else
                        swr.WriteLine();
                }
                swr.WriteLine(mData.Cursor.getCommand(i)+1);
                swr.WriteLine(mData.Cursor.getFPSDivider(i));
                swr.WriteLine(mData.Cursor.getHotspot(i).x);
                swr.WriteLine(mData.Cursor.getHotspot(i).y);
            }
            foreach (KeyValuePair<string,Item> item in mData.Items){
                swr.Write("//Item ");
                swr.WriteLine(item.Value.Name);
                for (int i = 0; i < STATES_MAX; ++i)
                {
                    for (int frames = 0; frames < FRAMES_MAX; ++frames)
                    {
                        string[] data = item.Value.getFrame(i, frames);
                        if (data != null)
                            swr.Write(data[0]);
                        //else
                        //    swr.Write();
                        swr.WriteLine(';');
                    }
                    swr.WriteLine(item.Value.getFPSDivider(i));
                }
            }
            foreach (KeyValuePair<string, AdvObject> obj in mData.Objects)
            {
                swr.Write("//Object ");
                swr.WriteLine(obj.Value.Name);
                swr.WriteLine(obj.Value.getSize(0).x);
                swr.WriteLine(obj.Value.getSize(0).y);
                swr.WriteLine(obj.Value.Lighten ? -1 : 0);
                for (int i = 0; i < STATES_MAX; ++i)
                {
                    for (int frames = 0; frames < FRAMES2_MAX; ++frames)
                    {
                        string[] data = obj.Value.getFrame(i, frames);
                        if (data == null)
                        {
                            swr.WriteLine();
                            swr.WriteLine();
                        }
                        else if (data.Length == 1)
                        {
                            swr.WriteLine(data[0]);
                            swr.WriteLine();
                        }
                        else
                        {
                            swr.WriteLine(data[0]);
                            swr.WriteLine(data[1]);
                        }
                        swr.Write(obj.Value.getFramePartOffset(i, frames, 0).x);
                        swr.Write(';');
                        swr.Write(obj.Value.getFramePartOffset(i, frames, 0).y);
                        swr.Write(';');
                        swr.Write(obj.Value.getFramePartOffset(i, frames, 1).x);
                        swr.Write(';');
                        swr.WriteLine(obj.Value.getFramePartOffset(i, frames, 1).y);
                    }
                    swr.WriteLine(obj.Value.getFPSDivider(i));
                }
            }
            foreach (KeyValuePair<string, AdvCharacter> charact in mData.Characters)
            {
                AdvCharacter chr = charact.Value;
                swr.Write("//Character ");
                swr.WriteLine(chr.Name);
                swr.WriteLine(chr.TextColor);
                swr.WriteLine(chr.WalkSpeed);
                swr.WriteLine(chr.NoZoom ? -1 : 0);
                swr.WriteLine(chr.RealLeftAnimations ? -1 : 0);
                swr.WriteLine(chr.MemoryReistent ? -1 : 0);
                swr.WriteLine(chr.Ghost ? -1 : 0);
                swr.WriteLine(chr.Walksound);
                for (int i = 0; i < 20; ++i)
                {
                    swr.Write(chr.getStateName(16 + i, false));
                    swr.Write(';');
                }
                swr.WriteLine();
                swr.WriteLine(chr.Font);
                swr.WriteLine(chr.Zoom);
                for (int i = 0; i < CHAR_STATES_MAX; ++i)
                {
                    swr.WriteLine(chr.getSize(i).x);
                    swr.WriteLine(chr.getSize(i).y);
                    swr.WriteLine(chr.getHotspot(i).x);
                    swr.WriteLine(chr.getHotspot(i).y);
                    for (int frames = 0; frames < FRAMES2_MAX; ++frames)
                    {
                        string[] data = chr.getFrame(i, frames);
                        if (data == null)
                        {
                            swr.WriteLine();
                            swr.WriteLine();
                        }
                        else if (data.Length == 1)
                        {
                            swr.WriteLine(data[0]);
                            swr.WriteLine();
                        }
                        else
                        {
                            swr.WriteLine(data[0]);
                            swr.WriteLine(data[1]);
                        }
                        swr.Write(chr.getFramePartOffset(i, frames, 0).x);
                        swr.Write(';');
                        swr.Write(chr.getFramePartOffset(i, frames, 0).y);
                        swr.Write(';');
                        swr.Write(chr.getFramePartOffset(i, frames, 1).x);
                        swr.Write(';');
                        swr.WriteLine(chr.getFramePartOffset(i, frames, 1).y);
                    }
                    swr.WriteLine(chr.getFPSDivider(i));
                }
            }
            foreach (KeyValuePair<string, ArrayList> charact in mData.CharacterInstances)
            {
                foreach (CharacterInstance chr in charact.Value)
                {
                    swr.Write("//Rcharacter ");
                    swr.WriteLine(chr.Name);
                    swr.WriteLine(chr.Character.Name);
                    swr.WriteLine(chr.Room);
                    swr.WriteLine(chr.RawPosition.x);
                    swr.WriteLine(chr.RawPosition.y);
                    swr.WriteLine(chr.LookDir);
                    swr.WriteLine(chr.Unmovable ? 0 : -1);
                    swr.WriteLine(chr.Locked ? -1 : 0);
                }
            }
            foreach (KeyValuePair<string, Room> rom in mData.Rooms)
            {
                Room room = rom.Value;
                swr.Write("//Room ");
                swr.WriteLine(room.Name);
                swr.WriteLine(room.Size.x);
                swr.WriteLine(room.Size.y);
                swr.WriteLine(room.ScrollOffset.x);
                swr.WriteLine(room.ScrollOffset.y);
                swr.WriteLine(room.Depthmap.x);
                swr.WriteLine(room.Depthmap.y);
                swr.WriteLine(room.Zoom);
                swr.WriteLine(room.Background);
                swr.WriteLine(room.ParallaxBackground);
                swr.WriteLine(room.DoubleWalkmap ? -1 : 0);
                for (int i = 0; i < FXSHAPES_MAX; ++i)
                {
                    FxShape shape = (FxShape)room.FXShapes[i];
                    swr.Write(shape.Active ? -1 : 0);
                    swr.Write(';');
                    swr.WriteLine(shape.DependingOnRoomPosition ? -1 : 0);
                    swr.WriteLine((int)shape.Effect);
                    swr.WriteLine(shape.Room);
                    swr.WriteLine(shape.Depth);
                    swr.WriteLine(shape.MirrorOffset.x);
                    swr.WriteLine(shape.MirrorOffset.y);
                    swr.WriteLine(shape.Strength);
                    for (int pos = 0; pos < 4; ++pos)
                    {
                        swr.Write(shape.Positions[pos].x);
                        swr.Write(';');
                        swr.Write(shape.Positions[pos].y);
                        swr.Write(';');
                    }
                    swr.WriteLine();
                }
                //Inventory
                swr.WriteLine(room.HasInventory ? -1 : 0);
                swr.Write(room.InvPos.x); swr.Write(';');
                swr.Write(room.InvPos.y); swr.Write(';');
                swr.Write(room.InvSize.x); swr.Write(';');
                swr.Write(room.InvSize.y); swr.Write(';');
                System.Globalization.NumberFormatInfo info = System.Globalization.NumberFormatInfo.InvariantInfo;
                swr.Write(String.Format(info, "{0:0.##############}",room.InvScale.x)); swr.Write(';');
                swr.WriteLine(String.Format(info, "{0:0.##############}", room.InvScale.y));
                //walkmap
                for (int i = 0; i < room.Walkmap.GetUpperBound(0); ++i)
                {
                    for (int j = 0; j < room.Walkmap.GetUpperBound(1); ++j)
                    {
                        Room.WalkMapEntry entry = room.Walkmap[i, j];
                        swr.Write(entry.isFree ? 0 : 1);
                        swr.Write(entry.hasScript ? 1 : 0);
                    }
                }
                swr.WriteLine();
                //objects
                foreach (ObjectInstance obj in room.Objects)
                {
                    swr.Write(";;Roomobject ");
                    swr.WriteLine(obj.Name);
                    swr.WriteLine(obj.Object.Name);
                    swr.WriteLine(obj.Position.x);
                    swr.WriteLine(obj.Position.y);
                    swr.WriteLine(obj.State);
                    swr.WriteLine(obj.Layer);
                    swr.WriteLine(obj.Depth);
                    swr.WriteLine(obj.Locked ? -1 : 0);
                }
            }
            swr.Flush();
        }

        void writeScripts(Stream strm)
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
