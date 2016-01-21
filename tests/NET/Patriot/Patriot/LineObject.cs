using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class LineObject : GraphicObject
    {


        protected int m_lastX1, m_lastY1, m_lastX2, m_lastY2;

        public LineObject(IGraphicsAdapter graphics) : base ( graphics)
        {

        }

        internal override void Draw()
        {
            m_lastX1 = getX1();
            m_lastY1 = getY1();
            m_lastX2 = getX2();
            m_lastY2 = getY2();
            m_graphics.FillRectangle(m_lastX1, m_lastY1, (m_lastX2-m_lastX1), 5, GetColor());

        }
        internal override void UnDraw()
        {
            m_graphics.FillRectangle(m_lastX1, m_lastY1, (m_lastX2 - m_lastX1), 5, 0x7f000000);
        }

        protected virtual int getX1() { return 0; }
        protected virtual int getY1() { return 0; }
        protected virtual int getX2() { return 0; }
        protected virtual int getY2() { return 0; }
        protected virtual int GetColor() { return 0; }


    }
}
