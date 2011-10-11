using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class RoomCtrlDlg : Form
    {
        public RoomCtrlDlg(Room room, AdvData data)
        {
            InitializeComponent();
            mRoom = room;
            for (int i = 1; i <= 9; ++i)
            {
                zoomfactor.Items.Add(i);
            }
            for (int i = 0; i <= 10; ++i)
            {
                obj_state.Items.Add(i);
            }
            zoomfactor.SelectedItem = mRoom.Zoom;
            doublewalkmap.Checked = mRoom.DoubleWalkmap;
            roomwidth.Maximum = data.Settings.Resolution.x * 3;
            roomheight.Maximum = data.Settings.Resolution.y * 2;
            roomwidth.Value = mRoom.Size.x;
            roomheight.Value = mRoom.Size.y;
        }

        public DrawableObject SelectedObject
        {
            get { return mObject; }
            set {
                mObject = value;
                objectgroup.Enabled = false;
                charactergroup.Enabled = false;
                if (mObject == null)
                    return;
                if (mObject is ObjectInstance)
                {
                    objectgroup.Enabled = true;
                    ObjectInstance obj = (ObjectInstance)mObject;
                    obj_instname.Text = obj.Name;
                    obj_locked.Checked = obj.Locked;
                    obj_state.SelectedIndex = obj.State;
                    switch (obj.Layer)
                    {
                        case 0:
                            draw_back.Checked = true;
                            break;
                        case 1:
                            draw_middle.Checked = true;
                            break;
                        case 2:
                            draw_front.Checked = true;
                            break;
                    }
                }
                else if (mObject is CharacterInstance)
                {
                    charactergroup.Enabled = true;
                    CharacterInstance chr = (CharacterInstance)mObject;
                    chr_instancename.Text = chr.Name;
                    chr_locked.Checked = chr.Locked;
                    chr_unmovable.Checked = chr.Unmovable;
                    switch (chr.LookDir)
                    {
                        case 1:
                            look_down.Checked = true;
                            break;
                        case 2:
                            look_up.Checked = true;
                            break;
                        case 3:
                            look_right.Checked = true;
                            break;
                        case 4:
                            look_left.Checked = true;
                            break;
                    }
                }
            }
        }

        private Room mRoom;
        private DrawableObject mObject;
    }
}