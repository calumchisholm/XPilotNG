package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.AffineTransform;
import javax.swing.*;
import java.io.PrintWriter;
import java.io.IOException;
import java.util.Map;


public abstract class MapObject extends ModelObject {
    
    protected Rectangle bounds;
    protected Stroke previewStroke;
    protected Image img;
    protected MapObjectPopup popup;
    
    public Object deepClone (Map context) {
        MapObject clone = (MapObject)super.deepClone(context);
        clone.bounds = new Rectangle(bounds);
        clone.popup = new MapObjectPopup(clone);
        return clone;
    }
    
    public MapObject () {
        this(null, 0, 0, 0, 0);
    }


    public MapObject (int width, int height) {
        this(null, 0, 0, width, height);
    }


    public MapObject (String imgName, int width, int height) {
        this(imgName, 0, 0, width, height);
    }


    public MapObject (String imgName, int x, int y, int width, int height) {

        bounds = new Rectangle(x, y, width, height);
        if (imgName != null) setImage(imgName);
        popup = new MapObjectPopup(this);
    }


    public Rectangle getBounds () {
        return bounds;
    }


    public void setBounds (Rectangle bounds) {
        this.bounds = bounds;
    }


    public boolean contains (Point p) {
        return getBounds().contains(p);
    }

    public int getZOrder () {
        return 10;
    }


    protected Image getImage () {
        return img;
    }


    protected void setImage (Image img) {
        this.img = img;
    }


    protected void setImage (String fileName) {
        this.img = 
            (new ImageIcon
                (getClass().getResource
                 ("/images/" + fileName))).getImage();
    }


    protected Shape getPreviewShape () {
        return getBounds();
    }


    protected Stroke getPreviewStroke (float scale) {
        if (previewStroke == null) {
            float dash[] = { 10 / scale };
            previewStroke = 
                new BasicStroke
                    (1, BasicStroke.CAP_BUTT, 
                     BasicStroke.JOIN_MITER, 
                     10 / scale, dash, 0.0f);
        }
        return previewStroke;
    }


    public CanvasEventHandler getCreateHandler (Runnable r) {
        return new CreateHandler(r);
    }


    public EditorPanel getPropertyEditor (MapCanvas canvas) {
        EditorPanel editor = new EditorPanel();
        editor.add(new javax.swing.JLabel("No editable properties"));
        return editor;
    }


    public boolean checkAwtEvent (MapCanvas canvas, AWTEvent evt) {

        if (evt instanceof MouseEvent) {
            MouseEvent me = (MouseEvent)evt;

            if (canvas.containsWrapped(this, me.getPoint())) {
                if (me.isPopupTrigger()) {
                    Point p = me.getPoint();
                    canvas.getTransform().transform(p, p);
                    popup.show(canvas, p.x, p.y);
                    return true;

                } else {
                    if (evt.getID() == MouseEvent.MOUSE_PRESSED) {
                        if (canvas.isErase()) {
                            canvas.saveUndo();
                            canvas.getModel().removeObject(this);
                            canvas.repaint();
                        } else {
                            canvas.setCanvasEventHandler(new MoveHandler(me));
                        }
                    }
                    return true;            
                }
            }
        }
        return false;
    }


    public void moveTo (int x, int y) {
        bounds.x = x;
        bounds.y = y;
    }
    

    public void paint (Graphics2D g, float scale) {

        if (img != null) {
            Rectangle r = getBounds();
            AffineTransform at = AffineTransform.getTranslateInstance
                (r.x, r.y + r.height);
            at.scale(64, -64);
            g.drawImage(img, at, null);
        }
    }


    public abstract void printXml (PrintWriter out) throws IOException;



    //---------------------------------------------------------------------
    // Inner classes

    

    protected class CreateHandler extends CanvasEventAdapter {
        
        private Runnable cmd;
        private Point offset;
        private Shape preview;
        private Shape toBeRemoved;
        private Stroke stroke;


        public CreateHandler (Runnable cmd) {
            this.cmd = cmd;
            Rectangle ob = MapObject.this.getBounds();
            offset = new Point(ob.width / 2, ob.height / 2);
            preview = getPreviewShape();
        }


        public void mouseMoved (MouseEvent evt) {

            Rectangle ob = MapObject.this.getBounds();
            MapCanvas c = (MapCanvas)evt.getSource();
            
            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            if (stroke == null) stroke = getPreviewStroke(c.getScale());
            //g.setStroke(stroke);
            
            if (toBeRemoved != null) c.drawShape(g, toBeRemoved);
            
            AffineTransform tx = AffineTransform.getTranslateInstance
                (evt.getX() - offset.x - ob.x, evt.getY() - offset.y - ob.y);
            c.drawShape(g, toBeRemoved = tx.createTransformedShape(preview));
        }


        public void mousePressed (MouseEvent evt) {

            MapCanvas c = (MapCanvas)evt.getSource();
            c.saveUndo();
            c.getModel().addToFront(MapObject.this);
            MapObject.this.moveTo(evt.getX() - offset.x, 
                                  evt.getY() - offset.y);
            c.setCanvasEventHandler(null);
            if (cmd != null) cmd.run();
            c.repaint();
        }
    }


    
    protected class MoveHandler extends CanvasEventAdapter {


        private Point offset;
        private Shape preview;
        private Shape toBeRemoved;
        private Stroke stroke;


        public MoveHandler (MouseEvent evt) {
            Rectangle ob = MapObject.this.getBounds();
            offset = new Point(evt.getX() - ob.x, evt.getY() - ob.y);
            preview = getPreviewShape();
        }


        public void mouseDragged (MouseEvent evt) {

            Rectangle ob = MapObject.this.getBounds();
            MapCanvas c = (MapCanvas)evt.getSource();

            Graphics2D g = (Graphics2D)c.getGraphics();
            g.setXORMode(Color.black);
            g.setColor(Color.white);
            if (stroke == null) stroke = getPreviewStroke(c.getScale());
            //g.setStroke(stroke);

            if (toBeRemoved != null) c.drawShape(g, toBeRemoved);

            AffineTransform tx = AffineTransform.getTranslateInstance
                (evt.getX() - offset.x - ob.x, evt.getY() - offset.y - ob.y);
            c.drawShape(g, toBeRemoved = tx.createTransformedShape(preview));
        }

        
        public void mouseReleased (MouseEvent evt) {
            MapCanvas c = (MapCanvas)evt.getSource();
            c.saveUndo();
            MapObject.this.moveTo(evt.getX() - offset.x, 
                                  evt.getY() - offset.y);
            c.setCanvasEventHandler(null);
            c.repaint();
        }
    }
}
