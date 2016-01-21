
namespace Morph
{
    class InvalidCastException : SystemException
    {
        const int Result = unchecked((int)0x80004002);  ///COR_E_INVALIDCAST

        // Constructors
        public InvalidCastException()
            : base("Invalid cast")
        {
            HResult = Result;
        }

        public InvalidCastException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public InvalidCastException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }

}