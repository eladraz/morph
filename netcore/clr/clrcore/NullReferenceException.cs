namespace Morph
{
    public class NullReferenceException : SystemException
    {
        const int Result = unchecked((int)0x80131502);

        public NullReferenceException()
            : base("null was thrown.")
        {
            HResult = Result;
        }

        public NullReferenceException(string message) :
            base(message)
        {
            HResult = Result;
        }

        public NullReferenceException(string message, Exception innerException) :
            base(message, innerException)
        {
            HResult = Result;
        }
    }
}
