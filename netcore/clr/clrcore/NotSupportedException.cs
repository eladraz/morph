namespace Morph
{
    public class NotSupportedException : SystemException
    {
        const int Result = unchecked((int)0x80131515);

        // Constructors
        public NotSupportedException()
            : base("Operation is not supported.")
        {
            HResult = Result;
        }

        public NotSupportedException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public NotSupportedException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
