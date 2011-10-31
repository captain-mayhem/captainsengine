using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using ICSharpCode.SharpZipLib.Zip;
using ICSharpCode.SharpZipLib.Core;

namespace StoryDesigner
{
    class AdvFileWriter
    {
        public AdvFileWriter(AdvData data){
            ZipConstants.DefaultCodePage = 1252;
            mData = data;
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
    }
}
