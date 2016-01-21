namespace Morph
{
    public struct SByte
    {
        public const sbyte MinValue = -128;
        public const sbyte MaxValue = 127;

        internal sbyte m_value;

        public int CompareTo(System.Object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.SByte))
            {
                throw new System.ArgumentException("Value is not a System.SByte.");
            }

            return this.CompareTo((sbyte)value);
        }

        public override bool Equals(System.Object obj)
        {
            if (!(obj is System.SByte))
                return false;

            return this.Equals((sbyte)obj);
        }

        public override int GetHashCode()
        {
            return m_value;
        }

        public int CompareTo(sbyte value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public bool Equals(sbyte obj)
        {
            return m_value == obj;
        }

        public static sbyte Parse(string s)
        {
            int tmpResult = int.Parse(s);

            if (tmpResult > Byte.MaxValue)
                throw new System.OverflowException("Value is too large");

            return (sbyte)tmpResult;
        }

        public static bool TryParse(string s, out sbyte result)
        {
            result = 0;
            int tmpResult;

            if (!int.TryParse(s, out tmpResult))
                return false;

            if (tmpResult >= MinValue && tmpResult <= MaxValue)
            {
                result = (sbyte)tmpResult;
                return true;
            }

            return false;
        }

        public override string ToString()
        {
            int i = m_value;
            return i.ToString();
        }
    }
}
