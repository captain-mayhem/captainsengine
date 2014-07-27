using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Security.Permissions;

namespace StoryDesigner
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        [SecurityPermission(SecurityAction.Demand, Flags= SecurityPermissionFlag.ControlAppDomain)]
        static void Main(string[] args)
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
            Update update = new Update(main);
            update.doUpdate();
            Application.Run(main);
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