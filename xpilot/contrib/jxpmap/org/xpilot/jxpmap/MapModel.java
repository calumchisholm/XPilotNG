package org.xpilot.jxpmap;

import java.awt.Color;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParserFactory;
import javax.swing.JOptionPane;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;
import org.xml.sax.InputSource;
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


    public List getEdgeStyles() {
        return edgeStyles;
    }


    public List getObjects() {
        return objects;
    }


    public MapOptions getOptions() {
        return options;
    }


    public List getPixmaps() {
        return pixmaps;
    }


    public List getPolyStyles() {
        return polyStyles;
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
    
    public Object[] validateModel () {
        for (Iterator i = objects.iterator(); i.hasNext();) {
            MapObject o = (MapObject)i.next();
            if (o instanceof MapPolygon) {
                Polygon p = ((MapPolygon)o).getPolygon();
                if (p.npoints < 1) continue;
                int x = p.xpoints[0];
                int y = p.ypoints[0];
                if (x < 0 || x >= options.size.width
                || y < 0 || y >= options.size.height) {
                    return new Object[] {
                    o, "Polygon is located outside map bounds." };
                }                
            } else {
                Rectangle b = o.getBounds();
                if (b.x < 0 || b.x >= options.size.width
                || b.y < 0 || b.y >= options.size.height) {
                    return new Object[] {
                    o, "Object is located outside map bounds." };
                }
            }
        }
        return null;
    }


    public void load (String name) 
    throws IOException, SAXException, ParserConfigurationException, 
    XPDFile.ParseException {
            
        edgeStyles.clear();
        LineStyle ls;
        ls = new LineStyle("internal", 0, Color.black, LineStyle.STYLE_HIDDEN);
        edgeStyles.add(ls);
        polyStyles.clear();
        pixmaps.clear();

        XPDFile xpd = XPDFile.load(new File(name));
        XPDFile.Part first = (XPDFile.Part)xpd.get(0);
        readXml(new String(first.data, "ISO8859_1"));

        for (Iterator i = pixmaps.iterator(); i.hasNext();) {
            Pixmap pixmap = (Pixmap)i.next();
            for (Iterator j = xpd.iterator(); j.hasNext();) {
                XPDFile.Part part = (XPDFile.Part)j.next();
                if (pixmap.getFileName().equals(part.name))
                    pixmap.load(new ByteArrayInputStream(part.data));
            }
            if (pixmap.getImage() == null)
                throw new IOException("missing image: " 
                    + pixmap.getFileName());
        }

        defEdgeStyle = (LineStyle)edgeStyles.get(edgeStyles.size() - 1);
        defPolygonStyle = (PolygonStyle)polyStyles.get(polyStyles.size() - 1);
    }
    
    
    private void readXml (String xml) 
    throws IOException, SAXException, ParserConfigurationException {
        SAXParserFactory spf = SAXParserFactory.newInstance();
        spf.setValidating(false);
        XMLReader reader = spf.newSAXParser().getXMLReader();
        reader.setContentHandler(new MapDocumentHandler());
        reader.parse(new InputSource(new StringReader(xml)));
    }


    public void save (File file) throws IOException {
        
        String xml = exportXml();
        
        String xmlName = file.getName();
        int dot = xmlName.lastIndexOf('.');
        if (dot != -1) xmlName = xmlName.substring(0, dot);
        xmlName += ".xp2";
        
        XPDFile xpd = new XPDFile();
        xpd.add(new XPDFile.Part(xmlName, xml.getBytes("ISO8859_1")));
        for (Iterator i = pixmaps.iterator(); i.hasNext();) {
            Pixmap pixmap = (Pixmap)i.next();
            xpd.add(new XPDFile.Part(pixmap.getFileName(), 
                new PPMEncoder().encode(pixmap.getImage())));
        }
        
        xpd.save(file);
    }
    
    
    private String downloadImages() throws IOException {
        String urlStr = (String)options.get("dataurl");
        if (urlStr == null) {
            JOptionPane.showMessageDialog(null,
                "The map has no dataURL option");
            return "";
        }
        File tmpDir = File.createTempFile("jxpmap", null);
        // JDK seems to create an actual file
        // I need to delete it before it can be used as a dir
        tmpDir.delete();
        if (!tmpDir.mkdir())
            throw new IOException("unable to create temporary directory: " 
                + tmpDir.getAbsolutePath());
        tmpDir.deleteOnExit();
        URL url = new URL(urlStr);
        File xpdFile = new File(tmpDir, new File(url.getPath()).getName());        
        InputStream in = url.openStream();
        try {
            FileOutputStream out = new FileOutputStream(xpdFile);
            try {
                byte buf[] = new byte[2048];
                for (int c = in.read(buf); c != -1; c = in.read(buf))
                    out.write(buf, 0, c);
            } finally {
                out.close();
            }
        } finally {
            in.close();
        }
        try {
            XPDFile xpd = XPDFile.load(xpdFile);
            xpdFile.delete();
            for (Iterator i = xpd.iterator(); i.hasNext();) {
                XPDFile.Part p = (XPDFile.Part)i.next();
                File pf = new File(tmpDir, p.name);
                FileOutputStream out = new FileOutputStream(pf);
                try {
                    out.write(p.data);
                } finally {
                    out.close();
                }
                pf.deleteOnExit();
            }
            return tmpDir.getAbsolutePath() + "/";
        } catch (XPDFile.ParseException pe) {
            throw new IOException("corrupted xpd file");
        }
    }
    
    
    public void importXml(String xml)
    throws IOException, SAXException, ParserConfigurationException {
        edgeStyles.clear();
        LineStyle ls;
        ls = new LineStyle("internal", 0, Color.black, LineStyle.STYLE_HIDDEN);
        edgeStyles.add(ls);
        polyStyles.clear();
        pixmaps.clear();

        readXml(xml);

        boolean asked = false;
        String tmpDir = "";
        for (Iterator i = pixmaps.iterator(); i.hasNext();) {
            Pixmap pixmap = (Pixmap)i.next();
            File f = new File(tmpDir + pixmap.getFileName());
            if (!asked && !f.exists()) {
                asked = true;
                if (JOptionPane.showConfirmDialog(null,
                    "Download the images used by this map?") 
                    == JOptionPane.YES_OPTION) {
                    tmpDir = downloadImages();
                    f = new File(tmpDir + pixmap.getFileName());
                }
            }
            if (f.exists()) {
                InputStream in = new BufferedInputStream(
                    new FileInputStream(f));
                try {
                    pixmap.load(in);
                } finally {
                    in.close();
                }
            }
        }

        defEdgeStyle = (LineStyle)edgeStyles.get(edgeStyles.size() - 1);
        defPolygonStyle = (PolygonStyle)polyStyles.get(polyStyles.size() - 1);        
    }
    
    public String exportXml() throws IOException {
        
        StringWriter w = new StringWriter();
        PrintWriter out = new PrintWriter(w);
        
        out.println("<XPilotMap version=\"1.1\">");  
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

        return w.toString();        
    }

    private interface MapTag {
        public MapObject toMapObject() throws SAXException;
    }   

    private class MapDocumentHandler extends DefaultHandler {

        private List tags;
        private PolyTag poly;
        private Map pstyles;
        private Map bstyles;
        private Map estyles;
        private Map opMap;

        public MapDocumentHandler () {
            tags = new ArrayList();
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
                    
                    if (poly == null) poly = new PolyTag();
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
                    tags.add(new ObjectTag(o));

                } else if (name.equalsIgnoreCase("ball")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    String style = atts.getValue("style");
                    tags.add(new BallTag(x, y, team, style));

                } else if (name.equalsIgnoreCase("base")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int dir = Integer.parseInt(atts.getValue("dir"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    Base o = new Base(x, y, dir, team);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    tags.add(new ObjectTag(o));

                } else if (name.equalsIgnoreCase("check")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createCheck();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    tags.add(new ObjectTag(o));
                    
                } else if (name.equalsIgnoreCase("itemconcentrator")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createItemConcentrator();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    tags.add(new ObjectTag(o));

                } else if (name.equalsIgnoreCase("asteroidconcentrator")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapObject o = SimpleMapObject.createAsteroidConcentrator();
                    Rectangle r = o.getBounds();
                    o.moveTo(x - r.width / 2, y - r.height / 2);
                    tags.add(new ObjectTag(o));
                    
                } else if (name.equalsIgnoreCase("grav")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    String type = atts.getValue("type");
                    String force = atts.getValue("force");
                    Grav grav = new Grav(x, y, type, force);
                    Rectangle r = grav.getBounds();
                    grav.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    tags.add(new ObjectTag(grav));

                } else if (name.equalsIgnoreCase("wormhole")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    String type = atts.getValue("type");
                    Wormhole hole = new Wormhole(x, y, type);
                    Rectangle r = hole.getBounds();
                    hole.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    tags.add(new ObjectTag(hole));

                } else if (name.equalsIgnoreCase("ballarea")) {

                    poly = new PolyTag();
                    poly.type = 1;

                } else if (name.equalsIgnoreCase("balltarget")) {

                    poly = new PolyTag();
                    poly.type = 2;
                    poly.team = Integer.parseInt(atts.getValue("team"));

                } else if (name.equalsIgnoreCase("decor")) {
                    
                    poly = new PolyTag();
                    poly.type = 3;

                } else if (name.equalsIgnoreCase("cannon")) {
                    
                    poly = new PolyTag();
                    poly.type = 4;
                    String tmp = atts.getValue("team");
                    poly.team = (tmp != null) ? Integer.parseInt(tmp) : -1;
                    poly.x = Integer.parseInt(atts.getValue("x"));
                    poly.y = Integer.parseInt(atts.getValue("y"));
                    poly.dir = Integer.parseInt(atts.getValue("dir")); 
                    
                } else if (name.equalsIgnoreCase("target")) {
                    
                    poly = new PolyTag();
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
                    tags.add(poly);
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
                    if (ps.color != null) style.setColor(ps.color);      
                    if (ps.textureId != null) 
                        style.setTexture((Pixmap)bstyles.get(ps.textureId));
                    style.parseFlags(ps.flags);
                    
                    LineStyle ls = (LineStyle)estyles.get(ps.defEdgeId);
                    if (ls == null)
                        throw new SAXException
                            ("Undefined edge style: " + ps.defEdgeId);
                    style.setDefaultEdgeStyle(ls);

                    polyStyles.add(style);
                    ps.ref = style;
                }

                for (Iterator i = tags.iterator(); i.hasNext();) {
                    addToFront(((MapTag)i.next()).toMapObject());
                }
                
            } catch (Exception e) {
                e.printStackTrace();
                throw new SAXException(e);
            }                
        }


        // Utility classes used during parsing
        
        private class ObjectTag implements MapTag {
            
            private MapObject o;
            
            ObjectTag(MapObject o) {
                this.o = o;
            }
            
            public MapObject toMapObject() {
                return o;
            }
        }

        private class PolyTag implements MapTag {
            
            String style;
            List points;
            int type, team, dir, x, y;
            boolean hasSpecialEdges;
            
            PolyTag() {
                points = new ArrayList();
            }
            
            public MapObject toMapObject() throws SAXException {
                PolyStyle ps = (PolyStyle)pstyles.get(style);
                if (ps == null) 
                    throw new SAXException
                        ("Undefined polygon style: " + style);
                        
                LineStyle defls = ps.ref.getDefaultEdgeStyle();
                ArrayList edges = hasSpecialEdges ? new ArrayList() : null;
                Polygon awtp = new Polygon();
                
                Iterator i2 = points.iterator();
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
                
                switch(type) {
                    case 1: 
                        return new BallArea(awtp, ps.ref, edges); 
                    case 2: 
                        return new BallTarget(awtp, ps.ref, edges, team); 
                    case 3: 
                        return new Decoration(awtp, ps.ref, edges); 
                    case 4: 
                        return new Cannon(awtp, ps.ref, edges, team, x, y, dir); 
                    case 5: 
                        return new Target(awtp, ps.ref, edges, team); 
                    default:
                        return new MapPolygon(awtp, ps.ref, edges); 
                }                
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
        
        private class BallTag implements MapTag {
            int x;
            int y;
            int team;
            String style;
            
            BallTag(int x, int y, int team, String style) {
                this.x = x;
                this.y = y;
                this.team = team;
                this.style = style;
            }
            
            public MapObject toMapObject() throws SAXException {
                PolygonStyle pstyle = null;
                if (style != null) {
                    PolyStyle ps = (PolyStyle)pstyles.get(style);
                    if (ps == null) 
                        throw new SAXException
                            ("Undefined polygon style: " + style);
                    pstyle = ps.ref;
                }
                MapObject o = new Ball(x, y, team, pstyle);
                Rectangle r = o.getBounds();
                o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                return o;
            }
        }
    }
}
