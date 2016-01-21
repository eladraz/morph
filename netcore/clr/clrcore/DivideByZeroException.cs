namespace Morph
{
    public class DivideByZeroException : ArithmeticException
    {
        const int Result = unchecked((int)0x80020012);

        // Constructors
        public DivideByZeroException()
            : base("Division by zero")
        {
            HResult = Result;
        }

        public DivideByZeroException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public DivideByZeroException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
