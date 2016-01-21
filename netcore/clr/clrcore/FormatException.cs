namespace Morph
{
    public class FormatException : SystemException
    {
        const int Result = unchecked((int)0x80131537);

        // Constructors
        public FormatException()
            : base("Invalid format.")
        {
            HResult = Result;
        }

        public FormatException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public FormatException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}