namespace Morph
{
    public class InvalidOperationException : SystemException
    {
        const int Result = unchecked((int)0x80131509);

        // Constructors
        public InvalidOperationException()
            : base("Operation is not valid due to the current state of the object")
        {
            HResult = Result;
        }

        public InvalidOperationException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public InvalidOperationException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
