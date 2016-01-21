namespace clrcore
{
    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = false)]
    public class CompilerOpcodes : System.Attribute
    {
        private string opcode;

        public CompilerOpcodes(string opcode)
        {
            this.opcode = opcode;
        }
    }
}
