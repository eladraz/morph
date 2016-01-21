namespace clrcore
{
    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = false)]
    public class Import : System.Attribute
    {
        private string importName;

        public Import(string importName)
        {
            this.importName = importName;
        }
    }

}
