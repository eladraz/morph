namespace Morph
{
    public class SystemException : Exception
    {
        const int Result = unchecked((int)0x80131501);

        // Constructors
        public SystemException()
            : base("A system exception has occurred.")
        {
            HResult = Result;
        }

        public SystemException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public SystemException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
