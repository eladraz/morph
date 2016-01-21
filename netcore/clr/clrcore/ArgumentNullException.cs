namespace Morph
{
    public class ArgumentNullException : ArgumentException
    {
        const int Result = unchecked((int)0x80004003);

        // Constructors
        public ArgumentNullException()
            : base("Argument cannot be null.")
        {
            HResult = Result;
        }

        public ArgumentNullException(string paramName)
            : base("Argument cannot be null.", paramName)
        {
            HResult = Result;
        }

        public ArgumentNullException(string paramName, string message)
            : base(message, paramName)
        {
            HResult = Result;
        }

        public ArgumentNullException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
