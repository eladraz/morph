using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class PatriotMissile : GraphicObject
    {
        protected int m_angle;
        public int m_locationX;
        public int m_locationY;
        protected int m_speedX;
        protected int m_speedY;
        protected int m_lastX1;
        protected int m_lastY1;
        protected int m_garvityFloatAdaptation;

        public PatriotMissile(int angle, int locationX, int locationY, int speedY, int speedX, IGraphicsAdapter graphic)
            : base(graphic)
        {
            m_angle = angle;

            this.m_locationX = locationX;
            this.m_locationY = locationY;
            this.m_speedX = speedX;
            this.m_speedY = speedY;
        }

        internal void Advance()
        {
            m_locationY += m_speedY;
            m_locationX += m_speedX;

            //Gravity
            m_garvityFloatAdaptation++;
            if (m_garvityFloatAdaptation == GameConsts.GravityFloatAdaptaion)
            {
                m_speedY += GameConsts.Gravity;
                m_garvityFloatAdaptation = 0;
            }
        }

        internal override void Draw()
        {
            m_lastX1 = m_locationX - 3;
            m_lastY1 = m_locationY - 3;
            m_graphics.FillRectangle(m_lastX1, m_lastY1, 6, 6, 0x7f00ff00);
        }
        internal override void UnDraw()
        {
            m_graphics.FillRectangle(m_lastX1, m_lastY1, 6, 6, 0x7f000000);
        }

        public bool CheckColission(Missile hostile)
        {
            //sqrt((locationx-hostile.loctionx) ^ 2 + (locationy - hostile.locationy)^2)
            int distance = MathHelper.SqrRoot((m_locationX - hostile.m_locationX) * (m_locationX - hostile.m_locationX)
                                            + (m_locationY - hostile.m_locationY) * (m_locationY - hostile.m_locationY)
                                            );
            return distance <= GameConsts.KillRadius;
        }
    }
}
