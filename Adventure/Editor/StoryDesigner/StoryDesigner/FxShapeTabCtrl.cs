using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace StoryDesigner
{
    public partial class FxShapeTabCtrl : UserControl
    {
        public FxShapeTabCtrl(System.Collections.ArrayList shapes, RoomCtrlDlg parent)
        {
            InitializeComponent();
            this.fxShapeCtrl1.setParent(parent);
            this.fxShapeCtrl1.setShape((FxShape)shapes[0]);
            this.fxShapeCtrl2.setParent(parent);
            this.fxShapeCtrl2.setShape((FxShape)shapes[1]);
            this.fxShapeCtrl3.setParent(parent);
            this.fxShapeCtrl3.setShape((FxShape)shapes[2]);
        }
    }
}
