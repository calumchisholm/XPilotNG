package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.util.List;
import java.util.ArrayList;

import java.io.PrintWriter;
import java.io.IOException;


public class MapPolygon extends MapObject {

    public static final int TYPE_NORMAL = 0;
    public static final int TYPE_BALLAREA = 1;
    public static final int TYPE_BALLTARGET = 2;


    protected Polygon polygon;
    protected PolygonStyle style;
    protected ArrayList edgeStyles;
    protected int type;
    protected int team;


    public MapPolygon () {}


    public MapPolygon (Polygon p, PolygonStyle style, ArrayList edgeStyles) {
        this.polygon = p;
        this.style = style;
        this.edgeStyles = edgeStyles;
    }

    
    public CanvasEventHandler getCreateHandler (Runnable r) {
        return new CreateHandler(r);
    }


    public PolygonStyle getStyle () {
        return style;
    }


    public void setStyle (PolygonStyle style) {
        this.style = style;
    }


    public int getType () {
        return type;
    }


    public void setType (int type) {
        this.type = type;
    }


    public int getTeam () {
        return team;
    }


    public void setTeam (int team) {
        this.team = team;
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

    
    public void printXml (PrintWriter out) throws IOException {

        if (polygon.npoints == 0) return;

        if (getType() == TYPE_BALLAREA) {
            out.print("<BallArea team=\"");
            out.print(getTeam());
            out.println("\">");
        } else if (getType() == TYPE_BALLTARGET) {
            out.print("<BallTarget team=\"");
            out.print(getTeam());
            out.println("\">");
        }

        out.print("<Polygon x=\"");
        out.print(polygon.xpoints[0]);
        out.print("\" y=\"");
        out.print(polygon.ypoints[0]);
        out.print("\" style=\"");
        out.print(getStyle().getId());
        out.println("\">");

        for (int i = 1; i < polygon.npoints; i++) {
            out.print("<Offset x=\"");
            out.print(polygon.xpoints[i] - polygon.xpoints[i - 1]);
            out.print("\" y=\"");
            out.print(polygon.ypoints[i] - polygon.ypoints[i - 1]);
            
            LineStyle es = getEdgeStyle(i - 1);
            if (es != null && es != getStyle().getDefaultEdgeStyle()) {
                out.print("\" style=\"");
                out.print(es.getId());
            }
            out.println("\"/>");
        }

        out.println("</Polygon>");

        if (getType() == TYPE_BALLAREA) {
            out.println("</BallArea>");
        } else if (getType() == TYPE_BALLTARGET) {
            out.println("</BallTarget>");
        }
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
            for (int i = 0; i < p.npoints - 1;) {
                int begin = i;
                LineStyle ls = (LineStyle)getEdgeStyle(i++);
                
                while (i < p.npoints - 1 && getEdgeStyle(i) == ls) i++;
                
                if (ls == null) ls = style.getDefaultEdgeStyle();
                
                if (ls.getStyle() != LineStyle.STYLE_HIDDEN) {
                    g.setColor(ls.getColor());
                    g.setStroke(ls.getStroke(scale));
                    
                    int nump = i + 1 - begin;
                    int[] xp = new int[nump];
                    int[] yp = new int[nump];

                    System.arraycopy(p.xpoints, begin, xp, 0, nump);
                    System.arraycopy(p.ypoints, begin, yp, 0, nump);

                    g.drawPolyline(xp, yp, nump);
                }
            }
        }
    }


    public void setEdgeStyle (int index, LineStyle style) {
        if (edgeStyles == null) edgeStyles = new ArrayList();
        while (edgeStyles.size() < index + 1) edgeStyles.add(null);
        edgeStyles.set(index, style);
    }


    public LineStyle getEdgeStyle (int index) {
        return 
            (edgeStyles == null || index > edgeStyles.size() - 1) ?
            style.getDefaultEdgeStyle() : 
            (LineStyle)edgeStyles.get(index);
    }


    public EditorPanel getPropertyEditor (MapCanvas canvas) {
        return new PolygonPropertyEditor(canvas, this);
    }


