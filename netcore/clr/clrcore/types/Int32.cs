namespace Morph
{
    // Code taken form Mono 2.10.7, be aware code modified for our needs.
    public struct Int32
    {

        public const int MaxValue = 0x7fffffff;
        public const int MinValue = -2147483648;

        private const uint absMaxValue = 2147483648;

        internal int m_value;

        public override int GetHashCode()
        {
            return m_value;
        }

        public int CompareTo(System.Object obj)
        {
            if (obj == null)
            {
                return 1;
            }

            if (!(obj is System.Int32))
            {
                throw new System.ArgumentException("Value is not a System.Int32");
            }

            return this.CompareTo((int)obj);
        }

        public int CompareTo(int value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is System.Int32))
                return false;

            return this.Equals((int)obj);
        }

        public bool Equals(int obj)
        {
            return obj == m_value;
        }

        public unsafe override string ToString()
        {
            uint i;
            if (m_value < 0)
            {
                i = (uint)(-m_value);
                return System.String.Concat("-", i.ToString());
            }
            i = (uint)(m_value);
            return i.ToString();
        }


        //public static int Parse(string s)
        //{
        //    int result = 0, i, len = s.Length, sign = 1;
        //    char c;

        //    //if (s == null)
        //    //    throw new ArgumentNullException();

        //    if (s[0] == '-')
        //    {
        //        //if (len > 11)
        //        //    throw new Exception("Too big number");
        //        i = 1;
        //        sign = -1;
        //    }
        //    else
        //    {
        //        //if (len > 10)
        //        //    throw new Exception("Too big number");
        //        i = 0;
        //    }

        //    for (; i < len; i++)
        //    {
        //        c = s[i];

        //        if (c >= '0' && c <= '9')
        //        {
        //            byte d = (byte)(c - '0');
        //            //if (result > (int.MaxValue / 10))
        //            //    throw new Exception("Too big number");

        //            if (result == (int.MaxValue / 10))
        //            {
        //                // if ((d > (int.MaxValue % 10)) && (sign == 1 || (d > ((int.MaxValue % 10) + 1))))
        //                //     throw new Exception("Too big number");

        //                if (sign == -1)
        //                    result = (result * sign * 10) - d;
        //                else
        //                    result = (result * 10) + d;
        //            }
        //            else
        //                result = result * 10 + d;
        //        }
        //        else
        //            ;//throw new Exception("type not match");
        //    }

        //    return result;
        //}

        //public static bool TryParse(string s)
        //{
        //    int result = 0, i, len = s.Length, sign = 1;
        //    char c;

        //    if (s == null)
        //        return false;

        //    if (s[0] == '-')
        //    {
        //        if (len > 11)
        //            return false;
        //        i = 1;
        //        sign = -1;
        //    }
        //    else
        //    {
        //        if (len > 10)
        //            return false;
        //        i = 0;
        //    }

        //    for (; i < len; i++)
        //    {
        //        c = s[i];

        //        if (c >= '0' && c <= '9')
        //        {
        //            byte d = (byte)(c - '0');
        //            if (result > (int.MaxValue / 10))
        //                return false;

        //            if (result == (int.MaxValue / 10))
        //            {
        //                if ((d > (int.MaxValue % 10)) && (sign == 1 || (d > ((int.MaxValue % 10) + 1))))
        //                    return false;

        //                if (sign == -1)
        //                    result = (result * sign * 10) - d;
        //                else
        //                    result = (result * 10) + d;
        //            }
        //            else
        //                result = result * 10 + d;
        //        }
        //        else
        //            return false;
        //    }

        //    return true;
        //}

        public static int Parse(string s)
        {
            uint temp;

            if (s == null)
                throw new System.ArgumentNullException();
            if (s.Length <= 0)
                throw new System.FormatException();

            if (s[0] == '-')
            {
                temp = uint.Parse(s.Substring(1));
                if (temp > absMaxValue)
                {
                    throw new System.OverflowException("Value is too large");
                }

                return -((int)temp);
            }

            temp = uint.Parse(s);
            if (temp > MaxValue)
            {
                throw new System.OverflowException("Value is too large");
            }

            return (int)temp;
        }

        public static bool TryParse(string s, out int result)
        {
            result = 0;


            if (s == null)
                return false;

            if (s.Length <= 0)
                return false;

            uint temp = 0;

            if (s[0] == '-')
            {
                if (!uint.TryParse(s.Substring(1), out temp))
                    return false;

                if (temp > absMaxValue)
                    return false;

                result = -((int)temp);
                return true;
            }

            if(!uint.TryParse(s, out temp))
                return false;

            if (temp > MaxValue)
                return false;

            result = (int)temp;
            return true;
        }
    }
}
