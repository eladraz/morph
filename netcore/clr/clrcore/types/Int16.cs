namespace Morph
{
    public struct Int16
    {
        public const short MaxValue = 32767;
        public const short MinValue = -32768;

        internal short m_value;

        public int CompareTo(object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.Int16))
            {
                throw new System.ArgumentException("Value is not a System.Int16.");
            }

            return this.CompareTo((short)value);
        }

        public override bool Equals(object obj)
        {
            if (!(obj is System.Int16))
                return false;

            return this.Equals((short)obj);
        }

        public override int GetHashCode()
        {
            return m_value;
        }

        public int CompareTo(short value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public bool Equals(short obj)
        {
            return obj == m_value;
        }

        public static short Parse(string s)
        {
            int temp = int.Parse(s);
            if (temp >= MinValue && temp <= MaxValue)
                return (short)temp;
            else
            {
                throw new System.OverflowException("Value is too large");
            }
        }

        public static bool TryParse(string s, out short result)
        {
            result = 0;
            int temp;

            if (!int.TryParse(s, out temp))
                return false;

            if (temp >= MinValue && temp <= MaxValue)
            {
                result = (short)temp;
                return true;
            }

            return false;
        }

        public override string ToString()
        {
            int val = m_value;

            return val.ToString();
        }
    }
}