package org.xpilot.jxpmap;

import java.awt.Color;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.DefaultHandler;


public class MapModel extends ModelObject {

    List pixmaps;
    List objects;
    List edgeStyles;
    List polyStyles;
    MapOptions options;
    float defaultScale;
    LineStyle defEdgeStyle;
    PolygonStyle defPolygonStyle;

    public Object deepClone (Map context) {

        MapModel clone = (MapModel)super.deepClone(context);
            
        ArrayList l = new ArrayList();
        for (Iterator i = pixmaps.iterator(); i.hasNext();)
            l.add(((Pixmap)i.next()).deepClone(context));
        clone.pixmaps = l;

        l = new ArrayList();
        for (Iterator i = objects.iterator(); i.hasNext();)
            l.add(((MapObject)i.next()).deepClone(context));
        clone.objects = l;
        
        l = new ArrayList();
        for (Iterator i = edgeStyles.iterator(); i.hasNext();)
            l.add(((LineStyle)i.next()).deepClone(context));
        clone.edgeStyles = l;
        
        l = new ArrayList();
        for (Iterator i = polyStyles.iterator(); i.hasNext();)
            l.add(((PolygonStyle)i.next()).deepClone(context));
        clone.polyStyles = l;
        
        clone.options = new MapOptions(options);
        return clone;
    }
    
    public MapModel () {

        defaultScale = 1f / 64;
        objects = new ArrayList();
        polyStyles = new ArrayList();
        pixmaps = new ArrayList();
        edgeStyles = new ArrayList();
        options = new MapOptions();

        LineStyle ls;
        ls = new LineStyle("internal", 0, Color.black, LineStyle.STYLE_HIDDEN);
        edgeStyles.add(ls);
        ls = new LineStyle("default", 0, Color.blue, LineStyle.STYLE_SOLID);
        edgeStyles.add(ls);
        defEdgeStyle = ls;
        
        PolygonStyle ps;
        ps = new PolygonStyle();
        ps.setId("default");
        ps.setVisible(true);
        ps.setVisibleInRadar(true);
        ps.setFillStyle(PolygonStyle.FILL_NONE);
        ps.setDefaultEdgeStyle(defEdgeStyle);
        polyStyles.add(ps);
        defPolygonStyle = ps;
    }


    public float getDefaultScale () {
        return defaultScale;
    }


    public void setDefaultScale (float def) {
        this.defaultScale = def;
    }


    public LineStyle getDefaultEdgeStyle () {
        return defEdgeStyle;
    }


    public void setDefaultEdgeStyle (LineStyle style) {
        this.defEdgeStyle = style;

    }


    public PolygonStyle getDefaultPolygonStyle () {
        return defPolygonStyle;
    }
    
    
    public void setDefaultPolygonStyle (PolygonStyle style) {
        this.defPolygonStyle = style;
    }


    public void addToFront (MapObject moNew) {
        addObject(moNew, true);
    }


    public void addToBack (MapObject moNew) {
        addObject(moNew, false);
    }


    public void addObject (MapObject moNew, boolean front) {

        for (ListIterator iter = objects.listIterator(); iter.hasNext();) {
            MapObject moOld = (MapObject)iter.next();
            int znew = moNew.getZOrder();
            int zold = moOld.getZOrder();
            
            if (znew < zold || (front && znew == zold)) {
                iter.previous();
                iter.add(moNew);
                return;
            }
        }
        objects.add(moNew);
    }

    public void addObject (int index, MapObject mo) {
        objects.add(index, mo);
    }
    
    public void replace (MapObject from, MapObject to) {
        int i = objects.indexOf(from);
        objects.set(i, to);
    }
    
    public int indexOf (MapObject mo) {
        return objects.indexOf(mo);
    }

    public void removeObject (MapObject mo) {
        objects.remove(mo);
    }

    public void removeObject (int index) {
        objects.remove(index);
    }


