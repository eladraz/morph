using System;
using System.Collections.Generic;
using System.Text;

namespace Morph
{
    public struct Int64
    {
        public const long MaxValue = 0x7fffffffffffffff;
        public const long MinValue = -9223372036854775808;

        private const ulong absMaxValue = 9223372036854775808;

        internal long m_value;

        public int CompareTo(object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.Int64))
            {
                throw new System.ArgumentException("Value is not a System.Int64.");
            }

            return this.CompareTo((long)value);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is System.Int64))
                return false;

            return this.Equals((long)obj);
        }

        public override int GetHashCode()
        {
            return (int)(m_value & 0xffffffff) ^ (int)(m_value >> 32);
        }

        public int CompareTo(long value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public bool Equals(long obj)
        {
            return obj == m_value;
        }

        public static long Parse(string s)
        {
            ulong temp;

            if (s[0] == '-')
            {
                temp = ulong.Parse(s.Substring(1));
                if (temp > absMaxValue)
                {
                    throw new System.OverflowException("Value is too large");
                }

                return (long)temp;
            }

            temp = ulong.Parse(s);
            if (temp > MaxValue)
            {
                throw new System.OverflowException("Value is too large");
            }

            return (long)temp;
        }

        public static bool TryParse(string s, out long result)
        {
            result = 0;
            ulong temp;

            if (s[0] == '-')
            {
                if (!ulong.TryParse(s.Substring(1), out temp))
                    return false;

                if (temp > absMaxValue)
                    return false;

                result = (long)temp;
                return true;
            }

            if (!ulong.TryParse(s, out temp))
                return false;

            if (temp > MaxValue)
                return false;

            result = (long)temp;
            return true;
        }

        public override string ToString()
        {
            uint temp;
            string sign = "";

            if (m_value > 0)
                temp = (uint)m_value;
            else
            {
                temp = (uint)(-m_value);
                sign = "-";
            }

            return sign + temp.ToString();
        }
    }

}
