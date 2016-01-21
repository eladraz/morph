using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    public interface IGraphicsAdapter
    {

        int GetHeight();
        int GetWidth();

        void DrawRectangle(int x, int y, int width, int height, int color);
        void FillRectangle(int x, int y, int width, int height, int color);
        void Text(int x, int y, string txt);
    }
}
