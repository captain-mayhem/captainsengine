using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Collections;

namespace StoryDesigner
{
    public partial class LanguageDlg : Form
    {
        public LanguageDlg(AdvData data)
        {
            InitializeComponent();
            mData = data;
            Language orig = mData.Languages["origin"];
            ArrayList speech = orig.getWords(Language.Section.Speech);
            ArrayList speechsounds = orig.getWords(Language.Section.Speech_Sounds);
            for (int i = 0; i < speech.Count; ++i){
                string origtxt = (string)speech[i];
                string origvoice = (string)speechsounds[i];
                speechGrid.Rows.Add(origvoice, origtxt);
            }
        }

        AdvData mData;
    }
}
