using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class MDIMain : Form, PluginHost
    {
        public MDIMain()
        {
            InitializeComponent();
            mMainForm = new MainForm();
            mMainForm.MainMenuStrip = this.MainMenuStrip;
            mMainForm.MdiParent = this;
            mMainForm.Show();
        }

        public MDIMain(string filename)
        {
            InitializeComponent();
            mMainForm = new MainForm(filename);
            mMainForm.MainMenuStrip = this.MainMenuStrip;
            mMainForm.MdiParent = this;
            mMainForm.Show();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.openToolStripMenuItem_Click(sender, e);
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.newToolStripMenuItem_Click(sender, e);
        }

        MainForm mMainForm;

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.saveToolStripMenuItem_Click(sender, e);
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.saveAsToolStripMenuItem_Click(sender, e);
        }

        private void languageAndSoundToolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.languageAndSoundToolsToolStripMenuItem_Click(sender, e);
        }

        private void createGameToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.createGameToolStripMenuItem_Click(sender, e);
        }

        private void beendenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void imageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.imageToolStripMenuItem_Click(sender, e);
        }

        private void soundeffectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.soundeffectToolStripMenuItem_Click(sender, e);
        }

        private void musicfileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.musicfileToolStripMenuItem_Click(sender, e);
        }

        private void videoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.videoToolStripMenuItem_Click(sender, e);
        }

        private void roomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.roomToolStripMenuItem_Click(sender, e);
        }

        private void objectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.objectToolStripMenuItem_Click(sender, e);
        }

        private void characterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.characterToolStripMenuItem_Click(sender, e);
        }

        private void itemToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.itemToolStripMenuItem_Click(sender, e);
        }

        private void scriptToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.scriptToolStripMenuItem_Click(sender, e);
        }

        private void commandsAndBoolsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.commandsAndBoolsToolStripMenuItem_Click(sender, e);
        }

        private void projectSetupToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.projectSetupToolStripMenuItem_Click(sender, e);
        }

        private void mouseiconsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.mouseiconsToolStripMenuItem_Click(sender, e);
        }

        private void walkmapOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.walkmapOnToolStripMenuItem_Click(sender, e);
        }

        private void deepmapOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.deepmapOnToolStripMenuItem_Click(sender, e);
        }

        private void inventoryfieldOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.inventoryfieldOnToolStripMenuItem_Click(sender, e);
        }

        private void fXShapesOnToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.fXShapesOnToolStripMenuItem_Click(sender, e);
        }

        private void fontsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            mMainForm.fontsToolStripMenuItem_Click(sender, e);
        }
    }
}
