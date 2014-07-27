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
            switch (algo)
            {
                case HashType.MD5:
                    return makeHashString(MD5.Create().ComputeHash(new FileStream(filePath, FileMode.Open)));
                case HashType.SHA1:
                    return makeHashString(SHA1.Create().ComputeHash(new FileStream(filePath, FileMode.Open)));
                case HashType.SHA512:
                    return makeHashString(SHA512.Create().ComputeHash(new FileStream(filePath, FileMode.Open)));
                default:
                    return "";
            }
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
