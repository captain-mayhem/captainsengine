using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Data;

namespace InTeamPlanner
{
    public class SQLiteException : Exception
    {
        public SQLiteException(string message) : base(message) { }
    }

    public class SQLite
    {
        const int SQLITE_OK = 0;
        const int SQLITE_ROW = 100;
        const int SQLITE_DONE = 101;

        const int SQLITE_OPEN_READWRITE = 2;
        const int SQLITE_OPEN_CREATE = 4;

        const int SQLITE_INTEGER = 1;
        const int SQLITE_FLOAT = 2;
        const int SQLITE_TEXT = 3;
        const int SQLITE_BLOB = 4;
        const int SQLITE_NULL = 5;

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_open_v2")]
        static extern int sqlite3_open_v2(string filename, out IntPtr db, int flags, IntPtr zVfs);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_close")]
        static extern int sqlite3_close(IntPtr db);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_prepare_v2")]
        static extern int sqlite3_prepare_v2(IntPtr db, string zSql, int nByte, out IntPtr ppStmt, IntPtr pzTail);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_step")]
        static extern int sqlite3_step(IntPtr stmHandle);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_finalize")]
        static extern int sqlite3_finalize(IntPtr db);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_errmsg")]
        static extern IntPtr sqlite3_errmsg_internal(IntPtr db);
        static string sqlite3_errmsg(IntPtr db)
        {
            IntPtr str = sqlite3_errmsg_internal(db);
            string value = Marshal.PtrToStringAnsi(str);
            return value;
        }

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_count")]
        static extern int sqlite3_column_count(IntPtr stmHandle);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_origin_name")]
        static extern IntPtr sqlite3_column_origin_name_internal(IntPtr stmHandle, int iCol);
        static string sqlite3_column_origin_name(IntPtr stmHandle, int iCol)
        {
            IntPtr str = sqlite3_column_origin_name_internal(stmHandle, iCol);
            string value = Marshal.PtrToStringAnsi(str);
            return value;
        }

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_type")]
        static extern int sqlite3_column_type(IntPtr stmHandle, int iCol);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_int")]
        static extern int sqlite3_column_int(IntPtr stmHandle, int iCol);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_text")]
        static extern IntPtr sqlite3_column_text_internal(IntPtr stmHandle, int iCol);
        static string sqlite3_column_text(IntPtr stmHandle, int iCol)
        {
            IntPtr str = sqlite3_column_text_internal(stmHandle, iCol);
            string value = Marshal.PtrToStringAnsi(str);
            return value;
        }

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_double")]
        static extern double sqlite3_column_double(IntPtr stmHandle, int iCol);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_last_insert_rowid")]
        static extern Int64 sqlite3_last_insert_rowid(IntPtr stmHandle);

        [DllImport("sqlite3.dll", EntryPoint = "sqlite3_column_decltype")]
        static extern IntPtr sqlite3_column_decltype_internal(IntPtr stmHandle, int iCol);
        static string sqlite3_column_decltype(IntPtr stmHandle, int iCol)
        {
            IntPtr str = sqlite3_column_decltype_internal(stmHandle, iCol);
            string value = Marshal.PtrToStringAnsi(str);
            return value;
        }

        private IntPtr mDB;
        private bool mIsOpen;

        public bool open(string path, bool create)
        {
            int flags = SQLITE_OPEN_READWRITE;
            if (create)
                flags |= SQLITE_OPEN_CREATE;
            if (sqlite3_open_v2(path, out mDB, flags, IntPtr.Zero) != SQLITE_OK)
            {
                if (!create)
                    return false;
                throw new SQLiteException("Could not open database file: " + path);
            }
            mIsOpen = true;
            return mIsOpen;
        }

        public void close()
        {
            if (mIsOpen)
                sqlite3_close(mDB);
            mIsOpen = false;
        }

        public void executeNonQuery(string query)
        {
            if (!mIsOpen)
                throw new SQLiteException("SQLite database is not open.");
            IntPtr stmHandle = prepare(query);
            if (sqlite3_step(stmHandle) != SQLITE_DONE)
                throw new SQLiteException("Could not execute SQL statement.");
            finalize(stmHandle);
        }

        public Int64 executeInsert(string query)
        {
            executeNonQuery(query);
            return sqlite3_last_insert_rowid(mDB);
        }

        public DataTable executeQuery(string query)
        {
            if (!mIsOpen)
                throw new SQLiteException("SQLite database is not open.");
            IntPtr stmHandle = prepare(query);

            int numColumns = sqlite3_column_count(stmHandle);

            DataTable dTable = new DataTable();
            for (int i = 0; i < numColumns; ++i)
            {
                dTable.Columns.Add(sqlite3_column_origin_name(stmHandle, i));
                string type = sqlite3_column_decltype(stmHandle, i);
                switch (type)
                {
                    case "INTEGER":
                        dTable.Columns[i].DataType = typeof(int);
                        break;
                    case "BOOLEAN":
                        dTable.Columns[i].DataType = typeof(bool);
                        break;
                    case "TEXT":
                        dTable.Columns[i].DataType = typeof(string);
                        break;
                    case "REAL":
                        dTable.Columns[i].DataType = typeof(float);
                        break;
                    default:
                        throw new SQLiteException("type " + type + " unhandled!");
                }
            }

            while (sqlite3_step(stmHandle) == SQLITE_ROW)
            {
                //object[] row = new object[numColumns];
                DataRow row = dTable.NewRow();
                for (int i = 0; i < numColumns; ++i)
                {
                    switch (sqlite3_column_type(stmHandle, i))
                    {
                        case SQLITE_INTEGER:
                            row[i] = sqlite3_column_int(stmHandle, i);
                            break;
                        case SQLITE_TEXT:
                            row[i] = sqlite3_column_text(stmHandle, i);
                            break;
                        case SQLITE_FLOAT:
                            row[i] = sqlite3_column_double(stmHandle, i);
                            break;
                    }
                }
                dTable.Rows.Add(row);
            }
            finalize(stmHandle);
            return dTable;
        }

        private IntPtr prepare(string query)
        {
            IntPtr stmHandle;
            if (sqlite3_prepare_v2(mDB, query, query.Length, out stmHandle, IntPtr.Zero) != SQLITE_OK)
                throw new SQLiteException(sqlite3_errmsg(mDB));
            return stmHandle;
        }

        private void finalize(IntPtr stmHandle)
        {
            if (sqlite3_finalize(stmHandle) != SQLITE_OK)
                throw new SQLiteException("Could not finalize SQL statement.");
        }
    }
}
