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
    }
}
