namespace Morph
{
    public struct Boolean
    {
        /// <value>
        /// The String representation of Boolean False
        /// </value>
        public static readonly string FalseString = "False";

        /// <value>
        /// The String representation of Boolean True
        /// </value>
        public static readonly string TrueString = "True";

        internal bool m_value;

        public int CompareTo(System.Object obj)
        {
            if (obj == null)
                return 1;

            if (!(obj is System.Boolean))
            {
                throw new System.ArgumentException("Object is not a Boolean.");
            }

            return this.CompareTo((bool)obj);
        }

        public override bool Equals(System.Object obj)
        {
            if (obj == null || !(obj is System.Boolean))
                return false;

            return this.Equals((bool)obj);
        }

        public int CompareTo(bool value)
        {
            if (m_value == value)
                return 0;

            //return !m_value ? -1 : 1;
            if (m_value)
                return 1;
            return -1;
        }

        public bool Equals(bool obj)
        {
            return m_value == obj;
        }

        public override int GetHashCode()
        {
            //return m_value ? 1 : 0;
            if (m_value)
                return 1;
            return 0;
        }

        public static bool Parse(string value)
        {
            if (value == null)
                throw new System.ArgumentNullException("value");

            value = value.Trim();

            if (string.Compare(value, TrueString, true) == 0)
                return true;

            if (string.Compare(value, FalseString, true) == 0)
                return false;

            throw new System.FormatException("Value is not equivalent to either TrueString or FalseString.");
        }

        public static bool TryParse(string value, out bool result)
        {
            result = false;
            if (value == null)
                return false;

            value = value.Trim();

            if (string.Compare(value, TrueString, true) == 0)
            {
                result = true;
                return true;
            }

            if (string.Compare(value, FalseString, true) == 0)
            {
                // result = false; // already set at false by default
                return true;
            }

            return false;
        }

        public override string ToString()
        {
            //return m_value ? TrueString : FalseString;
            if (m_value)
                return TrueString;
            else
                return FalseString;
        }
    }
}
