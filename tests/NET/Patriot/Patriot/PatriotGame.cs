using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Patriot
{
    public class PatriotGame
    {
        //private Board myBoard = new Board();
        SimpleCollection missileList = new SimpleCollection(50);
        SimpleCollection patriotList = new SimpleCollection(30);
        int score;
        private int currentBurstSize;
        private Turret m_turret;
        private IGameControl m_control;

        private int sizeX;
        private int sizeY;
        private IGraphicsAdapter myGraphics;

        //Game mechanic
        private Random   r = new Random();

        public void SetTurrentAngle(int angle)
        {
            m_turret.Angle = angle;
        }



        public void Tick()
        {
            int angle = m_control.GetAngle();
            m_turret.Angle = angle;
            bool isPressed = m_control.IsPressed();
            if (!isPressed)
                currentBurstSize = 0;
            else
            {
                bool tmp = (patriotList.Count >= GameConsts.maxPatriotsInAir);
                if (tmp)
                {
                    tmp = (currentBurstSize < GameConsts.maxPatriotBurst);
                    if (tmp)
            {
                FirePatriot();
                        currentBurstSize++;
                    }
                }
            }

            SimpleCollection missileRemoveList = new SimpleCollection(50);
            SimpleCollection patriotRemoveList = new SimpleCollection(30);

            AdvanceAndCheckBounds(missileRemoveList, patriotRemoveList);

            CheckInterceptions(missileRemoveList, patriotRemoveList);
            RemoveDeadObjects(missileRemoveList, patriotRemoveList);

            //add new missiles
            int rocket = r.Next();
            if ((rocket & 49) == 17)
            {
                FireHostileMissile();
            }

            DrawBoard();
        }

        private void CheckInterceptions(SimpleCollection missileRemoveList, SimpleCollection patriotRemoveList)
        {
            foreach (PatriotMissile patriot in patriotList)
            {
                foreach (Missile msle in missileList)
                {
                    if (patriot.CheckColission(msle))
                    {
                        missileRemoveList.Add(msle);
                        patriotRemoveList.Add(patriot);
                        score = score + 5;
                    }
                }
            }
        }

        private void RemoveDeadObjects(ICollection msleRemoveList, ICollection patriotRemoveList)
        {
            foreach (Missile rmv_missile in msleRemoveList)
            {
                rmv_missile.UnDraw();
                missileList.Remove(rmv_missile);
            }
            foreach (PatriotMissile rmv_patriot in patriotRemoveList)
            {
                rmv_patriot.UnDraw();
                patriotList.Remove(rmv_patriot);
            }

        }

        private void AdvanceAndCheckBounds(SimpleCollection msleRemoveList, SimpleCollection patriotRemoveList)
        {
            foreach (Missile msle in missileList)
            {
                msle.Advance();
                if (msle.m_locationX < 0 || msle.m_locationY > sizeY + 6)
                {
                    msleRemoveList.Add(msle);
                    score = score - 2;
                }
            }

            foreach (PatriotMissile patriot in patriotList)
            {
                patriot.Advance();

                if (patriot.m_locationX > sizeX || patriot.m_locationY <= -6)
                {
                    patriotRemoveList.Add(patriot);
                    score = score - 1;
                }
            }
        }

        public void DrawBoard()
        {
            //UnDraw Missiles
            foreach (Missile msle in missileList)
            {
                msle.UnDraw();
                msle.Draw();
            }
            foreach (PatriotMissile patriot in patriotList)
            {
                patriot.UnDraw();
                patriot.Draw();
            }

            m_turret.Draw();
            myGraphics.FillRectangle(0, 0, 100, 20, 0);
            myGraphics.DrawRectangle(0, 0, 0, 0, 0x008A2BE2);
            myGraphics.Text(0, 0, "score:" + score);
        }

        public void Init(IGraphicsAdapter graphics, IGameControl control)
        {
            myGraphics = graphics;
            m_control = control;
            currentBurstSize = 0;
            score = 0;
            sizeX = myGraphics.GetWidth();
            sizeY = myGraphics.GetHeight();
            DrawBackground();

            m_turret = new Turret(graphics, control.GetAngle());

            m_turret.Draw();

            //Init GameConsts
            GameConsts.stepCount = sizeX >> 8;
            if (GameConsts.stepCount == 0)
            {
                GameConsts.stepCount = 1;
            }
            GameConsts.Gravity = 1;
        }

        private void DrawBackground()
        {
            myGraphics.FillRectangle(0, 0, sizeX, sizeY, 0x78000000);
        }

        private void FirePatriot()
        {
            int speedX = MathHelper.Trigonometry.Cos(m_turret.Angle, GameConsts.PatriotBaseSpeed);
            int speedY = (-1) * MathHelper.Trigonometry.Sin(m_turret.Angle, GameConsts.PatriotBaseSpeed);
            PatriotMissile t_patriot = new PatriotMissile(m_turret.Angle, 0, sizeY, speedY, speedX, myGraphics);
            patriotList.Add(t_patriot);
        }

        private void FireHostileMissile()
        {
            int locationY = r.Next(sizeY >> 1) + r.Next(sizeY >> 2) + r.Next(sizeY >> 3) + r.Next(sizeY >> 3);
            int speedX = GameConsts.MissileSpeedX - r.Next(6);
            int speedY = GameConsts.MissileSpeedY - r.Next(4);
            Missile t_missile = new Missile(sizeX, locationY, speedY, speedX, myGraphics);
            missileList.Add(t_missile);
            int countForeach =0;
            foreach (Missile msle in missileList)
            {
                countForeach++;
            }

        }
    }
}
