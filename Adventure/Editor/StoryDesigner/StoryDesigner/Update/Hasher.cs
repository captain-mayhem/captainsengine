using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace StoryDesigner
{
    internal enum HashType
    {
        MD5,
        SHA1,
        SHA512,
    }

    internal static class Hasher
    {
        internal static string HashFile(string filePath, HashType algo)
        {
            FileStream fs = new FileStream(filePath, FileMode.Open);
            string ret = "";
            switch (algo)
            {
                case HashType.MD5:
                    ret = makeHashString(MD5.Create().ComputeHash(fs));
                    break;
                case HashType.SHA1:
                    ret = makeHashString(SHA1.Create().ComputeHash(fs));
                    break;
                case HashType.SHA512:
                    ret = makeHashString(SHA512.Create().ComputeHash(fs));
                    break;
                default:
                    break;
            }
            fs.Close();
            return ret;
        }

        private static string makeHashString(byte[] hash)
        {
            StringBuilder s = new StringBuilder(hash.Length * 2);
            foreach (byte b in hash)
            {
                s.Append(b.ToString("X2").ToLower());
            }
            return s.ToString();
        }
    }
}
