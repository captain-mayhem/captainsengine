using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace StoryDesigner
{
    //! Makes videos playable by the engine, with videoconv.exe (next to the StoryDesigner)
    /*! The engine plays MPEG-1 (with MP2 sound) and Ogg Theora (with Vorbis);
        other videos (AVI with any codec, WMV, MPEG-2) are converted to Ogg
        Theora with the codecs installed on Windows. The converted file is
        kept next to the original (or, where that can't be written, in the
        user's local application data) and made again when the original
        changes. Flash animations (.swf) play as they are. */
    static class VideoConverter
    {
        static string ConverterPath
        {
            get { return Path.Combine(Path.GetDirectoryName(Application.ExecutablePath), "videoconv.exe"); }
        }

        //! the file the engine plays: the video itself, or its conversion (made if needed); null on failure
        public static string GetPlayableFile(string filename, out string error)
        {
            error = null;
            if (string.Equals(Path.GetExtension(filename), ".swf", StringComparison.OrdinalIgnoreCase))
                return filename;
            if (!File.Exists(ConverterPath))
            {
                //an installation without the converter packs the videos as they are
                return filename;
            }
            string output;
            if (run("--check \"" + filename + "\"", out output, out error) == 0)
                return filename;
            string converted = convertedPath(filename);
            FileInfo done = new FileInfo(converted);
            if (done.Exists && done.Length > 0 && done.LastWriteTimeUtc >= File.GetLastWriteTimeUtc(filename))
                return converted;
            //the StoryDesigner has a Cursor class of its own
            System.Windows.Forms.Cursor previous = System.Windows.Forms.Cursor.Current;
            System.Windows.Forms.Cursor.Current = Cursors.WaitCursor;
            int result = run("\"" + filename + "\" \"" + converted + "\"", out output, out error);
            System.Windows.Forms.Cursor.Current = previous;
            if (result != 0)
            {
                error = "The video " + Path.GetFileName(filename) + " can't be converted:\n" + error;
                try
                {
                    File.Delete(converted);
                }
                catch (Exception)
                {
                }
                return null;
            }
            return converted;
        }

        //! next to the original, or in the local application data where that folder can't be written
        static string convertedPath(string filename)
        {
            string converted = Path.ChangeExtension(filename, ".ogv");
            try
            {
                string test = converted + ".tmp";
                using (new FileStream(test, FileMode.Create, FileAccess.Write))
                {
                }
                File.Delete(test);
                return converted;
            }
            catch (Exception)
            {
                string dir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "StoryDesigner", "videos");
                Directory.CreateDirectory(dir);
                return Path.Combine(dir, Path.GetFileName(converted));
            }
        }

        static int run(string arguments, out string output, out string error)
        {
            ProcessStartInfo info = new ProcessStartInfo(ConverterPath, arguments);
            info.UseShellExecute = false;
            info.CreateNoWindow = true;
            info.RedirectStandardOutput = true;
            info.RedirectStandardError = true;
            using (Process process = Process.Start(info))
            {
                //both pipes at once, so neither fills up
                var errorTask = process.StandardError.ReadToEndAsync();
                output = process.StandardOutput.ReadToEnd();
                error = errorTask.Result;
                process.WaitForExit();
                return process.ExitCode;
            }
        }
    }
}
