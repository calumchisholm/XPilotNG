package org.xpilot.jxpmap;

import java.io.PrintWriter;
import java.io.IOException;
import javax.swing.*;

public class MapBall extends MapObject {

    private int team;


    public MapBall () {
        this(0, 0, 1);
    }

    
    public MapBall (int x, int y, int team) {
        super("ball.gif", x, y, 21 * 64, 21 * 64);
        this.team = team;
    }


    public int getTeam () {
        return team;
    }


    public void setTeam (int team) {
        if (team < 1 || team > 10)
            throw new IllegalArgumentException
                ("illegal team: " + team);
        this.team = team;
    }


    public void printXml (PrintWriter out) throws IOException {
        out.print("<Ball x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.print("\" team=\"");
        out.print(getTeam());
        out.println("\"/>");
    }


    public EditorPanel getPropertyEditor (MapCanvas canvas) {
        return new BallPropertyEditor(canvas);
    }


    private class BallPropertyEditor extends EditorPanel {

        private JComboBox cmbTeam;
        private MapCanvas canvas;

        public BallPropertyEditor (MapCanvas canvas) {
            setTitle("Ball Properties");
            setLayout(new java.awt.GridLayout(1, 2));
            cmbTeam = new JComboBox();
            for (int i = 0; i < 10; i++) 
                cmbTeam.addItem(new Integer(i + 1));
            cmbTeam.setSelectedIndex(getTeam() - 1);
            add(new JLabel("Team:"));
            add(cmbTeam);
            this.canvas = canvas;
        }
        
        public boolean apply () {
            int newTeam = cmbTeam.getSelectedIndex() + 1;
            if (newTeam != getTeam()) 
                canvas.setBallTeam(MapBall.this, newTeam);
            return true;
        }
    }
}
