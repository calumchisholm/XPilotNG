package org.xpilot.jxpmap;

import java.awt.AWTEvent;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.Stroke;
import java.awt.TexturePaint;
import java.awt.event.InputEvent;
import java.awt.event.MouseEvent;
import java.awt.geom.AffineTransform;
import java.awt.geom.Line2D;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;


public class MapPolygon extends MapObject {

    protected Polygon polygon;
    protected PolygonStyle style;
    protected ArrayList edgeStyles;

    public Object deepClone (Map context) {

        MapPolygon clone = (MapPolygon)super.deepClone(context);

        clone.polygon = new Polygon(polygon.xpoints, 
            polygon.ypoints, 
            polygon.npoints);

        clone.style = (PolygonStyle)style.deepClone(context);

        if (edgeStyles != null) {
            ArrayList l = new ArrayList();
            for (Iterator i = edgeStyles.iterator(); i.hasNext();)
                l.add(((LineStyle)i.next()).deepClone(context));
            clone.edgeStyles = l;
        }

        return clone;
    }
    
    public MapObject copy() {
        MapPolygon copy = (MapPolygon)super.copy();
        copy.polygon = new Polygon(polygon.xpoints, 
            polygon.ypoints, 
            polygon.npoints);
        if (edgeStyles != null) 
            copy.edgeStyles = new ArrayList(edgeStyles);
        return copy;
    }

    public MapPolygon () {}


    public MapPolygon (Polygon p, PolygonStyle style, ArrayList edgeStyles) {
        this.polygon = p;
        this.style = style;
        this.edgeStyles = edgeStyles;
    }

    
    public CanvasEventHandler getCreateHandler (Runnable r) {
        return new CreateHandler(r);
    }
    
    protected MapObjectPopup createPopup() {
        return new PolygonPopup(this);
    }

    public PolygonStyle getStyle () {
        return style;
    }


    public void setStyle (PolygonStyle style) {
        this.style = style;
    }

    public Rectangle getBounds () {
        return polygon.getBounds();
    }


    public boolean contains (Point p) {
        return polygon.contains(p);
    }


    public int getZOrder () {
        return 20;
    }


    public Shape getPreviewShape () {
        return polygon;
    }


    public void moveTo (int x, int y) {
        Rectangle r = getBounds();
        polygon.translate(x - r.x, y - r.y);
    }
    
    
    public void rotate (double angle) {
        double cx = getBounds().getCenterX();
        double cy = getBounds().getCenterY();
        AffineTransform at = AffineTransform.getTranslateInstance(cx, cy);
        at.rotate(angle);
        at.translate(-cx, -cy);
        Point p = new Point();
        for (int i = 0; i < polygon.npoints; i++) {
            p.x = polygon.xpoints[i];
            p.y = polygon.ypoints[i];
            at.transform(p, p);
            polygon.xpoints[i] = p.x;
            polygon.ypoints[i] = p.y;
        }
    }


    public boolean isCounterClockwise () {

        long xi,yi,xj,yj,area = 0;
        int i, j;

        for (i = polygon.npoints - 1, j = 0; j < polygon.npoints; i = j, j++) {
            xi = polygon.xpoints[i];
            xj = polygon.xpoints[j];
            yi = polygon.ypoints[i];
            yj = polygon.ypoints[j];
            area += xi * yj - xj * yi;
        }

        return (area > 0);
    }


    public void setEdgeStyle (int index, LineStyle style) {
        if (edgeStyles == null) {
            if (style == null) return;
            edgeStyles = new ArrayList(polygon.npoints);
            for (int i = 0; i < polygon.npoints; i++)
                edgeStyles.add(null);
        }
        edgeStyles.set(index, style);
    }


