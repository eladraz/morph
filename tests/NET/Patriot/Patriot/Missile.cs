using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class Missile : LineObject
    {
        public int m_locationX;
        public int m_locationY;
        protected int m_speedX;
        protected int m_speedY;
        protected int m_garvityFloatAdaptation = 0;

        public Missile(int locationX, int locationY, int speedY, int speedX, IGraphicsAdapter graphic) : base(graphic)
        {
            this.m_locationX = locationX;
            this.m_locationY = locationY;
            this.m_speedX = speedX;
            this.m_speedY = speedY;
        }

        internal void Advance()
        {
            m_locationY += m_speedY ;
            m_locationX += m_speedX;

            //Gravity
            m_garvityFloatAdaptation++;
            if (m_garvityFloatAdaptation == GameConsts.GravityFloatAdaptaion)
            {
                m_speedY += GameConsts.Gravity;
                m_garvityFloatAdaptation = 0;
            }
        }

        protected override int getX1()
        {
            return m_locationX - 10;
        }
        protected override int getY1()
        {
            return m_locationY;
        }
        protected override int getX2()
        {
            return m_locationX + 10;
        }
        protected override int getY2()
        {
            return m_locationY;
        }
        protected override int GetColor()
        {
            return 0x7fff0000;
        }

    }
}
