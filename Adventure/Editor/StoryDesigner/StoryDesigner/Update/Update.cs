﻿using System;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace StoryDesigner
{
    public class Update
    {
        private Updatable[] applicationInfos;
        private BackgroundWorker bgWorker;

        public Update(Updatable[] applicationInfos)
        {
            this.applicationInfos = applicationInfos;

            this.bgWorker = new BackgroundWorker();
            bgWorker.DoWork += new DoWorkEventHandler(bgWorker_DoWork);
            bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(bgWorker_RunWorkerCompleted);
        }

        public void doUpdate()
        {
            if (!bgWorker.IsBusy)
                bgWorker.RunWorkerAsync(applicationInfos);
        }

        private void bgWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!e.Cancelled)
            {
                UpdateXml[] updates = (UpdateXml[])e.Result;
                if (updates == null)
                    return;
                bool needsUpdate = false;
                for (int i = 0; i < updates.Length; ++i)
                {
                    UpdateXml update = (UpdateXml)updates[i];
                    if (update != null && update.isNewerThan(applicationInfos[i].ApplicationVersion))
                    {
                        update.ShouldUpdate = true;
                        needsUpdate = true;
                    }
                }
                if (!needsUpdate)
                    return;
                if (new UpdateAccept(applicationInfos, updates).ShowDialog(applicationInfos[0].Context) == DialogResult.Yes)
                {
                    for (int i = 0; i < updates.Length; ++i)
                    {
                        if (updates[i].ShouldUpdate)
                            downloadUpdate(updates[i], applicationInfos[i].ApplicationLocation, applicationInfos[i].UpdateMode);
                    }
                    performUpdate();
                }
            }
        }

        private void bgWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            Updatable[] applications = (Updatable[])e.Argument;
            UpdateXml[] result = new UpdateXml[applications.Length];
            bool oneSucceeded = false;
            for (int i = 0; i < applications.Length; ++i)
            {
                Updatable application = applications[i];
                if (!UpdateXml.existsOnServer(application.UpdateXmlLocation))
                    result[i] = null;
                else
                    result[i] = UpdateXml.parse(application.UpdateXmlLocation, application.ApplicationID);
                if (!oneSucceeded && result[i] != null)
                    oneSucceeded = true;
            }
            if (!oneSucceeded)
                e.Cancel = true;
            e.Result = result;
        }

        private void downloadUpdate(UpdateXml update, string location, UpdateAction action)
        {
            UpdateDownloader form = new UpdateDownloader(update.Uri, update.MD5, applicationInfos[0].ApplicationIcon);
            DialogResult result = form.ShowDialog(applicationInfos[0].Context);
            form.Close();

            if (result == DialogResult.OK)
            {
                string currentPath = location;
                string newPath = Path.Combine(Path.GetDirectoryName(currentPath), update.FileName);
                if (action == UpdateAction.RESTART)
                    updateApplication(form.TempFilePath, currentPath, newPath, update.LaunchArgs);
                else if (action == UpdateAction.FILECOPY)
                    updateFile(form.TempFilePath, currentPath, newPath, update.Version.ToString());
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

        private string tempFilePath;
        private string currentPath;
        private string newPath;
        private string launchArgs;

        private void updateApplication(string tempFilePath, string currentPath, string newPath, string launchArgs)
        {
            this.tempFilePath = tempFilePath;
            this.currentPath = currentPath;
            this.newPath = newPath;
            this.launchArgs = launchArgs;
        }

        private void updateFile(string tempFilePath, string currentPath, string newPath, string version)
        {
            File.Delete(currentPath);
            File.Copy(tempFilePath, newPath);
            string verfile = Path.Combine(Path.GetDirectoryName(newPath), Path.GetFileNameWithoutExtension(newPath)) + ".ver";
            File.WriteAllText(verfile, version);
        }

        private void performUpdate()
        {
            if (tempFilePath == null)
                return;
            string newSD = Path.Combine(Path.GetTempPath(), Path.GetFileName(currentPath));
            File.Copy(currentPath, newSD, true);
            ProcessStartInfo info = new ProcessStartInfo();
            //string argument = "/C Choice /C Y /N /D Y /T 4 & Del /f /Q \"{0}\" & /C Choice /C Y /N /D Y /T 2 & Move /Y \"{1}\" \"{2}\" & Start \"\" /D \"{3}\" \"{4}\" \"{5}\"";          
            //info.Arguments = string.Format(argument, currentPath, tempFilePath, newPath, Path.GetDirectoryName(newPath),
            //    Path.GetFileName(newPath), launchArgs);
            info.Arguments = string.Format("--install {0} {1} {2} {3}", currentPath, tempFilePath, newPath,  launchArgs);
            info.WindowStyle = ProcessWindowStyle.Hidden;
            info.CreateNoWindow = true;
            //info.Verb = "runas";

            //info.FileName = "cmd.exe";
            info.FileName = newSD;
            Process.Start(info);
            Application.Exit();
        }

    }
}
