using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Reflection;
using System.Collections;

namespace StoryDesigner
{
    class ComponentInfo
    {
        public string Name;
        public string Version;
        public string Url;
        public string ReleaseNotesUrl;
    }

    class UpdateInfo
    {
        public void addComponent(ComponentInfo info)
        {
            mComponents.Add(info);
        }

        ArrayList mComponents = new ArrayList();
    }

    class Update
    {
        public void checkForUpdates()
        {
            UpdateInfo li = getLocalVersion();
            //string url = "http://adventure.captain-online.de/sdupdate.txt";
            //string url = "D:\\tmp\\sdupdate.txt";
            //UpdateInfo ri = getRemoteVersion(url);
        }

        UpdateInfo getLocalVersion()
        {
            UpdateInfo ui = new UpdateInfo();
            ComponentInfo ci = new ComponentInfo();
            ci.Name = "StoryDesigner";
            ci.Version = typeof(MDIMain).Assembly.GetName().Version.ToString();
            ui.addComponent(ci);
            return ui;
        }

        UpdateInfo getRemoteVersion(string url)
        {
            Stream st = openStream(url);
            if (st == null)
                return null;
            UpdateInfo ui = new UpdateInfo();
            StreamReader rdr = new StreamReader(st, Encoding.UTF8);
            while (!rdr.EndOfStream)
            {
                string component = rdr.ReadLine();
                string[] compparts = component.Split(' ');
                ComponentInfo ci = new ComponentInfo();
                ci.Name = compparts[0];
                ci.Version = compparts[1];
                ci.Url = compparts[2];
                ci.ReleaseNotesUrl = compparts[3];
                ui.addComponent(ci);
            }
            st.Close();
            return ui;
        }

        Stream openStream(string url)
        {
            if (url.StartsWith("http://"))
            {
                try
                {
                    HttpWebRequest req = (HttpWebRequest)HttpWebRequest.Create(url);
                    HttpWebResponse resp = (HttpWebResponse)req.GetResponse();
                    Stream recvStream = resp.GetResponseStream();
                    return recvStream;
                }
                catch (Exception)
                {
                    return null;
                }
            }
            else
            {
                try
                {
                    Stream fileStream = new FileStream(url, FileMode.Open);
                    return fileStream;
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }
    }
}
