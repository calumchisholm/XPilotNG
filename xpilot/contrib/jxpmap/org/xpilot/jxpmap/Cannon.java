package org.xpilot.jxpmap;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Map;

public class Cannon extends MapPolygon {
    
    private static final double[] arrow = {
        0, 0,
        27, 0,
        27, 3,
        30, 0, 
        27, -3,
        27, 0
    };

    private int x, y, dir;
    private int[] arrowx = new int[arrow.length / 2]; 
    private int[] arrowy = new int[arrow.length / 2];



    public Cannon() {
        super();
        setTeam(-1);
        computeArrow();
    }

    public Cannon(Polygon p, PolygonStyle style, ArrayList edgeStyles,
    int team, int x, int y, int dir) {
        super(p, style, edgeStyles);
        this.dir = dir;
        setTeam(team);
        this.x = x;
        this.y = y;
        computeArrow();
    }


    public int getDir () {
        return dir;
    }


    public void setDir (int dir) {
        
        if (dir > 127) dir = 127;
        if (dir < 0) dir = 0;
        this.dir = dir;
        computeArrow();
    }
    

    public void moveTo (int x, int y) {
        Rectangle r = getBounds();
        this.x += x - r.x;
        this.y += y - r.y;
        computeArrow();        
        super.moveTo(x, y);
    }
    
    
    public void rotate (double angle) {
        super.rotate(angle);
        double cx = getBounds().getCenterX();
        double cy = getBounds().getCenterY();
        AffineTransform at = AffineTransform.getTranslateInstance(cx, cy);
        at.rotate(angle);
        at.translate(-cx, -cy);
        Point p = new Point();
        p.x = x;
        p.y = y;
        at.transform(p, p);
        x = p.x;
        y = p.y;
        int d = getDir();
        d += (int)(127 * angle / (2 * Math.PI));
        if (d > 127) d %= 127;
        while(d < 0) d += 127;
        setDir(d);
    }
    
    public void computeArrow() {
        AffineTransform at = new AffineTransform();
        at.translate(x, y);
        at.rotate(2 * Math.PI * getDir() / 128.0);
        at.scale(64, 64);
        double[] pts = new double[arrow.length];
        at.transform(arrow, 0, pts, 0, pts.length / 2);
        for (int i = 0; i < arrowx.length; i++) {
            arrowx[i] = (int)pts[2 * i];
            arrowy[i] = (int)pts[2 * i + 1];
        }
    }

    public int getTeam () {
        return team;
    }


    public void setTeam (int team) {
        if (team < -1 || team > 10)
            throw new IllegalArgumentException
                ("illegal team: " + team);
        this.team = team;
    }

    
    public void printXml (PrintWriter out) throws IOException {
        out.println("<Cannon x=\"" + x + "\" y=\"" + y 
            + "\" dir=\"" + dir + "\">");
        super.printXml(out);
        out.println("</Cannon>");
    }

    
    public EditorPanel getPropertyEditor (MapCanvas c) {
        CompoundEditor ce = 
            new CompoundEditor("Cannon", c, this);
        ce.add(new TeamEditor("Team", c, this));
        ce.add(super.getPropertyEditor(c));
        return ce;
    }

    public void paint(Graphics2D g, float scale) {
        super.paint(g, scale);
        g.setColor(Color.yellow);
        g.drawPolyline(arrowx, arrowy, arrowx.length);
    }

    public CanvasEventHandler getCreateHandler(final Runnable r) {
        Runnable r2 = new Runnable() {
            public void run() {
                Rectangle b = getBounds();
                x = b.x + b.width;
                y = (int)b.getCenterY();
                computeArrow();
                if (r != null) r.run();
            }
        };
        return super.getCreateHandler(r2);
    }
    
    public MapObject copy() {
        Cannon copy = (Cannon)super.copy();
        copy.arrowx = new int[arrowx.length];
        copy.arrowy = new int[arrowy.length];
        copy.computeArrow();
        return copy;
    }

    public Object deepClone(Map context) {
        Cannon clone = (Cannon)super.deepClone(context);
        clone.arrowx = new int[arrowx.length];
        clone.arrowy = new int[arrowy.length];
        clone.computeArrow();
        return clone;
    }

}
