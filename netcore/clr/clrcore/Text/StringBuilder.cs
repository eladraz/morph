//This class implements a NON-EFFICIENT string builder, to server as a placeholder

namespace Morph.Text
{
    public class StringBuilder
    {
        private string m_mystring;

        public StringBuilder() : this(null) { }


        public StringBuilder(string value, int initialCapacity)
        {
            m_mystring = value;
            this.Capacity = initialCapacity;
        }

        public StringBuilder(string value)
        {
            if (value == null)
                m_mystring = "";

            m_mystring = value;
        }

        public void Append(string value)
        {
            m_mystring = m_mystring + value;
        }

        public override string ToString()
        {
            return m_mystring;
        }


        //Properties

        public int Capacity { get; set; }

        public int Length
        {

            get;
            set;
            //TODO: Implement Capacity:
            //        Like the String.Length property, the Length property indicates the length of the current string object. Unlike the String.Length property, which is read-only, the Length property allows you to modify the length of the string stored to the StringBuilder object.
            //   If the specified length is less than the current length, the current StringBuilder object is truncated to the specified length. If the specified length is greater than the current length, the end of the string value of the current StringBuilder object is padded with the Unicode NULL character (U+0000).
            //   If the specified length is greater than the current capacity, Capacity increases so that it is greater than or equal to the specified length.
        }

        //TODO: StringBuilder(int capacity)

        //TODO: Append(string value, int startIndex, int count)
    }
}