    public void load (String fileName) 
        throws IOException, SAXException, ParserConfigurationException {

        edgeStyles.clear();
        LineStyle ls;
        ls = new LineStyle("internal", 0, Color.black, LineStyle.STYLE_HIDDEN);
        edgeStyles.add(ls);
        polyStyles.clear();

        System.out.println("parsing map file: "+ fileName);

        String uri = "file:" + new File(fileName).getAbsolutePath();
        SAXParserFactory spf = SAXParserFactory.newInstance();
        spf.setValidating(false);

        XMLReader reader = spf.newSAXParser().getXMLReader();
        reader.setContentHandler(new MapDocumentHandler());
        reader.parse(uri);

        System.out.println(fileName + " parsed successfully");
        System.out.println("loading images");
        
        for (Iterator iter = pixmaps.iterator(); iter.hasNext();) {
            Pixmap px = (Pixmap)iter.next();
            System.out.println(px.getFileName());
            px.load();
        }

        defEdgeStyle = (LineStyle)edgeStyles.get(edgeStyles.size() - 1);
        defPolygonStyle = (PolygonStyle)polyStyles.get(polyStyles.size() - 1);

        System.out.println("ready");
    }


    public void save (File file) throws IOException {
        
        FileWriter fw = new FileWriter(file);
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(fw));
            out.println("<XPilotMap>");

            options.printXml(out);

            for (Iterator iter = pixmaps.iterator(); iter.hasNext();) {
                Pixmap p = (Pixmap)iter.next();
                p.printXml(out);
            }

            for (Iterator iter = edgeStyles.iterator(); iter.hasNext();) {
                LineStyle s = (LineStyle)iter.next();
                s.printXml(out);
            }

            for (Iterator iter = polyStyles.iterator(); iter.hasNext();) {
                PolygonStyle s = (PolygonStyle)iter.next();
                s.printXml(out);
            }

            for (int i = objects.size() - 1; i >= 0; i--) {
                ((MapObject)objects.get(i)).printXml(out);
            }

            out.println("</XPilotMap>");
            
