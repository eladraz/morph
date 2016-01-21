namespace Morph
{
    public class UIX
    {
        [clrcore.Import("_displayInit"), clrcore.CallingConvention("cdecl")]
        public static void Init()
        {
        }

        [clrcore.Import("_displayGetWidth"), clrcore.CallingConvention("cdecl")]
        public static int GetWidth()
        {
            return 320;
        }

        [clrcore.Import("_displayGetHeight"), clrcore.CallingConvention("cdecl")]
        public static int GetHeight()
        {
            return 200;
        }

        [clrcore.Import("_displayDrawRectangle"), clrcore.CallingConvention("cdecl")]
        public static void DrawRectangle(int x, int y, int width, int height, int color)
        {
        }

        [clrcore.Import("_displayFillRectangle"), clrcore.CallingConvention("cdecl")]
        public static void FillRectangle(int x, int y, int width, int height, int color)
        {
        }

        [clrcore.Import("_displayIsKeyPressed"), clrcore.CallingConvention("cdecl")]
        public static bool IsKeyPressed()
        {
            return false;
        }

        [clrcore.Import("_displayGetPotentiometer"), clrcore.CallingConvention("cdecl")]
        public static int GetPotentiometer()
        {
            return 0;
        }

        // Wrapper function for System.String to char*
        public static void Text(int x, int y, string str)
        {
            Morph.String mstr = Imports.convertToMorphString(str);
            unsafe
            {
                Text(x,y, (char*)mstr.getTCharBuffer(), (uint)mstr.Length);
            }
        }

        [clrcore.Import("_displayText"), clrcore.CallingConvention("cdecl")]
        public unsafe static void Text(int x, int y, char* text, uint length)
        {
        }
    }
}
