package org.xpilot.jxpmap;

import javax.swing.*;
import javax.swing.undo.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.AffineTransform;
import java.util.Iterator;
import java.util.HashMap;
import java.util.ArrayList;
import java.io.*;

public class MapCanvas extends JComponent {

    private MapModel model;
    private float scale;
    private AffineTransform at, it;
    private CanvasEventHandler eventHandler;
    private boolean erase;
    private Point offset;
    private UndoManager undoManager;

    public MapCanvas () {

        setOpaque(true);
        offset = new Point(0, 0);
        AwtEventHandler handler = new AwtEventHandler();
        addMouseListener(handler);
        addMouseMotionListener(handler);
        undoManager = new UndoManager();
    }


    public void setCanvasEventHandler (CanvasEventHandler h) {
        eventHandler = h;
    }
    
    
    public CanvasEventHandler getCanvasEventHandler () {
        return eventHandler;
    }

    public UndoManager getUndoManager() {
        return undoManager;
    }
    
    public MapModel getModel () {
        return model;
    }
    

    public void setModel (MapModel  m) {
        this.model = m;
        this.scale = m.getDefaultScale();
        this.at = null;
        this.it = null;
        eventHandler = null;
        repaint();
    }

    
    public float getScale () {
        return scale;
    }
    
    
    public void setScale (float  s) {
        this.scale = s;
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
    
    public void paint (Graphics _g) {
        
        if (model == null) return;

        Graphics2D g = (Graphics2D)_g;
        Dimension mapSize = model.options.size;

        g.setColor(Color.black);
        g.fill(g.getClipBounds());

        AffineTransform rootTx = new AffineTransform(g.getTransform());
        rootTx.concatenate(getTransform());
        g.setTransform(rootTx);

        g.setColor(Color.red);
        Rectangle world = new Rectangle(0, 0, mapSize.width, mapSize.height);
        g.draw(world);
        //drawShapeNoTx(g, world);
        

        Rectangle view = g.getClipBounds();
        Point min = new Point();
        Point max = new Point();

        for (int i = model.objects.size() - 1; i >= 0; i--) {
            MapObject o = (MapObject)model.objects.get(i);
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
        if (g.getClipBounds() == null) g.setClip(getBounds());
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

        AffineTransform tx = new AffineTransform();
        for (int xoff = min.x; xoff <= max.x; xoff++) {
            for (int yoff = min.y; yoff <= max.y; yoff++) {
                tx.setTransform(origTx);
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

    public Point[] computeWraps (Rectangle r, Point p) {
        Dimension mapSize = model.options.size;
        Point min = new Point();
        Point max = new Point();
        Rectangle b = new Rectangle(p.x, p.y, 0, 0);

        computeBounds(min, max, r, b, mapSize);
        Point[] wraps = new Point[(max.x - min.x + 1) * (max.y - min.y + 1)];
        int i = 0;
        for (int xoff = min.x; xoff <= max.x; xoff++) {
            for (int yoff = min.y; yoff <= max.y; yoff++) {
                wraps[i++] = new Point(xoff * mapSize.width + p.x,
                                       yoff * mapSize.height + p.y);
            }
        }
        return wraps;
    }

    public boolean containsWrapped (Rectangle r, Point p) {
        return computeWraps(r, p).length > 0;
    }
    
    public boolean containsWrapped (MapObject o, Point p) {
        Point[] wraps = computeWraps(o.getBounds(), p);
        for (int i = 0; i < wraps.length; i++)
            if (o.contains(wraps[i])) return true;
        return false;
    }

    public AffineTransform getTransform () {
        if (at == null) {
            at = new AffineTransform();
            at.translate(-scale * model.options.size.width / 2
                         + getSize().width / 2 - offset.x, 
                         scale * model.options.size.height / 2
                         + getSize().height / 2 - offset.y);
            at.scale(scale, -scale);
        }
        return at;
    }

    public AffineTransform getInverse () {
        try {
            if (it == null) it = getTransform().createInverse();
            return it;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private void doEdit (MapEdit e) {
        e.edit();
        getUndoManager().addEdit(e);
	repaint();
    }

    private abstract class MapEdit extends AbstractUndoableEdit {
	public abstract void unedit ();
	public abstract void edit ();
	public void undo () {
	    super.undo();
	    unedit();
	    repaint();
	}
	public void redo () {
	    super.redo();
	    edit();
	    repaint();
	}
    }

    public void setPolygonEdgeStyle (final MapPolygon p, 
                                     final int edgeIndex, 
                                     final LineStyle newStyle) {
        final LineStyle oldStyle = p.getEdgeStyle(edgeIndex);
        doEdit(new MapEdit() {
                public void unedit () {
                    p.setEdgeStyle(edgeIndex, oldStyle);
                }
                public void edit () {
                    p.setEdgeStyle(edgeIndex, newStyle);
                }
            });
    }

    public void removePolygonPoint (final MapPolygon p, final int i) {
        final Point point = p.getPoint(i);
        doEdit(new MapEdit() {
                public void unedit () {
                    p.insertPoint(i, point);
                }
                public void edit () {
                    p.removePoint(i);
                }
            });        
    }

    public void insertPolygonPoint (final MapPolygon p,
                                    final int i,
                                    final Point point) {
        doEdit(new MapEdit() {
                public void unedit () {
                    p.removePoint(i);
                }
                public void edit () {
                    p.insertPoint(i, point);
                }
            });                
    }

    public void movePolygonPoint (final MapPolygon p, 
                                  final int i,
                                  final int newX,
                                  final int newY) {
        final Point oldPoint = p.getPoint(i);
        doEdit(new MapEdit() {
                public void unedit () {
                    p.setPoint(i, oldPoint.x, oldPoint.y);
                }
                public void edit () {
                    p.setPoint(i, newX, newY);
                }
            });
    }

    public void setPolygonProperties (final MapPolygon p,
                                      final PolygonStyle newStyle,
                                      final int newType,
                                      final int newTeam) {
        final PolygonStyle oldStyle = p.getStyle();
        final int oldType = p.getType();
        final int oldTeam = p.getTeam();
        final PolygonStyle oldDefaultStyle = model.getDefaultPolygonStyle();
        doEdit(new MapEdit() {
                public void unedit () {
                    p.setStyle(oldStyle);
                    p.setType(oldType);
                    p.setTeam(oldTeam);
                    model.setDefaultPolygonStyle(oldDefaultStyle);
                }
                public void edit () {
                    p.setStyle(newStyle);
                    p.setType(newType);
                    p.setTeam(newTeam);
                    model.setDefaultPolygonStyle(newStyle);
                }
            });        
    }


    public void setBallTeam (final MapBall b, final int newTeam) {
        final int oldTeam = b.getTeam();
        doEdit(new MapEdit() {
                public void unedit () {
                    b.setTeam(oldTeam);
                }
                public void edit () {
                    b.setTeam(newTeam);
                }
            });
    }

    public void setBaseProperties (final MapBase b, 
                                   final int newTeam,
                                   final int newDir) {
        final int oldTeam = b.getTeam();
        final int oldDir = b.getDir();
        doEdit(new MapEdit() {
                public void unedit () {
                    b.setTeam(oldTeam);
                    b.setDir(oldDir);
                }
                public void edit () {
                    b.setTeam(newTeam);
                    b.setDir(newDir);
                }
            });
    }

    public void removeMapObject (final MapObject o) {
        final int i = getModel().indexOf(o);
        if (i == -1) return;
        doEdit(new MapEdit() {
                public void unedit () {
                    getModel().addObject(i, o);
                }
                public void edit () {
                    getModel().removeObject(i);
                }
            });
    }

    public void addMapObject (final MapObject o) {
        doEdit(new MapEdit() {
                public void unedit () {
                    getModel().removeObject(o);
                }
                public void edit () {
                    getModel().addToFront(o);
                }
            });
    }

    public void moveMapObject (final MapObject o, 
                               final int newX, 
                               final int newY) {
        final int oldX = o.getBounds().x;
        final int oldY = o.getBounds().y;
        doEdit(new MapEdit() {
                public void unedit () {
                    o.moveTo(oldX, oldY);
                }
                public void edit () {
                    o.moveTo(newX, newY);
                }
            });
    }

    public void addEdgeStyle (final LineStyle style) {
        doEdit(new MapEdit() {
                public void unedit () {
                    getModel().edgeStyles.remove(style);
                }
                public void edit () {
                    getModel().edgeStyles.add(style);
                }
            });
    }

    public void removeEdgeStyle (final LineStyle style) {
	final java.util.List oldStyles = getModel().edgeStyles;
        doEdit(new MapEdit() {
                public void unedit () {
		    getModel().edgeStyles = oldStyles;
                }
                public void edit () {
                    getModel().edgeStyles.remove(style);
                }
            });
    }

    public void addPolygonStyle (final PolygonStyle style) {
        doEdit(new MapEdit() {
                public void unedit () {
                    getModel().polyStyles.remove(style);
                }
                public void edit () {
                    getModel().polyStyles.add(style);
                }
            });
    }

    public void removePolygonStyle (final PolygonStyle style) {
	final java.util.List oldStyles = new ArrayList(getModel().polyStyles);
        doEdit(new MapEdit() {
                public void unedit () {
                    getModel().polyStyles = oldStyles;
                }
                public void edit () {
                    getModel().polyStyles.remove(style);
                }
            });
    }

    public void setEdgeStyleProperties (final LineStyle style,
                                        final String newId,
                                        final int newStyle,
                                        final int newWidth,
                                        final Color newColor) {
        final String oldId = style.getId();
        final int oldStyle = style.getStyle();
        final int oldWidth = style.getWidth();
        final Color oldColor = style.getColor();
        doEdit(new MapEdit() {
                public void unedit () {
                    set(oldId, oldStyle, oldWidth, oldColor);
                }
                public void edit () {
                    set(newId, newStyle, newWidth, newColor);
                }
                private void set (String id, int s, int w, Color c) {
                    style.setId(id);
                    style.setStyle(s);
                    style.setWidth(w);
                    style.setColor(c);
                }
            });
    }


    public void setPolygonStyleProperties (final PolygonStyle style,
                                           final String newId,
                                           final int newFillStyle,
                                           final Color newColor,
                                           final Pixmap newTexture,
                                           final LineStyle newDefEdgeStyle,
                                           final boolean newVisible,
                                           final boolean newVisInRadar) {
        final String oldId = style.getId();
        final int oldFillStyle = style.getFillStyle();
        final Color oldColor = style.getColor();
        final Pixmap oldTexture = style.getTexture();
        final LineStyle oldDefEdgeStyle = style.getDefaultEdgeStyle();
        final boolean oldVisible = style.isVisible();
        final boolean oldVisInRadar = style.isVisibleInRadar();
        doEdit(new MapEdit() {
                public void unedit () {
                    set(oldId, oldFillStyle, oldColor, oldTexture, 
                        oldDefEdgeStyle, oldVisible, oldVisInRadar);
                }
                public void edit () {
                    set(newId, newFillStyle, newColor, newTexture, 
                        newDefEdgeStyle, newVisible, newVisInRadar);
                }
                private void set (String id, int fs, Color c, Pixmap t, 
                                  LineStyle es, boolean v, boolean vr) {
                    style.setId(id);
                    style.setFillStyle(fs);
                    style.setColor(c);
                    style.setTexture(t);
                    style.setDefaultEdgeStyle(es);
                    model.setDefaultEdgeStyle(es);
                    style.setVisible(v);
                    style.setVisibleInRadar(vr);
                }
            });
    }
    
    private class AwtEventHandler implements CanvasEventHandler {

        private Point dragStart;
        private Point offsetStart;
        
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

            dragStart = evt.getPoint();
            offsetStart = new Point(offset);

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
            at = null;
            it = null;
            offset.x = offsetStart.x + (dragStart.x - evt.getX());
            offset.y = offsetStart.y + (dragStart.y - evt.getY());
            repaint();
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
