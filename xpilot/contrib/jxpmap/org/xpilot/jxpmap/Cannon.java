package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import javax.swing.*;
import java.io.PrintWriter;
import java.io.IOException;

public class Cannon extends MapObject {

    private static final double OUTLINE[] =  new double[] { 
        0, 0, 
        -17 * 64, 17 * 64,
        -17 * 64, -17 * 64
    };


    private int dir;
    private int team;
    private Polygon poly;
    private PolygonStyle style;


    public Cannon (PolygonStyle style) {
        this(style, 0, 0, 32, -1);
    }

    
    public Cannon (PolygonStyle style, int x, int y, int dir, int team) {
        super(null, x, y, 35 * 64, 35 * 64);
        setDir(dir);
        setTeam(team);
        this.style = style;
        computePoly();
    }


    private void computePoly () {
        Rectangle b = getBounds();
        AffineTransform at = new AffineTransform();
        at.translate(b.x + b.width / 2 , b.y + b.height / 2);
        at.rotate(2 * Math.PI * getDir() / 128.0);
        double[] pts = new double[OUTLINE.length];
        at.transform(OUTLINE, 0, pts, 0, pts.length / 2);
        Polygon p = new Polygon();
        for (int i = 0; i < pts.length; i += 2)
            p.addPoint((int)pts[i], (int)pts[i+1]);
        this.poly = p;
    }


    public int getDir () {
        return dir;
    }


    public void setDir (int dir) {
        
        if (dir > 127) dir = 127;
        if (dir < 0) dir = 0;
        this.dir = dir;
        computePoly();
    }


    public void moveTo (int x, int y) {
        super.moveTo(x, y);
        computePoly();
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

    public void paint (Graphics2D g, float scale) {

        if (!style.isVisible()) return;

        if (style.getFillStyle() == PolygonStyle.FILL_COLOR) {
            g.setColor(style.getColor());
            g.fillPolygon(poly);
            
        } else if (style.getFillStyle() == PolygonStyle.FILL_TEXTURED) {
            BufferedImage img = style.getTexture().getImage();
            Rectangle b = poly.getBounds();
            g.setPaint(new TexturePaint(img, new Rectangle
                (b.x, b.y, img.getWidth() * 64, -img.getHeight() * 64)));
            g.fill(poly);
        }
        
        LineStyle ls = style.getDefaultEdgeStyle();
        if (ls.getStyle() != LineStyle.STYLE_HIDDEN) {
            g.setColor(ls.getColor());
            g.setStroke(ls.getStroke(scale));
            g.draw(poly);
        }
    }

    
    public void printXml (PrintWriter out) throws IOException {
        out.print("<Cannon x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.print("\" dir=\"");
        out.print(getDir());
        if (getTeam() != -1) {
            out.print("\" team=\"");
            out.print(getTeam());
        }
        out.println("\">");
        for (int i = 1; i <= poly.npoints; i++) {
            out.print("<Offset x=\"");
            out.print(poly.xpoints[i % poly.npoints] - 
                      poly.xpoints[i - 1]);
            out.print("\" y=\"");
            out.print(poly.ypoints[i % poly.npoints] - 
                      poly.ypoints[i - 1]);
        }
        out.println("</Cannon>");
    }

    
    public EditorPanel getPropertyEditor (MapCanvas c) {
        return new CannonPropertyEditor(c);
    }

    private class CannonPropertyEditor extends EditorPanel {

        private JComboBox cmbTeam;
        private JSlider sldDir;
        private MapCanvas canvas;


        public CannonPropertyEditor (MapCanvas canvas) {

            setTitle("Cannon Properties");

            cmbTeam = new JComboBox();
            for (int i = -1; i < 10; i++) 
                cmbTeam.addItem(new Integer(i));
            cmbTeam.setSelectedIndex(getTeam() + 1);
            
            sldDir = new JSlider(0, 127);
            sldDir.setMajorTickSpacing(16);
            sldDir.setMinorTickSpacing(2);
            sldDir.setPaintTicks(true);
            sldDir.setValue(getDir());
            
            setLayout(new GridLayout(2,2));
            add(new JLabel("Team:"));
            add(cmbTeam);
            add(new JLabel("Direction:"));
            add(sldDir);

            this.canvas = canvas;
        }
        
        
        public boolean apply () {
            int newTeam = cmbTeam.getSelectedIndex() - 1;
            int newDir = sldDir.getValue();
            if (newTeam != getTeam() || newDir != getDir())
                canvas.setCannonProperties(Cannon.this, newTeam, newDir);
            return true;
        }
    }
}
