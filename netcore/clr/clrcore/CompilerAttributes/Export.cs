namespace clrcore
{
    [System.AttributeUsage(System.AttributeTargets.Method, AllowMultiple = false)]
    public class Export : System.Attribute
    {
        private string exportName;

        public Export(string exportName)
        {
            this.exportName = exportName;
        }
    }

}
