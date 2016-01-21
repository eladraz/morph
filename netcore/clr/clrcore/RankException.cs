namespace Morph
{
    public class RankException : SystemException
    {
        const int Result = unchecked((int)0x80131517);

        // Constructors
        public RankException()
            : base("Two arrays must have the same number of dimensions.")
        {
            HResult = Result;
        }

        public RankException(string message)
            : base(message)
        {
            HResult = Result;
        }

        public RankException(string message, Exception innerException)
            : base(message, innerException)
        {
            HResult = Result;
        }
    }
}
