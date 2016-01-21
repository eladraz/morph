namespace Morph
{
    public class OverflowException : ArithmeticException
    {
        const int Result = unchecked((int)0x80131516);

        // Constructors
        public OverflowException()
            : base("Number overflow.")
        {
            HResult = Result;
        }

        public OverflowException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public OverflowException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
