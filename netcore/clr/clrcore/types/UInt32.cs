namespace Morph
{
    // Code taken form Mono 2.10.7, be aware code modified for our needs.
    public struct UInt32
    {

        public const uint MaxValue = (uint)0xFFFFFFFF;
        public const uint MinValue = unchecked((uint)0x00);
        internal uint m_value;

        public override int GetHashCode()
        {
            return (int)m_value;
        }

        public int CompareTo(uint value)
        {
            if (m_value == value)
                return 0;
            if (m_value > value)
                return 1;
            else
                return -1;
        }

        public bool Equals(uint obj)
        {
            return obj == m_value;
        }

        public int CompareTo(System.Object value)
        {
            if (value == null)
                return 1;

            if (!(value is System.UInt32))
            {
                throw new System.ArgumentException("Value is not a System.UInt32.");
            }

            return CompareTo((uint)value);
        }

        public override bool Equals(System.Object obj)
        {

            if (!(obj is System.UInt32))
                return false;

            return Equals((uint)obj);
        }

        public unsafe override string ToString()
        {
            uint value = m_value;
            byte* newString = (byte*)Morph.Imports.allocate(20);
            uint size = 0;

            // TODO! UInt32!!!!
            if (value == 0)
            {
                newString[0] = (byte)'0';
                size = 1;
            }
            else
            {
                // Get digits
                while (value > 0)
                {
                    newString[size] = (byte)('0' + (value % 10));
                    size++;
                    value /= 10;
                }
                // reverse order
                for (uint i = 0; i < (size / 2); i++)
                {
                    byte temp = newString[i];
                    newString[i] = newString[size - i - 1];
                    newString[size - i - 1] = temp;
                }
            }

            Morph.String obj = new Morph.String(size, newString, true);
            Morph.Imports.free(newString);
            return Morph.Imports.convertToString(obj);
        }

        public static uint Parse(string s)
        {
            uint result = 0;
            int i = 0, len = s.Length;
            char c;

            if (s == null)
                throw new System.ArgumentNullException();

            if (len <= 0)
                return 0;

            if (len > 10)
                throw new System.OverflowException("Value too large.");

            for (; i < len; i++)
            {
                c = s[i];

                if (c >= '0' && c <= '9')
                {
                    byte d = (byte)(c - '0');
                    if (result > (uint.MaxValue / 10))
                        throw new System.OverflowException("Value too large.");

                    if (result == (uint.MaxValue / 10))
                    {
                        if (d > (uint.MaxValue % 10))
                            throw new System.OverflowException("Value too large.");

                        result = result * 10 + d;
                    }
                    else
                        result = result * 10 + d;
                }
                else
                    throw new System.ArgumentException("Value is not a System.UInt32.");
            }

            return result;
        }

        public static bool TryParse(string s, out uint result)
        {
            result = 0;

            uint _result = result;

            if (s == null)
                return false;

            int i = 0, len = s.Length;
            char c;

            if (len <= 0)
                return false;

            if (len > 10)
                return false;

            for (; i < len; i++)
            {
                c = s[i];

                if (c >= '0' && c <= '9')
                {
                    byte d = (byte)(c - '0');
                    if (_result > (uint.MaxValue / 10))
                        return false;

                    if (_result == (uint.MaxValue / 10))
                    {
                        if (d > (uint.MaxValue % 10))
                            return false;

                        _result = _result * 10 + d;
                    }
                    else
                        _result = _result * 10 + d;
                }
                else
                    return false;
            }

            result = _result;

            return true;
        }
    }
}
