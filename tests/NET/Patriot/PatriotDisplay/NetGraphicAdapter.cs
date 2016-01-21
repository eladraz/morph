using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
namespace PatriotDisplay
{
    class NetGraphicAdapter : Patriot.IGraphicsAdapter
    {
        private System.Drawing.Graphics m_graphics;

        public NetGraphicAdapter(System.Drawing.Graphics graphics)
        {
            m_graphics = graphics;
        }

        public void DrawRectangle(int x, int y, int width, int height, int argbColor)
        {
            m_graphics.DrawRectangle(new Pen(Color.FromArgb(argbColor)), new Rectangle(x, y, width, height));
        }

        public int GetHeight()
        {
            return (int) m_graphics.VisibleClipBounds.Height;
        }

        public int GetWidth()
        {
            return (int)m_graphics.VisibleClipBounds.Width;
        }

        public void FillRectangle(int x, int y, int width, int height, int color)
        {
            Brush brush = new SolidBrush(GetOpaqueColor(color));
            m_graphics.FillRectangle(brush, new Rectangle(x, y, width, height));
        }

        public void DrawLine(int x1, int y1, int x2, int y2, int width, int color)
        {
            m_graphics.DrawLine(new Pen(GetOpaqueColor(color), width), x1, y1, x2, y2);
        }

        private Color GetOpaqueColor(int color)
        {
            return Color.FromArgb(255, Color.FromArgb(color));
        }
    }
}
