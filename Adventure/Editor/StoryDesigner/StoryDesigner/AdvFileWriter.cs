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
            string dir = Path.GetDirectoryName(path);
            string datadir = dir + Path.DirectorySeparatorChar + "data";
            if (!Directory.Exists(datadir))
                Directory.CreateDirectory(datadir);
            datadir += Path.DirectorySeparatorChar;
            writeProjectFile(datadir+"game.dat");
            writeFonts(datadir);
            packGraphics(datadir);
            packData(datadir, mData.Sounds, "sfx.dat");
            packData(datadir, mData.Music, "music.dat");
            packData(datadir, mData.Videos, "movie.dat");
        }

        public void writeProjectFile(string path)
        {
            try
            {
                string ext = Path.GetExtension(path);
                bool writeDat = ext == ".dat";
                string entry = Path.GetFileNameWithoutExtension(path);
                FileStream fs = new FileStream(path, FileMode.Create);
                ZipOutputStream zs = new ZipOutputStream(fs);
                zs.UseZip64 = UseZip64.Off;

                ZipEntry ze1 = new ZipEntry(entry+".001");
                zs.PutNextEntry(ze1);
                writeSettings(zs, writeDat);
                zs.CloseEntry();

                ZipEntry ze2 = new ZipEntry(entry+".002");
                zs.PutNextEntry(ze2);
                writeObjects(zs);
                zs.CloseEntry();

                ZipEntry ze3 = new ZipEntry(entry+".003");
                zs.PutNextEntry(ze3);
                writeScripts(zs);
                zs.CloseEntry();

                ZipEntry ze4 = new ZipEntry(entry+".004");
                zs.PutNextEntry(ze4);
                writeLanguages(zs);
                zs.CloseEntry();

                zs.Finish();
                fs.Close();
            }
            catch (Exception)
            {
                MessageBox.Show("Failed to write game.dat. Please make sure that it is a writable file.");
            }
        }

        void writeSettings(Stream strm, bool convertPngs)
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
                if (convertPngs && Path.GetExtension(image.Value) == ".png")
                {
                    string towrite = image.Value.Substring(0, image.Value.Length - 1);
                    towrite += 'j';
                    swr.WriteLine(towrite);
                }
                else
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
                System.Globalization.NumberFormatInfo info = new System.Globalization.NumberFormatInfo();
                info.NumberDecimalSeparator = ",";
                swr.Write(String.Format(info, "{0:0.##############}",room.InvScale.x)); swr.Write(';');
                swr.WriteLine(String.Format(info, "{0:0.##############}", room.InvScale.y));
                //walkmap
                for (int i = 0; i <= room.Walkmap.GetUpperBound(0); ++i)
                {
                    for (int j = 0; j <= room.Walkmap.GetUpperBound(1); ++j)
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
            foreach (KeyValuePair<string,Script> script in mData.getScripts(Script.Type.CUTSCENE)){
                swr.Write("//Cutscene ");
                swr.WriteLine(script.Value.Name);
                swr.WriteLine(script.Value.Text);
            }
            foreach (KeyValuePair<string, Script> script in mData.getScripts(Script.Type.ITEM))
            {
                swr.Write("//Item ");
                swr.WriteLine(script.Value.Name);
                swr.WriteLine(script.Value.Text);
            }
            foreach (KeyValuePair<string, Script> script in mData.getScripts(Script.Type.CHARACTER))
            {
                swr.Write("//Character ");
                swr.WriteLine(script.Value.Name);
                swr.WriteLine(script.Value.Text);
            }
            foreach (KeyValuePair<string, Room> room in mData.Rooms){
                Script scr = mData.getScript(Script.Type.ROOM, room.Value.Name);
                if (scr != null)
                {
                    swr.Write("//Room ");
                    swr.WriteLine(scr.Name);
                    swr.WriteLine(scr.Text);
                }
                foreach (ObjectInstance obj in room.Value.Objects)
                {
                    Script oscr = mData.getScript(Script.Type.OBJECT, Script.toScriptName(obj.Name, room.Value.Name));
                    if (oscr != null)
                    {
                        swr.Write("//Object ");
                        swr.WriteLine(oscr.Name);
                        if (oscr.Text != null)
                            swr.WriteLine(oscr.Text);
                    }
                }
                for (int x = 0; x <= room.Value.Walkmap.GetUpperBound(0); ++x)
                {
                    for (int y = 0; y <= room.Value.Walkmap.GetUpperBound(1); ++y)
                    {
                        if (room.Value.Walkmap[x, y].hasScript)
                        {
                            Script wms = mData.getScript(Script.Type.WALKMAP, Script.toScriptName(x, y, room.Value.Name, null));
                            swr.Write("//Walkmap ");
                            swr.WriteLine(wms.Name);
                            swr.WriteLine(wms.Text);
                        }
                    }
                }
            }
            swr.Flush();
        }

        void writeLanguages(Stream strm)
        {
            StreamWriter swr = new StreamWriter(strm, Encoding.GetEncoding("iso-8859-1"));
            foreach (KeyValuePair<string,Language> language in mData.Languages){
                string lang = "*/*"+language.Value.Name+';';
                swr.WriteLine(lang+"speech");
                foreach(string str in language.Value.getWords(Language.Section.Speech))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "voicespeech");
                foreach (string str in language.Value.getWords(Language.Section.Speech_Sounds))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "offspeech");
                foreach (string str in language.Value.getWords(Language.Section.Offspeech))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "voiceoffspeech");
                foreach (string str in language.Value.getWords(Language.Section.Offspeech_Sounds))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "showinfo");
                foreach (string str in language.Value.getWords(Language.Section.Showinfo))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "textout");
                foreach (string str in language.Value.getWords(Language.Section.Textout))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "setstring");
                foreach (string str in language.Value.getWords(Language.Section.Setstring))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "row");
                foreach (string str in language.Value.getWords(Language.Section.Textscenes))
                    swr.WriteLine(str);
                swr.WriteLine(lang + "kommandos");
                foreach (string str in language.Value.getWords(Language.Section.Commands))
                    swr.WriteLine(str);
            }
            swr.Flush();
        }

        void writeFonts(string datadir)
        {
            FileStream fs = new FileStream(datadir+"fonts.dat", FileMode.Create);
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
            //write system font
            System.Reflection.Assembly assy = System.Reflection.Assembly.GetExecutingAssembly();
            Stream fontstr = assy.GetManifestResourceStream("StoryDesigner.Resources.font.dat");
            int length = (int)fontstr.Length;
            byte[] fontdat = new byte[length];
            fontstr.Read(fontdat, 0, length);
            fontstr.Close();
            FileStream fontout = new FileStream(datadir + "font.dat", FileMode.Create);
            fontout.Write(fontdat, 0, length);
            fontout.Close();
        }

        void packGraphics(string datadir)
        {
            FileStream fs = new FileStream(datadir + "gfx.dat", FileMode.Create);
            ZipOutputStream zos = new ZipOutputStream(fs);
            zos.UseZip64 = UseZip64.Off;

            foreach (KeyValuePair<string,string> entry in mData.Images){
                Bitmap bmp = (Bitmap)Bitmap.FromFile(entry.Value);
                string name = Path.GetFileName(entry.Value);
                ImageFormat fmt = bmp.RawFormat;
                if (fmt.Equals(ImageFormat.Png))
                {
                    //special handling for pngs
                    Bitmap rgb = new Bitmap(bmp.Width, bmp.Height);
                    Bitmap alpha = new Bitmap(bmp.Width, bmp.Height);
                    for (int x = 0; x < bmp.Width; ++x)
                    {
                        for (int y = 0; y < bmp.Height; ++y)
                        {
                            Color c = bmp.GetPixel(x, y);
                            rgb.SetPixel(x, y, Color.FromArgb(c.R, c.G, c.B));
                            alpha.SetPixel(x, y, Color.FromArgb(c.A, c.A, c.A));
                        }
                    }
                    string basename = Path.GetFileNameWithoutExtension(name);
                    EncoderParameters param = new EncoderParameters(1);
                    param.Param[0] = new EncoderParameter(System.Drawing.Imaging.Encoder.Quality, 100L);
                    ZipEntry ze1 = new ZipEntry(basename + ".pnj");
                    zos.PutNextEntry(ze1);
                    rgb.Save(zos, ImageFormat.Jpeg);
                    zos.CloseEntry();
                    ZipEntry ze2 = new ZipEntry(basename + ".pna");
                    zos.PutNextEntry(ze2);
                    alpha.Save(zos, ImageFormat.Jpeg);
                    zos.CloseEntry();
                }
                else
                {
                    ZipEntry ze = new ZipEntry(name);
                    zos.PutNextEntry(ze);
                    bmp.Save(zos, fmt);
                    zos.CloseEntry();
                }
            }

            zos.Finish();
            fs.Close();
        }

        void packData(string datadir, Dictionary<string, string> files, string output)
        {
            FileStream fs = new FileStream(datadir + output, FileMode.Create);
            ZipOutputStream zos = new ZipOutputStream(fs);
            zos.UseZip64 = UseZip64.Off;

            foreach (KeyValuePair<string, string> entry in files)
            {
                string name = Path.GetFileName(entry.Value);
                FileStream source = new FileStream(entry.Value, FileMode.Open);
                int length = (int)source.Length;
                byte [] buffer = new byte[length];
                int count;
                int sum = 0;
                while((count = source.Read(buffer, sum, length-sum)) > 0)
                    sum += count;
                source.Close();
                
                ZipEntry ze = new ZipEntry(name);
                zos.PutNextEntry(ze);
                zos.Write(buffer, 0, length);
                zos.CloseEntry();
            }

            zos.Finish();
            fs.Close();
        }

        protected AdvData mData;
        TreeView mGamepool;
        TreeView mMediapool;
    }
}
