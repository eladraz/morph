namespace Morph
{
    public class NotImplementedException : SystemException
    {
        const int Result = unchecked((int)0x80131515);

        // Constructors
        public NotImplementedException()
            : base("Operation is not yet implemented.")
        {
            HResult = Result;

        }

        public NotImplementedException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public NotImplementedException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