            out.flush();
            out.close();

        } finally {
            try { fw.close(); } catch (IOException ignore) {}
        }
    }


    private class MapDocumentHandler extends DefaultHandler {

        private List polys;
        private Poly poly;
        private Map pstyles;
        private Map bstyles;
        private Map estyles;
        private Map opMap;

        public MapDocumentHandler () {
            polys = new ArrayList();
            estyles = new HashMap();
            estyles.put("internal", edgeStyles.get(0));
            pstyles = new HashMap();
            bstyles = new HashMap();
            opMap = new HashMap();
        }
        

        public void startElement (String ns, 
                                  String local,
                                  String name, 
                                  Attributes atts) 
            throws SAXException {

            try {
                if (name.equalsIgnoreCase("polygon")) {
                    
                    if (poly == null) poly = new Poly();
                    poly.style = atts.getValue("style");
                   
                    poly.points.add(new PolyPoint
                        (Integer.parseInt(atts.getValue("x")), 
                         Integer.parseInt(atts.getValue("y")), 
                         null));

                } else if (name.equalsIgnoreCase("offset")) {
                    
                    String es = atts.getValue("style");
                    if (es != null) poly.hasSpecialEdges = true;
                    poly.points.add(new PolyPoint
                        (Integer.parseInt(atts.getValue("x")), 
                         Integer.parseInt(atts.getValue("y")),
                         es));
                    
                } else if (name.equalsIgnoreCase("edgestyle")) {
                    
                    String id = atts.getValue("id");
                    Color color = 
                        new Color(Integer.parseInt
                                  (atts.getValue("color"), 16));
                    int width = Integer.parseInt(atts.getValue("width"));
                    int style = Integer.parseInt(atts.getValue("style"));
                    if (width == -1) {
                        width = 1;
                        style = LineStyle.STYLE_HIDDEN;
                    }
                    
                    LineStyle ls = new LineStyle(id, width, color, style);
                    estyles.put(id, ls);
                    edgeStyles.add(ls);
                    
                } else if (name.equalsIgnoreCase("polystyle")) {

                    String id = atts.getValue("id");                    
                    Color col = null;
                    String cstr = atts.getValue("color");
                    if (cstr != null) {
                        col = new Color(Integer.parseInt(cstr, 16));
                    }

                    int flags = 1;
                    String flagstr = atts.getValue("flags");
                    if (flagstr != null) flags = Integer.parseInt(flagstr);
                    
                    pstyles.put(id, new PolyStyle
                        (id, col,
                         atts.getValue("texture"),
                         atts.getValue("defedge"),
                         flags));
                    
                } else if (name.equalsIgnoreCase("bmpstyle")) {
                    
                    String id = atts.getValue("id");
                    String fileName = atts.getValue("filename");
                    int flags = Integer.parseInt(atts.getValue("flags"));
                    
                    Pixmap pm = new Pixmap();
                    pm.setFileName(fileName);
                    pm.setScalable(flags != 0);
                    
                    bstyles.put(id, pm);
                    pixmaps.add(pm);

                } else if (name.equalsIgnoreCase("option")) {

                    opMap.put(atts.getValue("name"), atts.getValue("value"));

                } else if (name.equalsIgnoreCase("fuel")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createFuel();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    addToFront(o);

                } else if (name.equalsIgnoreCase("ball")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    Ball o = new Ball(x, y, team);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addToFront(o);

                } else if (name.equalsIgnoreCase("base")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int dir = Integer.parseInt(atts.getValue("dir"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    Base o = new Base(x, y, dir, team);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addToFront(o);

                } else if (name.equalsIgnoreCase("check")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createCheck();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    addToFront(o);
                    
                } else if (name.equalsIgnoreCase("itemconcentrator")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createItemConcentrator();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    addToFront(o);

                } else if (name.equalsIgnoreCase("asteroidconcentrator")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createAsteroidConcentrator();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    addToFront(o);
                    
                } else if (name.equalsIgnoreCase("grav")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    String type = atts.getValue("type");
                    String force = atts.getValue("force");
                    Grav grav = new Grav(x, y, type, force);
                    Rectangle r = grav.getBounds();
                    grav.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addToFront(grav);

                } else if (name.equalsIgnoreCase("wormhole")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    String type = atts.getValue("type");
                    Wormhole hole = new Wormhole(x, y, type);
                    Rectangle r = hole.getBounds();
                    hole.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addToFront(hole);

                } else if (name.equalsIgnoreCase("ballarea")) {

                    poly = new Poly();
                    poly.type = 1;

                } else if (name.equalsIgnoreCase("balltarget")) {

                    poly = new Poly();
                    poly.type = 2;
                    poly.team = Integer.parseInt(atts.getValue("team"));

                } else if (name.equalsIgnoreCase("decor")) {
                    
                    poly = new Poly();
                    poly.type = 3;

                } else if (name.equalsIgnoreCase("cannon")) {
                    
                    poly = new Poly();
                    poly.type = 4;
                    String tmp = atts.getValue("team");
                    poly.team = (tmp != null) ? Integer.parseInt(tmp) : -1;
                    poly.x = Integer.parseInt(atts.getValue("x"));
                    poly.y = Integer.parseInt(atts.getValue("y"));
                    poly.dir = Integer.parseInt(atts.getValue("dir")); 
                    
                } else if (name.equalsIgnoreCase("target")) {
                    
                    poly = new Poly();
                    poly.type = 5;
                    poly.team = Integer.parseInt(atts.getValue("team"));
                    
                } 

            } catch (Exception e) {
                e.printStackTrace();
                throw new SAXException(e);
            }
        }


        public void endElement (String ns, String local, String name) 
            throws SAXException {
            try {
                if (name.equalsIgnoreCase("polygon")) {
                    polys.add(poly);
                    poly = null;
                }
            } catch (Exception e) {
                e.printStackTrace();
                throw new SAXException(e);
            }                
        }

    
        public void endDocument () throws SAXException {
            try {

                options = new MapOptions(opMap);
                
                for (Iterator iter = pstyles.values().iterator(); 
                     iter.hasNext();) {

                    PolyStyle ps = (PolyStyle)iter.next();

                    PolygonStyle style = new PolygonStyle();
                    style.setId(ps.id);
                    style.setVisible(true);
                    style.setVisibleInRadar(true);
                    
                    if (ps.textureId != null) {
                        style.setTexture((Pixmap)bstyles.get(ps.textureId));
                        style.setFillStyle(PolygonStyle.FILL_TEXTURED);

                    } else if (ps.color != null) {
                        style.setColor(ps.color);
                        style.setFillStyle(PolygonStyle.FILL_COLOR);

                    } else {
                        style.setFillStyle(PolygonStyle.FILL_NONE);
                    }
                    
                    LineStyle ls = (LineStyle)estyles.get(ps.defEdgeId);
                    if (ls == null)
                        throw new SAXException
                            ("Undefined edge style: " + ps.defEdgeId);
                    style.setDefaultEdgeStyle(ls);

                    polyStyles.add(style);
                    ps.ref = style;
                }

                for (Iterator iter = polys.iterator(); iter.hasNext();) {

                    Poly p = (Poly)iter.next();
                    
                    PolyStyle ps = (PolyStyle)pstyles.get(p.style);
                    if (ps == null) 
                        throw new SAXException
                            ("Undefined polygon style: " + p.style);
                    PolygonStyle style = ps.ref;

                    LineStyle defls = style.getDefaultEdgeStyle();
                    ArrayList edges = p.hasSpecialEdges ? 
                        new ArrayList() : null;
                    Polygon awtp = new Polygon();
                    
                    Iterator i2 = p.points.iterator();
                    PolyPoint pnt = (PolyPoint)i2.next();
                    int x = pnt.x;
                    int y = pnt.y;
                    awtp.addPoint(x, y);
                    
                    // Current line style. 
                    // Using null to indicate default.
                    LineStyle cls = null;

                    while (i2.hasNext()) {

                        pnt = (PolyPoint)i2.next();

                        // last point is not needed
                        if (i2.hasNext()) {
                            x += pnt.x;
                            y += pnt.y;
                            awtp.addPoint(x, y);
                        }

                        if (edges != null) {
                            if (pnt.style != null) {
                                cls = (LineStyle)estyles.get(pnt.style);
                                if (cls == null)
                                    throw new SAXException
                                        ("Undefined edge style: " + pnt.style);
                                if (cls == defls) cls = null;
                            }
                            edges.add(cls);
                        }
                    }
                    
                    switch(p.type) {
                        case 1: 
                            addToFront(new BallArea(awtp, style, edges)); 
                            break;
                        case 2: 
                            addToFront(new BallTarget(awtp, style, edges, p.team)); 
                            break;
                        case 3: 
                            addToFront(new Decoration(awtp, style, edges)); 
                            break;
                        case 4: 
                            addToFront(new Cannon(awtp, style, edges, p.team, p.x, p.y, p.dir)); 
                            break;
                        case 5: 
                            addToFront(new Target(awtp, style, edges, p.team)); 
                            break;
                        default:
                            addToFront(new MapPolygon(awtp, style, edges)); 
                            break;
                    }
                }

                
            } catch (Exception e) {
                e.printStackTrace();
                throw new SAXException(e);
            }                
        }


        // Utility classes used during parsing

        private class Poly {
            String style;
            List points;
            int type, team, dir, x, y;
            boolean hasSpecialEdges;
            public Poly() {
                points = new ArrayList();
            }
        }


        private class PolyPoint {
            int x;
            int y;
            String style;

            PolyPoint (int x, int y, String style) {
                this.x = x;
                this.y = y;
                this.style = style;
            }
        }


        private class PolyStyle {
            String id;
            Color color;
            String textureId;
            String defEdgeId;
            int flags;
            PolygonStyle ref;

            PolyStyle (String id, Color color, String textureId, 
                       String defEdgeId, int flags) {
                this.id = id;
                this.color = color;
                this.textureId = textureId;
                this.defEdgeId = defEdgeId;
                this.flags = flags;
            }
        }
    }
}
