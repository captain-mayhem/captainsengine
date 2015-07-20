using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Globalization;
//using System.Data.SQLite;

namespace InTeamPlanner
{
    public partial class MainForm : Form, StorageListener
    {
        public MainForm()
        {
            InitializeComponent();
            grid.MouseClick += new MouseEventHandler(grid_MouseClick);
            grid.CellValueChanged += new DataGridViewCellEventHandler(grid_CellValueChanged);
            DateTime now = DateTime.Today;
            Calendar cal = CultureInfo.CurrentCulture.Calendar;
            mWeek = cal.GetWeekOfYear(now, CalendarWeekRule.FirstDay, DayOfWeek.Monday);
            mYear = now.Year - 2000;
            addWeek(mWeek - 1, mYear);
            addWeek(mWeek, mYear);
            addWeek(mWeek + 1, mYear);
            mStorage = new Storage(this);
            mStorage.initialize();
        }

        void grid_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            if (grid.Rows[e.RowIndex+1].HeaderCell.Tag != null)
            {
                DataGridViewRow row = new DataGridViewRow();
                grid.Rows.Insert(e.RowIndex+1, row);
            }
        }

        void grid_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                Point p = grid.PointToScreen(new Point(e.X, e.Y));
                tableRight.Show(p);
            }
        }

        void addWeek(int week, int year)
        {
            for (int i = 0; i < 5; ++i){
                DataGridViewColumn newColl = new DataGridViewColumn();
                DataGridViewTextBoxCell weekHeader = new DataGridViewTextBoxCell();
                newColl.CellTemplate = weekHeader;
                string day = CultureInfo.CurrentCulture.DateTimeFormat.DayNames[i+1];
                newColl.HeaderText = day+" CW "+week + "/" + year;
                grid.Columns.Add(newColl);
            }
        }

        public void userAdded(User usr)
        {
            DataGridViewRow row = new DataGridViewRow();
            row.HeaderCell.Value = usr.Name;
            row.HeaderCell.Tag = usr;
            grid.Rows.Add(row);
        }

        private void quitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        int mWeek;
        int mYear;
        Storage mStorage;

        private void manageUsersToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Users usrs = new Users(mStorage);
            usrs.ShowDialog();
        }
    }
}