    public LineStyle getEdgeStyle (int index) {
        if (edgeStyles == null) return style.getDefaultEdgeStyle();
        return (LineStyle)edgeStyles.get(index);
    }

    
    public void printXml (PrintWriter out) throws IOException {

        if (polygon.npoints < 2) return;

        out.print("<Polygon x=\"");
        out.print(polygon.xpoints[0]);
        out.print("\" y=\"");
        out.print(polygon.ypoints[0]);
        out.print("\" style=\"");
        out.print(getStyle().getId());
        out.println("\">");

        LineStyle cls = null;

        if (isCounterClockwise()) {
            for (int i = 1; i <= polygon.npoints; i++) {
                out.print("<Offset x=\"");
                out.print(polygon.xpoints[i % polygon.npoints] - 
                          polygon.xpoints[i - 1]);
                out.print("\" y=\"");
                out.print(polygon.ypoints[i % polygon.npoints] - 
                          polygon.ypoints[i - 1]);
                
                if (edgeStyles != null) {
                    LineStyle ls = getEdgeStyle(i - 1);
                    if (ls != cls) {
                        out.print("\" style=\"");
                        out.print((ls != null) ? ls.getId() : 
                                  style.getDefaultEdgeStyle().getId());
                        cls = ls;
                    }
                }
                out.println("\"/>");
            }

        } else {
            for (int i = polygon.npoints; i > 0; i--) {
                out.print("<Offset x=\"");
                out.print(polygon.xpoints[i - 1] - 
                          polygon.xpoints[i % polygon.npoints]);
                out.print("\" y=\"");
                out.print(polygon.ypoints[i - 1] - 
                          polygon.ypoints[i % polygon.npoints]);
                
                if (edgeStyles != null) {
                    LineStyle ls = getEdgeStyle(i - 1);
                    if (ls != cls) {
                        out.print("\" style=\"");
                        out.print((ls != null) ? ls.getId() : 
                                  style.getDefaultEdgeStyle().getId());
                        cls = ls;
                    }
                }
                out.println("\"/>");
            }
        }

        out.println("</Polygon>");

    }
    

    public void paint (Graphics2D g, float scale) {

        if (!style.isVisible()) return;

        Polygon p = polygon;

        if (style.getFillStyle() == PolygonStyle.FILL_COLOR) {
            g.setColor(style.getColor());
            g.fillPolygon(p);
            
        } else if (style.getFillStyle() == PolygonStyle.FILL_TEXTURED) {
            BufferedImage img = style.getTexture().getImage();
            Rectangle b = polygon.getBounds();
            g.setPaint(new TexturePaint(img, new Rectangle
                (b.x, b.y, img.getWidth() * 64, -img.getHeight() * 64)));
            g.fill(p);
        }

        if (edgeStyles == null) {
            LineStyle def = style.getDefaultEdgeStyle();
            if (def.getStyle() != LineStyle.STYLE_HIDDEN) {
                g.setColor(def.getColor());
                g.setStroke(def.getStroke(scale));
                g.draw(p);
            }
        } else {
            for (int i = 0; i < p.npoints;) {
                int begin = i;
                LineStyle ls = getEdgeStyle(i++);
                
                while (i < p.npoints && getEdgeStyle(i) == ls) i++;
                if (ls == null) ls = style.getDefaultEdgeStyle();
                if (i == p.npoints && 
                    ls == style.getDefaultEdgeStyle() && 
                    begin == 0) edgeStyles = null;
                
                if (ls.getStyle() != LineStyle.STYLE_HIDDEN) {
                    g.setColor(ls.getColor());
                    g.setStroke(ls.getStroke(scale));
                    
                    int nump = i - begin + 1;
                    int[] xp = new int[nump];
                    int[] yp = new int[nump];
                    int aclen = nump;
                    
                    if (begin + nump > p.npoints) {
                        aclen--;
                        xp[aclen] = p.xpoints[0];
                        yp[aclen] = p.ypoints[0];
                    }

                    System.arraycopy(p.xpoints, begin, xp, 0, aclen);
                    System.arraycopy(p.ypoints, begin, yp, 0, aclen);
                    
                    g.drawPolyline(xp, yp, nump);
                }
            }
        }
    }


    public EditorPanel getPropertyEditor (MapCanvas canvas) {
        return new PolygonPropertyEditor(canvas, this);
    }
    
    public CanvasEventHandler getRotateHandler() {
        return new RotateHandler();
    }


