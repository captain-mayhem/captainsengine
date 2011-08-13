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
            writeFonts();
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
