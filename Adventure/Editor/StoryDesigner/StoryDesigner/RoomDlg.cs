using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class RoomDlg : Form
    {
        public RoomDlg(Room room, AdvData data)
        {
            InitializeComponent();
            mRoom = room;
            mData = data;
            this.Text = "Room (" + room.Name + ")";
            this.ClientSize = new Size(data.Settings.Resolution.x, data.Settings.Resolution.y);
            this.Paint += new PaintEventHandler(RoomDlg_Paint);
        }

        void RoomDlg_Paint(object sender, PaintEventArgs e)
        {
            if (mRoom.ParallaxBackground.Length > 0)
            {
                Bitmap bmp = mData.getImage(mRoom.ParallaxBackground);
                e.Graphics.DrawImage(bmp, 0, 0, bmp.Width, bmp.Height);
            }
            if (mRoom.Background.Length > 0)
            {
                Bitmap bmp = mData.getImage(mRoom.Background);
                e.Graphics.DrawImage(bmp, 0, 0, bmp.Width, bmp.Height);
            }
        }

        private Room mRoom;
        private AdvData mData;
    }
}