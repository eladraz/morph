using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Patriot
{
    class Turret : GraphicObject
    {
        int m_angle;
        int m_oldX2;
        int m_oldY2;

        public Turret(IGraphicsAdapter graphic, int initial_angle)
        : base(graphic)
        {
            m_angle = initial_angle;
        }

        internal void TurretUp()
        {
            if (m_angle < 90 - GameConsts.TurretTurenStep)
            {
                this.UnDrawAim();
                m_angle += GameConsts.TurretTurenStep;
                this.Draw();
            }
        }
        internal void TurretDown()
        {
            if (m_angle > GameConsts.TurretTurenStep)
            {
                this.UnDrawAim();
                m_angle -= GameConsts.TurretTurenStep;
                this.Draw();
            }
        }
        public int Angle
        {
            set
            {
                if (value > 0 && value < 90)
                {
                    this.UnDrawAim();
                    m_angle = value;
                    this.Draw();
                }
            }
            get
            {
                return m_angle;
            }
        }

        internal override void Draw()
        {
            //base rectangle
            m_graphics.FillRectangle(0, m_graphics.GetHeight()-10, 10, 10, 0x7f00ff00);
            //upper rectangle
            m_oldX2 = MathHelper.Trigonometry.Cos(m_angle, GameConsts.TurretRadius) - 5;
            m_oldY2 = m_graphics.GetHeight() - MathHelper.Trigonometry.Sin(m_angle, GameConsts.TurretRadius) - 5;
            m_graphics.FillRectangle(m_oldX2, m_oldY2, 10, 10, 0x7f00ff00);
        }

        internal override void UnDraw()
        {
            //base rectangle
            UnDrawBase();
            //upper rectangle
            UnDrawAim();
        }

        private void UnDrawAim()
        {
            m_graphics.FillRectangle(m_oldX2, m_oldY2, 10, 10, 0x7000000);
        }

        private void UnDrawBase()
        {
            m_graphics.FillRectangle(0, m_graphics.GetHeight(), 10, 10, 0x7f000000);
        }
    }
}
