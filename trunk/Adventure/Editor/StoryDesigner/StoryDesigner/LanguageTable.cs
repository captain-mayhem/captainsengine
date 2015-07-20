using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class LanguageTable : UserControl
    {
        public LanguageTable()
        {
            InitializeComponent();
        }

        public DataGridViewRowCollection Rows
        {
            get { return grid.Rows; }
        }

        public bool VoiceAndText
        {
            set
            {
                mVoiceAndText = value;
                grid.Columns[0].Visible = mVoiceAndText;
                grid.Columns[1].Width = mVoiceAndText ? 305 : 405;
                grid.Columns[3].Visible = mVoiceAndText;
                grid.Columns[4].Width = mVoiceAndText ? 305 : 405;
            }
            get { return mVoiceAndText; }
        }

        bool mVoiceAndText = true;
    }
}
