namespace Morph
{
    public class ArithmeticException : SystemException
    {
        const int Result = unchecked((int)0x80070216);

        // Constructors
        public ArithmeticException()
            : base("Overflow or underflow in the arithmetic operation.")
        {
            HResult = Result;
        }

        public ArithmeticException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public ArithmeticException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