    public boolean checkAwtEvent (MapCanvas canvas, AWTEvent evt) {

        if (evt.getID() == MouseEvent.MOUSE_PRESSED ||
            evt.getID() == MouseEvent.MOUSE_CLICKED) {

            MouseEvent me = (MouseEvent)evt;
            Rectangle b = getBounds();
            Point p = me.getPoint();
            Polygon pl = polygon;
            Rectangle larger = 
                new Rectangle(b.x - 20 * 64,
                              b.y - 20 * 64,
                              b.width + 40 * 64,
                              b.height + 40 * 64);
            Point[] wraps = canvas.computeWraps(larger, p);
            if (wraps.length == 0) return false;
            
            double thresholdSq = 100 / (canvas.getScale() * canvas.getScale());
            if ((me.getModifiers() & InputEvent.BUTTON1_MASK) != 0
                && me.getID() == MouseEvent.MOUSE_PRESSED) {
                
                for (int pi = 0; pi < wraps.length; pi++) {
                    Point wp = wraps[pi];
                    for (int i = 0; i < pl.npoints; i++) {
                        if (Point.distanceSq
                            (wp.x, wp.y, pl.xpoints[i], 
                             pl.ypoints[i]) < thresholdSq) {
                            
                            if (canvas.isErase()) {
                                canvas.removePolygonPoint(this, i);
                            } else {
                                canvas.setCanvasEventHandler
                                    (new PolygonPointMoveHandler(me, i));
                            }
                            return true;
                        }
                    }
                }
            }
                
            if (!canvas.isErase()) {
                    
                for (int pi = 0; pi < wraps.length; pi++) {
                    Point wp = wraps[pi];
                    for (int i = 0; i < pl.npoints; i++) {
                        int ni = (i + 1) % pl.npoints;
                        if (Line2D.ptSegDistSq(pl.xpoints[i],
                                               pl.ypoints[i],
                                               pl.xpoints[ni],
                                               pl.ypoints[ni],
                                               wp.x, wp.y) < thresholdSq) {
                            
                            if ((me.getModifiers() & 
                                 InputEvent.BUTTON1_MASK) != 0) {
                                
                                if (me.getID() == MouseEvent.MOUSE_PRESSED) {
                                    canvas.insertPolygonPoint(this, i + 1, p);
                                    canvas.setCanvasEventHandler
                                        (new PolygonPointMoveHandler
                                         (me, i + 1));
                                    return true;
                                }
                                
                            } else {
                                
                                if (me.getID() == MouseEvent.MOUSE_CLICKED) {
                                    EditorDialog.show
                                        (canvas, new EdgePropertyEditor
                                         (canvas, this, i),
                                         true,
                                         EditorDialog.OK_CANCEL);
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return super.checkAwtEvent(canvas, evt);
    }


    public void removePoint (int i) {
        
        int pc = polygon.npoints - 1;
        int[] xps = remove(polygon.xpoints, i);
        int[] yps = remove(polygon.ypoints, i);
        
        polygon = new Polygon(xps, yps, pc);
        
        if (edgeStyles != null) {
            if (i == 0) edgeStyles.remove(edgeStyles.size() - 1);
            else edgeStyles.remove(i - 1);
        }
    }
    
    
    public int getNumPoints() {
        return polygon.npoints;
    }


    public Point getPoint (int i) {
        if (i < 0) throw new IllegalArgumentException("i = " + i);
        return new Point(polygon.xpoints[i], polygon.ypoints[i]);
    }

    public void setPoint (int i, int x, int y) {
        if (i < 0 || i >= polygon.npoints)
            throw new IllegalArgumentException("i = " + i);
        polygon.xpoints[i] = x;
        polygon.ypoints[i] = y;
        polygon.invalidate();
    }
    
    public void insertPoint (int i, Point p) {
        if (i < 0) throw new IllegalArgumentException("i = " + i);
        int pc = polygon.npoints + 1;
        int xps[] = insert(p.x, polygon.xpoints, i);
        int yps[] = insert(p.y, polygon.ypoints, i);
        
        polygon = new Polygon(xps, yps, pc);
        
        if (edgeStyles != null) {
            LineStyle es = getEdgeStyle(i - 1);
            edgeStyles.add(i - 1, es);
        }
    }


    private static int[] remove (int a[], int p) {

        if (p >= a.length || p < 0) 
            throw new ArrayIndexOutOfBoundsException(p);

        int rv[] = new int[a.length - 1];
        if (p > 0) System.arraycopy(a, 0, rv, 0, p);
        if (p < rv.length) System.arraycopy(a, p + 1, rv, p, rv.length - p);

        return rv;
    }


    private static int[] insert (int i, int a[], int p) {

        if (p > a.length || p < 0) throw new ArrayIndexOutOfBoundsException(p);
        
        int rv[] = new int[a.length + 1];
        System.arraycopy(a, 0, rv, 0, p);
        rv[p] = i;
        if (p < a.length) System.arraycopy(a, p, rv, p + 1, a.length - p);

        return rv;
    }


    private class CreateHandler extends CanvasEventAdapter {

        private Runnable cmd;
        private Polygon poly;
        private Point latest, first;
        private Stroke stroke;
        private boolean remove;


        public CreateHandler (Runnable cmd) {
            this.cmd = cmd;
        }

        
        public void mousePressed (MouseEvent me) {

            if (poly == null) {
                poly = new Polygon();
                first = me.getPoint();
                poly.addPoint(first.x, first.y);
                poly.addPoint(0, 0);
                return;
            }

            MapCanvas c = (MapCanvas)me.getSource();
            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            if (stroke == null) stroke = getPreviewStroke(c.getScale());
            //g.setStroke(stroke);

            if (remove) {
                c.drawShape(g, poly);
                remove = false;
            }

            latest = me.getPoint();

            if (poly.npoints > 2) {
                if ((me.getModifiers() & InputEvent.BUTTON1_MASK) == 0) {
                    poly.addPoint(latest.x, latest.y);
                    MapPolygon.this.polygon = 
                        new Polygon(poly.xpoints, 
                                    poly.ypoints, 
                                    poly.npoints - 1);
                    MapPolygon.this.style = 
                        c.getModel().getDefaultPolygonStyle();
                    c.addMapObject(MapPolygon.this);
                    c.setCanvasEventHandler(newInstance().getCreateHandler(cmd));
                    if (cmd != null) cmd.run();
                    return;
                }
            }
            
            poly.addPoint(latest.x, latest.y);
            c.drawShape(g, poly);
            remove = true;
        }


        public void mouseMoved (MouseEvent me) {
            
            if (poly == null) return;

            MapCanvas c = (MapCanvas)me.getSource();
            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            if (stroke == null) stroke = getPreviewStroke(c.getScale());
            //g.setStroke(stroke);
            if (remove) c.drawShape(g, poly);
            
            int li = poly.npoints - 1;
            poly.xpoints[li] = me.getX();
            poly.ypoints[li] = me.getY();
            
            c.drawShape(g, poly);
            remove = true;
        }
        
    }


    private class PolygonPointMoveHandler extends CanvasEventAdapter {


        private int index;
        private Line2D.Float l1, l2;
        private boolean virgin;
        private Stroke stroke;
        

        public PolygonPointMoveHandler (MouseEvent me, int index) {

            this.index = index;
            virgin = true;

            int ip = index - 1;
            if (ip < 0) ip += polygon.npoints;
            Point prev = new Point(polygon.xpoints[ip], polygon.ypoints[ip]);
            l1 = new Line2D.Float(prev, me.getPoint());
            
            int in = (index + 1) % polygon.npoints;
            Point next = new Point(polygon.xpoints[in], polygon.ypoints[in]);
            l2 = new Line2D.Float(next, me.getPoint());

            
            float dash[] = { 10.0f };
            stroke = new BasicStroke(1, BasicStroke.CAP_BUTT, 
                                     BasicStroke.JOIN_MITER, 
                                     10.0f, dash, 0.0f);
        }


        public void mouseDragged (MouseEvent evt) {

            MapCanvas c = (MapCanvas)evt.getSource();
            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            //g.setStroke(stroke);

            if (!virgin) drawPreview(g, c);
            else virgin = false;

            l1.x2 = l2.x2 = evt.getX();
            l1.y2 = l2.y2 = evt.getY();
            
            drawPreview(g, c);
        }
        

        public void mouseReleased (MouseEvent evt) {

            MapCanvas c = (MapCanvas)evt.getSource();
            c.movePolygonPoint(MapPolygon.this, index, evt.getX(), evt.getY());
            c.setCanvasEventHandler(null);
        }


        private void drawPreview (Graphics2D g, MapCanvas c) {
            c.drawShape(g, l1);
            c.drawShape(g, l2);
        }
    }
    
    
    private class RotateHandler extends CanvasEventAdapter {
        
        private double cx, cy, angle;
        private Stroke stroke;
        private Shape origShape;
        private Shape previewShape;
        private AffineTransform backTx;
        private boolean firstTime;
        
        public RotateHandler() {
            cx = getBounds().getCenterX();
            cy = getBounds().getCenterY();
            origShape = AffineTransform.getTranslateInstance(
                -cx , -cy).createTransformedShape(polygon);
            backTx = AffineTransform.getTranslateInstance(cx, cy);
            updatePreviewShape();
            firstTime = true;
        }
        
        public void mouseMoved(MouseEvent me) {
            MapCanvas c = (MapCanvas)me.getSource();            
            if (!firstTime) drawPreview(c);
            angle = 2 * Math.PI * (cx - me.getX()) / (800 / c.getScale());
            updatePreviewShape();
            drawPreview(c);
            firstTime = false;
        }
        
        public void mousePressed(MouseEvent me) {
            MapCanvas c = (MapCanvas)me.getSource();
            if (!firstTime) drawPreview(c);
            c.rotatePolygon(MapPolygon.this, angle);
            c.setCanvasEventHandler(null);
        }
        
        private void updatePreviewShape() {
            AffineTransform at = AffineTransform.getRotateInstance(angle);
            at.preConcatenate(backTx);
            previewShape = at.createTransformedShape(origShape);
        }
                
        private void drawPreview(MapCanvas c) {
            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            if (stroke == null) stroke = getPreviewStroke(c.getScale());
            //g.setStroke(stroke);
            c.drawShape(g, previewShape);
        }
    }    
}
