namespace Morph
{
    public sealed class IndexOutOfRangeException : SystemException
    {
        // Constructors
        public IndexOutOfRangeException()
            : base("Array index is out of range.")
        {
        }

        public IndexOutOfRangeException(string message)
            : base(message)
        {
        }

        public IndexOutOfRangeException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
