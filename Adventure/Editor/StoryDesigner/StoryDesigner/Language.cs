using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace StoryDesigner
{
    public class Language
    {
        public enum Section
        {
            Speech = 0,
            Speech_Sounds,
            Offspeech,
            Offspeech_Sounds,
            Showinfo,
            Textout,
            Setstring,
            Textscenes,
            Commands,
            NumSections
        }

        public Language()
        {
            for (int i = 0; i < (int)Section.NumSections; ++i)
            {
                mSections[i] = new ArrayList();
            }
        }

        public string Name;

        public void addWordWithoutIndex(Section section, string text)
        {
            int idx = (int)section;
            mSections[idx].Add(text);
        }

        public void addWord(Section section, string text, string voice)
        {
            int sect = (int)section;
            int sect2 = -1;
            if (section == Section.Speech)
                sect2 = (int)Section.Speech_Sounds;
            else if (section == Section.Offspeech)
                sect2 = (int)Section.Offspeech_Sounds;
            for (int i = 0; i < mSections[sect].Count; ++i){
                string check = (string)mSections[sect][i];
                if (check == text)
                {
                    if (sect2 != -1)
                    {
                        string check_voice = (string)mSections[sect2][i];
                        if (check_voice == voice)
                        {
                            mEncountered[sect][i] = true;
                            mEncountered[sect2][i] = true;
                            return;
                        }
                    }
                    else
                    {
                        mEncountered[sect][i] = true;
                        return;
                    }
                }
            }
            mSections[sect].Add(text);
            mEncountered[sect].Add(true);
            if (sect2 != -1)
            {
                mSections[sect2].Add(voice);
                mEncountered[sect2].Add(true);
            }
        }

        public void buildIndex()
        {
            for (int i = 0; i < (int)Language.Section.NumSections; ++i)
            {
                ArrayList list = mSections[i];
                mEncountered[i] = new ArrayList(mSections[i].Count);
                for (int j = 0; j < list.Count; ++j)
                {
                    mEncountered[i].Add(false);
                }
            }
        }

        public ArrayList getWords(Section section)
        {
            return mSections[(int)section];
        }

        public static void beginRefresh()
        {
            for (int i = 0; i < (int)Language.Section.NumSections; ++i)
            {
                for (int j = 0; j < mEncountered[i].Count; ++j )
                {
                    mEncountered[i][j] = false;
                }
            }
        }

        public static void endRefresh(AdvData data)
        {
            for (int i = 0; i < (int)Language.Section.NumSections; ++i)
            {
                ArrayList encountersToRemove = new ArrayList();
                for (int j = 0; j < mEncountered[i].Count; ++j)
                {
                    if (!(bool)mEncountered[i][j])
                    {
                        encountersToRemove.Add(j);
                    }
                }
                for (int c = encountersToRemove.Count-1; c >= 0; --c)
                {
                    int removeMe = (int)encountersToRemove[c];
                    mEncountered[i].RemoveRange(removeMe, 1);
                    foreach (KeyValuePair<string,Language> langpair in data.Languages)
                    {
                        Language lang = langpair.Value;
                        lang.mSections[i].RemoveRange(removeMe, 1);
                    }
                }
            }
        }

        ArrayList[] mSections = new ArrayList[(int)Section.NumSections];
        static ArrayList[] mEncountered = new ArrayList[(int)Section.NumSections];
    }
}
