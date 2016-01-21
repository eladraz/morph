using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Patriot;

namespace PatriotDisplay
{
    class NetGameControl : IGameControl
    {
        private int m_angle = 45;
        private bool m_pressed;

        public int GetAngle()
        {
            return m_angle;
        }

        public void SetAngle(int angle)
        {
            if (angle < 90 && angle > 0)
            {
                m_angle = angle;
            }
        }

        internal void TurretUp()
        {
            if (m_angle < 90 - GameConsts.TurretTurenStep)
            {
                m_angle += GameConsts.TurretTurenStep;
            }
        }
        internal void TurretDown()
        {
            if (m_angle > GameConsts.TurretTurenStep)
            {
                m_angle -= GameConsts.TurretTurenStep;
            }
        }

        public void Press()
        {
            m_pressed = true;
        }
        public bool IsPressed()
        {
            bool result = m_pressed;
            m_pressed = false;
            return result;
        }
    }
}
