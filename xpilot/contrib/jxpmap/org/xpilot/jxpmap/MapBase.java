package org.xpilot.jxpmap;

import java.awt.*;
import javax.swing.*;
import java.io.PrintWriter;
import java.io.IOException;

public class MapBase extends MapObject {

    private static final int LEFT = 0;
    private static final int DOWN = 1;
    private static final int RIGHT = 2;
    private static final int UP = 3;

    private static final String[] IMAGES = 
    { "base_left.gif", "base_down.gif", "base_right.gif", "base_up.gif" };

    private int dir;
    private int team;


    public MapBase () {
        this(0, 0, 32, 1);
    }


    public MapBase (int x, int y, int dir, int team) {
        super(null, x, y, 35 * 64, 35 * 64);
        setDir(dir);
        setTeam(team);
    }


    public int getDir () {
        return dir;
    }


    public void setDir (int dir) {
        
        if (dir > 127) dir = 127;
        if (dir < 0) dir = 0;
        setImage(IMAGES[getOrientationByDir(dir)]);
        this.dir = dir;
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
        out.print("<Base x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.print("\" team=\"");
        out.print(getTeam());
        out.print("\" dir=\"");
        out.print(getDir());
        out.println("\"/>");
    }

    
    public PropertyEditor getPropertyEditor (MapCanvas c) {
        return new BasePropertyEditor();
    }


    private int getOrientationByDir (int dir) {
        if (dir < 16) return LEFT;
        if (dir < 48) return DOWN;
        if (dir < 80) return RIGHT;
        if (dir < 112) return UP;
        return LEFT;
    }


    private class BasePropertyEditor extends PropertyEditor {

        private JComboBox cmbTeam;
        private JComboBox cmbDir;


        public BasePropertyEditor () {

            cmbTeam = new JComboBox();
            for (int i = 0; i < 10; i++) 
                cmbTeam.addItem(new Integer(i + 1));
            cmbTeam.setSelectedIndex(getTeam() - 1);
            
            cmbDir = new JComboBox();
            cmbDir.addItem("LEFT");
            cmbDir.addItem("DOWN");
            cmbDir.addItem("RIGHT");
            cmbDir.addItem("UP");
            cmbDir.setSelectedIndex(getOrientationByDir(getDir()));
            
            
            setLayout(new GridLayout(2,2));
            add(new JLabel("Team:"));
            add(cmbTeam);
            add(new JLabel("Direction:"));
            add(cmbDir);
        }
        
        
        public boolean apply () {
            setTeam(cmbTeam.getSelectedIndex() + 1);
            setDir(cmbDir.getSelectedIndex() * 32);
            return true;
        }
    }
}
