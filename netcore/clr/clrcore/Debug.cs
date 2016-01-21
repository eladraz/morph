namespace Morph.Diagnostics
{
    class Debug
    {
        public static void Assert(bool condition)
        {
            if (!condition)
                Morph.Imports.assertFalse();
        }
    }
}
