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
            position.MouseDown += new MouseEventHandler(position_MouseDown);
            position.MouseMove += new MouseEventHandler(position_MouseMove);
            position.MouseUp += new MouseEventHandler(position_MouseUp);
            System.IO.Stream strm = System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream("StoryDesigner.Images.FxshapeManDark.png");
            mDarkMan = new Bitmap(strm);
            strm = System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream("StoryDesigner.Images.FxshapeManLight.png");
            mLightMan = new Bitmap(strm);
        }

        void position_MouseUp(object sender, MouseEventArgs e)
        {
            mDragOffset.x = -1;
        }

        const int lmXPos = 26;
        const int lmYPos = 27;

        void position_MouseMove(object sender, MouseEventArgs e)
        {
            if (mDragOffset.x == -1)
                return;
            int posx = e.X - mDragOffset.x;
            int posy = e.Y - mDragOffset.y;
            if (posy < -100)
                posy = -100;
            if (posy > 100)
                posy = 100;
            if (posx < -100)
                posx = -100;
            if (posx > 100)
                posx = 100;
            if (!mShape.DependingOnRoomPosition)
                mShape.MirrorOffset.x = posx;
            mShape.MirrorOffset.y = posy;
            position.Invalidate();
        }

        void position_MouseDown(object sender, MouseEventArgs e)
        {
            mDragOffset.x = e.X - mShape.MirrorOffset.x;
            mDragOffset.y = e.Y - mShape.MirrorOffset.y;
        }

        void position_Paint(object sender, PaintEventArgs e)
        {
            if (mShape.Effect != FxShape.FxEffect.WALL_MIRROR)
                return;
            Vec2i offset = mShape.MirrorOffset/2;
            if (mShape.DependingOnRoomPosition)
                offset.x = 0;
            e.Graphics.DrawImage(mLightMan, new Point(lmXPos+offset.x, lmYPos+offset.y));
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
            setActive(mShape.Active);
            strength.Value = shape.Strength;
            depOnRP.Checked = shape.DependingOnRoomPosition;
            if (mShape.Active)
                setEffect(mShape.Effect);
        }

        private void setEffect(FxShape.FxEffect effect)
        {
            if (effect == FxShape.FxEffect.WALL_MIRROR)
            {
                position.Enabled = true;
                depOnRP.Enabled = true;
            }
            else
            {
                position.Enabled = false;
                depOnRP.Enabled = false;
            }
            if (effect == FxShape.FxEffect.PARTICLE_BARRIER)
            {
                strength.Enabled = false;
            }
            else
            {
                strength.Enabled = true;
            }
        }

        private void setActive(bool active)
        {
            position.Enabled = active;
            depOnRP.Enabled = active;
            strength.Enabled = active;
            effect.Enabled = active;
        }

        private FxShape mShape;
        private RoomCtrlDlg mParent;
        private Bitmap mDarkMan;
        private Bitmap mLightMan;
        private Vec2i mDragOffset = new Vec2i(-1, -1);

        private void activate_CheckedChanged(object sender, EventArgs e)
        {
            mShape.Active = activate.Checked;
            setActive(mShape.Active);
            if (mShape.Active)
                setEffect(mShape.Effect);
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
            setEffect(mShape.Effect);
            mParent.redrawRoom();
        }

        private void strength_Scroll(object sender, EventArgs e)
        {
            mShape.Strength = strength.Value;
        }

        private void depOnRP_CheckedChanged(object sender, EventArgs e)
        {
            mShape.DependingOnRoomPosition = depOnRP.Checked;
            position.Invalidate();
        }
    }
}
