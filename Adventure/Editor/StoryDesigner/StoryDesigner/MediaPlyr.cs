﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace StoryDesigner
{
    public partial class MediaPlyr : Form
    {
        public MediaPlyr(string file)
        {
            InitializeComponent();
            this.Paint += new PaintEventHandler(MediaPlyr_Paint);
#if !MONO
            mPlayer = new WMPLib.WindowsMediaPlayer();
            mPlayer.URL = file;
#endif
            mName = Path.GetFileName(file);
        }

        void MediaPlyr_Paint(object sender, PaintEventArgs e)
        {
            Brush b = Brushes.White;
            e.Graphics.DrawString(mName, this.Font, b, new PointF());
        }

        private void play_Click(object sender, EventArgs e)
        {
#if !MONO
            mPlayer.controls.play();
#endif
        }

#if !MONO
        WMPLib.WindowsMediaPlayer mPlayer;
#endif
        string mName;

        private void stop_Click(object sender, EventArgs e)
        {
#if !MONO
            mPlayer.controls.stop();
#endif
        }
    }
}