    public boolean checkAwtEvent (MapCanvas canvas, AWTEvent evt) {

        if (evt.getID() == MouseEvent.MOUSE_PRESSED ||
            evt.getID() == MouseEvent.MOUSE_CLICKED) {

            MouseEvent me = (MouseEvent)evt;
            Rectangle b = getBounds();
            Point p = me.getPoint();
            Polygon pl = polygon;


            if ((me.getModifiers() & InputEvent.BUTTON1_MASK) != 0
                && me.getID() == me.MOUSE_PRESSED) {
                
                if (b.x - 20 * 64 > p.x
                    || b.y - 20 * 64 > p.y
                    || b.x + b.width + 20 * 64 < p.x
                    || b.y + b.height + 20 * 64 < p.y) return false;
                
                for (int i = 0; i < pl.npoints; i++) {
                    if (Point.distanceSq
                        (p.x, p.y, pl.xpoints[i], 
                         pl.ypoints[i]) < 25 * 64 * 64) {
                        
                        if (canvas.isErase()) {
                            int pc = pl.npoints - 1;
                            int[] xps = remove(pl.xpoints, i);
                            int[] yps = remove(pl.ypoints, i);
                            
                            polygon = new Polygon(xps, yps, pc);
                            canvas.repaint();
                            
                        } else {
                            canvas.setCanvasEventHandler
                                (new PolygonPointMoveHandler(me, i));
                        }
                        return true;
                    }
                }
            }
                
            if (!canvas.isErase()) {
                    
                for (int i = 0; i < pl.npoints; i++) {
                        
                    int ni = (i + 1) % pl.npoints;
                    if (Line2D.ptSegDistSq(pl.xpoints[i],
                                           pl.ypoints[i],
                                           pl.xpoints[ni],
                                           pl.ypoints[ni],
                                           p.x, p.y) < 25 * 64 * 64) {

                        if ((me.getModifiers() & 
                             InputEvent.BUTTON1_MASK) != 0) {
                            
                            if (me.getID() == me.MOUSE_PRESSED) {
                            
                                int pc = pl.npoints + 1;
                                int xps[] = insert(p.x, pl.xpoints, i + 1);
                                int yps[] = insert(p.y, pl.ypoints, i + 1);
                                
                                polygon = new Polygon(xps, yps, pc);
                                
                                canvas.setCanvasEventHandler
                                    (new PolygonPointMoveHandler(me, i + 1));
                                return true;
                            }

                        } else {

                            if (me.getID() == me.MOUSE_CLICKED) {
                                EditorDialog.show
                                    (canvas,
                                     new EdgePropertyEditor(canvas, this, i),
                                     true,
                                     EditorDialog.OK_CANCEL);
                                return true;
                            }
                        }
                    }
                }
            }
        }

        return super.checkAwtEvent(canvas, evt);
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

        if (p >= a.length || p < 0) 
            throw new ArrayIndexOutOfBoundsException(p);

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
            g.setStroke(stroke);

            if (remove) {
                c.drawShape(g, poly);
                remove = false;
            }

            latest = me.getPoint();

            if (poly.npoints > 2) {
                if (first.distanceSq(latest) < 100 * 64 * 64) {
                    poly.xpoints[poly.npoints - 1] = first.x;
                    poly.ypoints[poly.npoints - 1] = first.y;
                    MapPolygon.this.polygon = poly;
                    MapPolygon.this.style = 
                        c.getModel().getDefaultPolygonStyle();
                    
                    c.getModel().addObject(MapPolygon.this);
                    c.setCanvasEventHandler(null);
                    if (cmd != null) cmd.run();
                    c.repaint();
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
            g.setStroke(stroke);
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
            g.setStroke(stroke);

            if (!virgin) drawPreview(g, c);
            else virgin = false;

            l1.x2 = l2.x2 = evt.getX();
            l1.y2 = l2.y2 = evt.getY();
            
            drawPreview(g, c);
        }
        

        public void mouseReleased (MouseEvent evt) {
            
            polygon.xpoints[index] = evt.getX();
            polygon.ypoints[index] = evt.getY();
            Polygon p = new Polygon(polygon.xpoints, 
                                    polygon.ypoints, 
                                    polygon.npoints);
            polygon = p;
            MapCanvas c = (MapCanvas)evt.getSource();
            c.setCanvasEventHandler(null);
            c.repaint();
        }


        private void drawPreview (Graphics2D g, MapCanvas c) {
            c.drawShape(g, l1);
            c.drawShape(g, l2);
        }
    }    
}
