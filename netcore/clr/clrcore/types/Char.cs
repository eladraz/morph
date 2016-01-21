
namespace Morph
{
    public struct Char
    {
        public const char MaxValue = (char)0xff; //or 0xffff
        public const char MinValue = (char)0;

        internal char m_value;

        public static bool IsWhiteSpace(System.Char c)
        {
            byte b = (byte)c;

            return (b >= 9 && b <= 13) || b == 32 || b == 133 || b == 160;
        }

        public static bool IsWhiteSpace(string s, int index)
        {
            CheckParameter(s, index);
            return IsWhiteSpace(s[index]);
        }


        public int CompareTo(System.Object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.Char))
            {
                throw new System.ArgumentException("Value is not a System.Char");
            }

            return CompareTo((char)value);
        }

        public int CompareTo(char value)
        {
            if (m_value == value)
                return 0;

            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public override bool Equals(System.Object obj)
        {
            if (!(obj is System.Char))
                return false;

            return Equals((char)obj);
        }

        public bool Equals(char obj)
        {
            return m_value == obj;
        }

        public override int GetHashCode()
        {
            return m_value;
        }

        public override string ToString()
        {
            return new string(m_value, 1);
        }

        public static string ToString(char c)
        {
            return new string(c, 1);
        }

        public static char Parse(string s)
        {
            if (s == null)
                throw new System.ArgumentNullException("s");

            if (s.Length != 1)
                throw new System.FormatException("s contains more than one character.");

            return s[0];
        }

        public static bool TryParse(string s, out char result)
        {
            if (s == null || s.Length != 1)
            {
                result = (char)0;
                return false;
            }

            result = s[0];
            return true;
        }

        public static char ToUpper(char c)
        {
            if (c >= 'a' && c <= 'z')
                return (char)(c + ('A' - 'a'));

            return c;
        }

        public static char ToLower(char c)
        {
            if (c >= 'A' && c <= 'Z')
                return (char)(c - ('A' - 'a'));

            return c;
        }

        public static bool IsUpper(char c)
        {
            return (c >= 'A' && c <= 'Z');
        }

        public static bool IsUpper(string s, int index)
        {
            CheckParameter(s, index);
            return IsUpper(s[index]);
        }

        public static bool IsNumber(char c)
        {
            return IsDigit(c);
        }

        public static bool IsNumber(string s, int index)
        {
            CheckParameter(s, index);
            return IsNumber(s[index]);
        }

        public static bool IsLower(char c)
        {
            return (c >= 'a' && c <= 'z');
        }

        public static bool IsLower(string s, int index)
        {
            CheckParameter(s, index);
            return IsLower(s[index]);
        }

        public static bool IsLetter(char c)
        {
            return (IsUpper(c) || IsLower(c));
        }

        public static bool IsLetter(string s, int index)
        {
            CheckParameter(s, index);
            return IsLetter(s[index]);
        }

        public static bool IsDigit(char c)
        {
            return (c >= '0' && c <= '9');
        }

        public static bool IsDigit(string s, int index)
        {
            CheckParameter(s, index);
            return IsDigit(s[index]);
        }

        public static bool IsLetterOrDigit(char c)
        {
            return (IsLetter(c) || IsDigit(c));
        }

        public static bool IsLetterOrDigit(string s, int index)
        {
            CheckParameter(s, index);
            return IsLetterOrDigit(s[index]);
        }

        public static bool IsControl(char c)
        {
            byte b = (byte)c;

            return ((b >= 0 && b <= 31) || (b >= 127 && b <= 159) );        //0->31 and 127->159
        }

        public static bool IsControl(string s, int index)
        {
            CheckParameter(s, index);
            return IsControl(s[index]);
        }

        public static bool IsPunctuation(char c)
        {
            byte b = (byte)c;

            return ((b >= 33 && b <= 47) || (b >= 58 && b <= 64) || (b >= 91 && b <= 96) || (b >= 123 && b <= 126));
        }

        public static bool IsPunctuation(string s, int index)
        {
            CheckParameter(s, index);
            return IsPunctuation(s[index]);
        }

        public static bool IsSeparator(char c)
        {
            byte b = (byte)c;

            return (b == 32 || b == 160);
        }

        public static bool IsSeparator(string s, int index)
        {
            CheckParameter(s, index);
            return IsSeparator(s[index]);
        }

        //public static int GetNumericValue(char c)
        //{
        //    if (IsDigit(c))
        //        return (c - '0');

        //    return -1;
        //}


        //public static int GetNumericValue(string s, int index)
        //{
        //    CheckParameter(s, index);
        //    return GetNumericValue(s[index]);
        //}

        private static void CheckParameter(string s, int index)
        {
            if (s == null)
                throw new System.ArgumentNullException("s");

            if (index < 0 || index >= s.Length)
                throw new System.ArgumentOutOfRangeException("The value of index is less than zero, or greater than or equal to the length of s.");
        }



    }
}
