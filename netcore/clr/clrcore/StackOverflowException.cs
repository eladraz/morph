namespace Morph
{
    public sealed class StackOverflowException : SystemException
    {
        // Constructors
        public StackOverflowException()
            : base("The requested operation caused a stack overflow.")
        {
        }

        public StackOverflowException(string message)
            : base(message)
        {
        }

        public StackOverflowException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
