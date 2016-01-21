namespace Morph
{
    public struct Byte
    {
        public const byte MinValue = 0;
        public const byte MaxValue = 255;

        internal byte m_value;

        public int CompareTo(System.Object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.Byte))
            {
                throw new System.ArgumentException("Value is not a System.Byte.");
            }

            return this.CompareTo((byte) value);
        }

        public override bool Equals(System.Object obj)
        {
            if (!(obj is System.Byte))
                return false;

            return this.Equals((byte)obj);
        }

        public override int GetHashCode()
        {
            return m_value;
        }

        public int CompareTo(byte value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public bool Equals(byte obj)
        {
            return m_value == obj;
        }

        public static byte Parse(string s)
        {
            uint tmpResult = uint.Parse(s);

            if (tmpResult > Byte.MaxValue)
                throw new System.OverflowException("Value too large.");

            return (byte)tmpResult;
        }

        public static bool TryParse(string s,out byte result)
        {
            result = 0;
            uint tmpResult;

            if (!uint.TryParse(s, out tmpResult))
                return false;

            if (tmpResult >= MinValue && tmpResult <= MaxValue)
            {
                result = (byte)tmpResult;
                return true;
            }

            return false;
        }

        public override string ToString()
        {
            uint i = m_value;
            return i.ToString();
        }
    }
}
