package org.xpilot.jxpmap;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.AffineTransform;
import java.util.Iterator;

public class MapCanvas extends JComponent {

    private MapModel model;
    private float scale;
    private Dimension prefSize;
    private AffineTransform at, it;
    private CanvasEventHandler eventHandler;
    private boolean erase;


    public MapCanvas () {

        setOpaque(true);
        prefSize = new Dimension(0, 0);

        AwtEventHandler handler = new AwtEventHandler();
        addMouseListener(handler);
        addMouseMotionListener(handler);
    }


    public void setCanvasEventHandler (CanvasEventHandler h) {
        eventHandler = h;
    }
    
    
    public CanvasEventHandler getCanvasEventHandler () {
        return eventHandler;
    }
    
    
    public MapModel getModel () {
        return model;
    }
    

    public void setModel (MapModel  m) {
        this.model = m;
        this.scale = m.getDefaultScale();
        prefSize = null;
        this.at = null;
        this.it = null;
        eventHandler = null;
        revalidate();
    }

    
    public float getScale () {
        return scale;
    }
    
    
    public void setScale (float  s) {
        this.scale = s;
        this.prefSize = null;
        this.at = null;
        this.it = null;
        eventHandler = null;
        revalidate();
    }


    public boolean isErase () {
        return erase;
    }


    public void setErase (boolean erase) {
        this.erase = erase;
    }


    public Dimension getPreferredSize () {

        if (prefSize == null) {
            if (model != null) {
                Dimension d = model.options.size;
                int ew = model.options.edgeWrap ? 2 : 1;
                prefSize = new Dimension
                ((int)(ew * d.width * scale), 
                 (int)(ew * d.height * scale));
            } else {
                prefSize = new Dimension(0, 0);
            }
        }
        return prefSize;
    }
    
    
    public void paint (Graphics _g) {
        
        if (model == null) return;

        Graphics2D g = (Graphics2D)_g;
        Dimension mapSize = model.options.size;

        g.setColor(Color.black);
        g.fill(g.getClipBounds());

        AffineTransform rootTx = new AffineTransform(g.getTransform());
        rootTx.concatenate(getTransform());
        g.setTransform(rootTx);

        g.setColor(Color.blue);
        Rectangle world = new Rectangle(0, 0, mapSize.width, mapSize.height);
        drawShapeNoTx(g, world);
        

        Rectangle view = g.getClipBounds();
        Point min = new Point();
        Point max = new Point();

        for (Iterator iter = model.objects.iterator(); iter.hasNext();) {

            MapObject o = (MapObject)iter.next();
            computeBounds(min, max, view, o.getBounds(), mapSize);
            
            for (int xoff = min.x; xoff <= max.x; xoff++) {
                for (int yoff = min.y; yoff <= max.y; yoff++) {
                    
                    AffineTransform tx = new AffineTransform(rootTx);
                    tx.translate(xoff * mapSize.width, yoff * mapSize.height);
                    g.setTransform(tx);
                    o.paint(g, scale);
                }
            }
        }
        g.setTransform(rootTx);
    }

    
    public void drawShape (Graphics2D g, Shape s) {
        if (g.getClipBounds() == null) {
            Rectangle clip = 
                (getParent() instanceof JViewport) ?
                ((JViewport)getParent()).getViewRect() :
                getBounds();
            g.setClip(clip);
        }
        AffineTransform origTx = g.getTransform();
        AffineTransform rootTx = new AffineTransform(origTx);
        rootTx.concatenate(getTransform());
        g.setTransform(rootTx);
        drawShapeNoTx(g, s);
        g.setTransform(origTx);
    }


    private void drawShapeNoTx (Graphics2D g, Shape s) {

        Rectangle view = g.getClipBounds();
        AffineTransform origTx = g.getTransform();
        Point min = new Point();
        Point max = new Point();
        Dimension mapSize = model.options.size;

        computeBounds(min, max, view, s.getBounds(), mapSize);

        for (int xoff = min.x; xoff <= max.x; xoff++) {
            for (int yoff = min.y; yoff <= max.y; yoff++) {
                
                AffineTransform tx = new AffineTransform(origTx);
                tx.translate(xoff * mapSize.width, yoff * mapSize.height);
                g.setTransform(tx);
                g.draw(s);
            }
        }

        g.setTransform(origTx);
    }


    private void computeBounds (Point min, Point max, 
                                Rectangle view, Rectangle b, 
                                Dimension map) {
        
        min.x = (view.x - (b.x + b.width)) / map.width;
        if (view.x > b.x + b.width) min.x++;
        max.x = (view.x + view.width - b.x) / map.width;
        if (view.x + view.width < b.x) max.x--;
        min.y = (view.y - (b.y + b.height)) / map.height;
        if (view.y > b.y + b.height) min.y++;
        max.y = (view.y + view.height - b.y) / map.height;
        if (view.y + view.height < b.y) max.y--;

    }

    private AffineTransform getTransform () {
        if (at == null) {
            at = new AffineTransform();
            at.translate(0, scale * model.options.size.height);
            at.scale(scale, -scale);
        }
        return at;
    }


    private AffineTransform getInverse () {
        try {
            if (it == null) it = getTransform().createInverse();
            return it;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    
    private class AwtEventHandler implements CanvasEventHandler {

        
        public void mouseClicked (MouseEvent evt) {

            if (model == null) return;
            transformEvent(evt);
            
            if (eventHandler != null) {
                eventHandler.mouseClicked(evt);
                return;
            }

            for (Iterator iter = model.objects.iterator();
                 iter.hasNext();) {
                MapObject o = (MapObject)iter.next();
                if (o.checkAwtEvent(MapCanvas.this, evt)) {
                    return;
                }
            }
        }

        public void mouseEntered (MouseEvent evt) {
            if (model == null) return;
            if (eventHandler != null) {
                transformEvent(evt);
                eventHandler.mouseEntered(evt);
                return;
            }
        }

        public void mouseExited (MouseEvent evt) {
            if (model == null) return;
            if (eventHandler != null) {
                transformEvent(evt);
                eventHandler.mouseExited(evt);
                return;
            }
        }

        public void mousePressed (MouseEvent evt) {

            if (model == null) return;
            transformEvent(evt);

            if (eventHandler != null) {
                eventHandler.mousePressed(evt);
                return;
            }

            for (Iterator iter = model.objects.iterator();
                 iter.hasNext();) {
                MapObject o = (MapObject)iter.next();
                if (o.checkAwtEvent(MapCanvas.this, evt)) {
                    return;
                }
            }
        }

        public void mouseReleased (MouseEvent evt) {
            if (model == null) return;
            if (eventHandler != null) {
                transformEvent(evt);
                eventHandler.mouseReleased(evt);
                return;
            }
        }

        public void mouseDragged (MouseEvent evt) {
            if (model == null) return;
            if (eventHandler != null) {
                transformEvent(evt);
                eventHandler.mouseDragged(evt);
                return;
            }
        }

        public void mouseMoved (MouseEvent evt) {
            if (model == null) return;
            if (eventHandler != null) {
                transformEvent(evt);
                eventHandler.mouseMoved(evt);
                return;
            }
        }

        private void transformEvent (MouseEvent evt) {
            Point mapp = new Point();
            Point evtp = evt.getPoint();
            getInverse().transform(evtp, mapp);
            evt.translatePoint(mapp.x - evtp.x, mapp.y - evtp.y);
        }
    }

    
}
