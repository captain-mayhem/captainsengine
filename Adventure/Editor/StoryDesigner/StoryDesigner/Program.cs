using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Security.Permissions;
using System.Threading;
using System.IO;
using System.Diagnostics;

namespace StoryDesigner
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        [SecurityPermission(SecurityAction.Demand, Flags = SecurityPermissionFlag.ControlAppDomain)]
        static void Main(string[] args)
        {
            if (args.Length > 5 && args[0] == "--install")
            {
                Thread.Sleep(4000);
                File.Delete(args[1]);
                Thread.Sleep(2000);
                File.Move(args[2], args[3]);
                ProcessStartInfo info = new ProcessStartInfo();
                if (args.Length > 6)
                    info.Arguments = args[6];
                info.WorkingDirectory = args[4];
                info.FileName = args[5];
                Process.Start(info);
                Application.Exit();
            }
            else
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
                Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
                AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
                Persistence pers = Persistence.load();
                System.Threading.Thread.CurrentThread.CurrentUICulture = new System.Globalization.CultureInfo(pers.LCID);
                MDIMain main = null;
                if (args.Length > 0)
                    main = new MDIMain(args[0], pers);
                else
                    main = new MDIMain(pers);
                main.checkForUpdates();
                Application.Run(main);
            }
        }

        static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            MessageBox.Show(((Exception)e.ExceptionObject).Message + "\n" + ((Exception)e.ExceptionObject).StackTrace, e.ToString());
        }

        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e)
        {
            MessageBox.Show(e.Exception.Message + "\n" + e.Exception.StackTrace, e.ToString());
        }
    }
}