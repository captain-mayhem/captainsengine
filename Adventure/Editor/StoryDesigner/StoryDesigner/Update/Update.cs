﻿using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace StoryDesigner
{
    public class Update
    {
        private Updatable applicationInfo;
        private BackgroundWorker bgWorker;

        public Update(Updatable applicationInfo)
        {
            this.applicationInfo = applicationInfo;

            this.bgWorker = new BackgroundWorker();
            bgWorker.DoWork += new DoWorkEventHandler(bgWorker_DoWork);
            bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bgWorker_RunWorkerCompleted);
        }

        public void doUpdate()
        {
            if (!bgWorker.IsBusy)
                bgWorker.RunWorkerAsync(applicationInfo);
        }

        private void bgWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!e.Cancelled)
            {
                UpdateXml update = (UpdateXml)e.Result;
                if (update != null && update.isNewerThan(applicationInfo.ApplicationAssembly.GetName().Version))
                {
                    if (new UpdateAccept(applicationInfo, update).ShowDialog(applicationInfo.Context) == DialogResult.Yes)
                    {
                        downloadUpdate(update);
                    }
                }
            }
        }

        private void bgWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            Updatable application = (Updatable)e.Argument;
            if (!UpdateXml.existsOnServer(application.UpdateXmlLocation))
                e.Cancel = true;
            else
                e.Result = UpdateXml.parse(application.UpdateXmlLocation, application.ApplicationID);
        }

        private void downloadUpdate(UpdateXml update)
        {
            UpdateDownloader form = new UpdateDownloader(update.Uri, update.MD5, applicationInfo.ApplicationIcon);
            DialogResult result = form.ShowDialog(applicationInfo.Context);

            if (result == DialogResult.OK)
            {
                string currentPath = applicationInfo.ApplicationAssembly.Location;
                string newPath = Path.Combine(Path.GetDirectoryName(currentPath), update.FileName);
                updateApplication(form.TempFilePath, currentPath, newPath, update.LaunchArgs);
                Application.Exit();
            }
            else if (result == DialogResult.Abort)
            {
                MessageBox.Show("The update download was cancelled.\nThis program has not been modified.", "Update Download Cancelled", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                MessageBox.Show("There was a problem downloading the update.\nPlease try again later.", "Update Download Error", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void updateApplication(string tempFilePath, string currentPath, string newPath, string launchArgs)
        {
            string argument = "/C Choice /C Y /N /D Y /T 4 & Del /f /Q \"{0}\" & /C Choice /C Y /N /D Y /T 2 & Move /Y \"{1}\" \"{2}\" & Start \"\" /D \"{3}\" \"{4}\" \"{5}\"";
            ProcessStartInfo info = new ProcessStartInfo();
            info.Arguments = string.Format(argument, currentPath, tempFilePath, newPath, Path.GetDirectoryName(newPath),
                Path.GetFileName(newPath), launchArgs);
            info.WindowStyle = ProcessWindowStyle.Hidden;
            info.CreateNoWindow = true;
            info.FileName = "cmd.exe";
            Process.Start(info);
        }

    }
}
