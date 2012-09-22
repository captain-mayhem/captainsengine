using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class FxShapeCtrl : UserControl
    {
        public FxShapeCtrl()
        {
            InitializeComponent();
            position.Paint += new PaintEventHandler(position_Paint);
            System.IO.Stream strm = System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream("StoryDesigner.Images.FxshapeManDark.png");
            mDarkMan = new Bitmap(strm);
            strm = System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream("StoryDesigner.Images.FxshapeManLight.png");
            mLightMan = new Bitmap(strm);
        }

        void position_Paint(object sender, PaintEventArgs e)
        {
            e.Graphics.DrawImage(mLightMan, new Point(28, 20));
            e.Graphics.DrawImage(mDarkMan, new Point(32, 27));
        }

        public void setParent(RoomCtrlDlg dlg)
        {
            mParent = dlg;
        }

        public void setShape(FxShape shape)
        {
            mShape = shape;
            activate.Checked = shape.Active;
            switch (mShape.Effect)
            {
                case FxShape.FxEffect.FLOOR_MIRROR:
                    effect.SelectedIndex = 0;
                    break;
                case FxShape.FxEffect.WALL_MIRROR:
                    effect.SelectedIndex = 1;
                    break;
                case FxShape.FxEffect.PARTICLE_BARRIER:
                    effect.SelectedIndex = 2;
                    break;
            }
            strength.Value = shape.Strength;
            depOnRP.Checked = shape.DependingOnRoomPosition;
        }

        private FxShape mShape;
        private RoomCtrlDlg mParent;
        private Bitmap mDarkMan;
        private Bitmap mLightMan;

        private void activate_CheckedChanged(object sender, EventArgs e)
        {
            mShape.Active = activate.Checked;
            mParent.redrawRoom();
        }

        private void effect_SelectedIndexChanged(object sender, EventArgs e)
        {
            switch (effect.SelectedIndex)
            {
                case 0:
                    mShape.Effect = FxShape.FxEffect.FLOOR_MIRROR;
                    break;
                case 1:
                    mShape.Effect = FxShape.FxEffect.WALL_MIRROR;
                    break;
                case 2:
                    mShape.Effect = FxShape.FxEffect.PARTICLE_BARRIER;
                    break;
            }
            mParent.redrawRoom();
        }

        private void strength_Scroll(object sender, EventArgs e)
        {
            mShape.Strength = strength.Value;
        }

        private void depOnRP_CheckedChanged(object sender, EventArgs e)
        {
            mShape.DependingOnRoomPosition = depOnRP.Checked;
        }
    }
}
