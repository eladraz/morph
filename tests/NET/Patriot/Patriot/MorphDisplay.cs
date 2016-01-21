using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class MorphDisplay : IGraphicsAdapter
    {
        public MorphDisplay()
        {
            Morph.UIX.Init();
        }
        public int GetHeight()
        {
            return Morph.UIX.GetHeight();
        }

        public int GetWidth()
        {
            return Morph.UIX.GetWidth();
        }

        public void DrawRectangle(int x, int y, int width, int height, int color)
        {
            Morph.UIX.DrawRectangle(x, y, width, height, color);
        }

        public void FillRectangle(int x, int y, int width, int height, int color)
        {
            Morph.UIX.FillRectangle(x, y, width, height, color);
        }

        public void Text(int x, int y, string txt)
        {
            Morph.UIX.Text(x, y, txt);
        }
    }
}
