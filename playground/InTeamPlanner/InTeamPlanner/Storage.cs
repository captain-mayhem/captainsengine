using System;
using System.Collections.Generic;
using System.Text;
using System.Data;

namespace InTeamPlanner
{
    public class User{
        public User(Int64 id, string login, string name, bool showUser)
        {
            mID = id;
            mLogin = login;
            mName = name;
            mShowUser = showUser;
        }
        public string Login
        {
            get { return mLogin; }
        }
        public string Name
        {
            get { return mName; }
        }
        Int64 mID;
        string mLogin;
        string mName;
        bool mShowUser;
    }

    public interface StorageListener
    {
        void userAdded(User usr);
    }

    public class Storage
    {
        public Storage(StorageListener listener)
        {
            mListener = listener;
        }

        public void initialize()
        {
            string dbname = "itp.db";
            mDB = new SQLite();
            bool exists = mDB.open(dbname, false);
            if (!exists)
            {
                mDB.open(dbname, true);
                mDB.executeNonQuery("CREATE TABLE Users(" +
                    "userid INTEGER PRIMARY KEY, " +
                    "login TEXT UNIQUE, " +
                    "name TEXT, " +
                    "showUser BOOLEAN)");
                mDB.executeNonQuery("CREATE TABLE Categories(" +
                    "catid INTEGER PRIMARY KEY, " +
                    "category TEXT, "+
                    "shorthand TEXT, "+
                    "supervisor INTEGER, "+
                    "FOREIGN KEY(supervisor) REFERENCES Users(userid)"+
                    ")");
                mDB.executeNonQuery("CREATE TABLE Tasks(" +
                    "taskid INTEGER PRIMARY KEY, "+
                    "title TEXT,"+
                    "description TEXT,"+
                    "actionPoints TEXT,"+
                    "category INTEGER,"+
                    "duration INTEGER,"+
                    "inserted DATE, " +
                    "modified DATE, " +
                    "state INTEGER, " +
                    "FOREIGN KEY(category) REFERENCES Categories(catid)"+
                    ")");
                mDB.executeNonQuery("CREATE TABLE Schedule(" +
                   "schedid INTEGER PRIMARY KEY, " +
                   "task INTEGER," +
                   "user INTEGER," +
                   "startdate DATE," +
                   "FOREIGN KEY(task) REFERENCES Tasks(taskid)" +
                   "FOREIGN KEY(user) REFERENCES Users(userid)" +
                   ")");
            }
            DataTable users = mDB.executeQuery("SELECT userid, login, name, showUser FROM Users WHERE showUser='true'");
            foreach (DataRow row in users.Rows)
            {
                User usr = new User((int)row[0], (string)row[1], (string)row[2], (bool)row[3]);
                mListener.userAdded(usr);
            }
            if (!exists)
            {
                addUser("mast2639", "Marco", true);
            }
        }

        public User addUser(string login, string name, bool showUser)
        {
            Int64 rowid = mDB.executeInsert("INSERT INTO Users (login, name, showUser) VALUES ("+
                "'"+login+"', '"+name+"', "+(showUser?"'true'":"'false'")+")");
            User usr = new User(rowid, login, name, showUser);
            if (showUser)
                mListener.userAdded(usr);
            return usr;
        }

        public void addCategory(string name)
        {
        }

        SQLite mDB;
        StorageListener mListener;
    }
}
