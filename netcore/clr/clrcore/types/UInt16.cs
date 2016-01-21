using System;
using System.Collections.Generic;
using System.Text;

namespace Morph
{
    public struct UInt16
    {
        public const ushort MaxValue = 0xffff;
        public const ushort MinValue = 0;

        internal ushort m_value;

        public int CompareTo(object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.UInt16))
            {
                throw new System.ArgumentException("Value is not a System.UInt16.");
            }

            return this.CompareTo((ushort)value);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is System.UInt16))
                return false;

            return this.Equals((ushort) obj);
        }

        public override int GetHashCode()
        {
            return m_value;
        }

        public int CompareTo(ushort value)
        {
            return m_value - value;
        }

        public bool Equals(ushort obj)
        {
            return obj == m_value;
        }

        public static ushort Parse(string s)
        {
            uint temp = uint.Parse(s);
            if (temp <= MaxValue)
                return (ushort)temp;
            else
            {
                throw new System.OverflowException("Value too large.");
            }
        }

        public static bool TryParse(string s, out ushort result)
        {
            result = 0;
            uint temp;

            if (!uint.TryParse(s, out temp))
                return false;

            if (temp <= MaxValue)
            {
                result = (ushort)temp;
                return true;
            }

            return false;
        }

        public override string ToString()
        {
            uint val = m_value;

            return val.ToString();
        }
    }
}
