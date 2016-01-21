using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Patriot;

namespace PatriotDisplay
{
    public partial class Form1 : Form
    {
        Patriot.PatriotGame m_game;
        NetGameControl m_control = new NetGameControl();

        public Form1()
        {
            InitializeComponent();
            this.SetStyle(
                          ControlStyles.AllPaintingInWmPaint |
                          ControlStyles.UserPaint |
                          ControlStyles.DoubleBuffer, true);
            timer1.Tick += new EventHandler(TimerTick);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
        }

        private void btnStartGame_Click(object sender, EventArgs e)
        {
            IGraphicsAdapter graph = new NetGraphicAdapter(this.CreateGraphics());
            m_game = new PatriotGame();
            m_game.Init(graph, m_control);

            timer1.Interval =40;

            timer1.Enabled = true;
        }

        void TimerTick(object sender, EventArgs e)
        {
            m_game.Tick();
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.A || e.KeyCode == Keys.W)
            {
                m_control.TurretUp();
            }
            if (e.KeyCode == Keys.D || e.KeyCode == Keys.S)
            {
                m_control.TurretDown();
            }
            if (e.KeyCode == Keys.T)
            {
                m_control.Press();
            }
        }
    }
}
