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
            ArrayList offspeech = orig.getWords(Language.Section.Offspeech);
            ArrayList offspeechsounds = orig.getWords(Language.Section.Offspeech_Sounds);
            for (int i = 0; i < offspeech.Count; ++i)
            {
                string origtxt = (string)offspeech[i];
                string origvoice = (string)offspeechsounds[i];
                offspeechGrid.Rows.Add(origvoice, origtxt);
            }
            ArrayList showinfo = orig.getWords(Language.Section.Showinfo);
            foreach (string str in showinfo)
            {
                showinfoGrid.Rows.Add("", str);
            }
            ArrayList textout = orig.getWords(Language.Section.Textout);
            foreach (string str in textout)
            {
                textoutGrid.Rows.Add("", str);
            }
            ArrayList setstring = orig.getWords(Language.Section.Setstring);
            foreach (string str in setstring)
            {
                setstringGrid.Rows.Add("", str);
            }
            ArrayList textscenes = orig.getWords(Language.Section.Textscenes);
            foreach (string str in textscenes)
            {
                textsceneGrid.Rows.Add("", str);
            }
            ArrayList commands = orig.getWords(Language.Section.Commands);
            foreach (string str in commands)
            {
                commandsGrid.Rows.Add("", str);
            }
        }

        AdvData mData;
    }
}
