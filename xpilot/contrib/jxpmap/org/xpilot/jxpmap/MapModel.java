package org.xpilot.jxpmap;

import java.awt.Point;
import java.awt.Polygon;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Rectangle;
import java.io.*;
import java.util.*;
import javax.xml.parsers.*;
import org.xml.sax.*;


public class MapModel {

    List pixmaps;
    List objects;
    List edgeStyles;
    List polyStyles;
    MapOptions options;
    PolygonStyle defPolyStyle;
    LineStyle defLineStyle;
    float defaultScale;

    
    public MapModel () {
        defaultScale = 1f / 64;
        objects = new ArrayList();
        polyStyles = new ArrayList();
        pixmaps = new ArrayList();
        edgeStyles = new ArrayList();
        options = new MapOptions();
        
        defLineStyle = new LineStyle
            ("0", 1, Color.blue, LineStyle.STYLE_SOLID);
                
        defPolyStyle = new PolygonStyle();
        defPolyStyle.setId("0");
        defPolyStyle.setVisible(true);
        defPolyStyle.setVisibleInRadar(true);
        defPolyStyle.setFillStyle(defPolyStyle.FILL_NONE);
        defPolyStyle.setDefaultEdgeStyle(defLineStyle);

        edgeStyles.add(getDefaultEdgeStyle());
        polyStyles.add(getDefaultPolygonStyle());
    }


    public PolygonStyle getDefaultPolygonStyle () {
        return defPolyStyle;
    }
    
    
    public LineStyle getDefaultEdgeStyle () {
        return defLineStyle;
    }


    public float getDefaultScale () {
        return defaultScale;
    }


    public void setDefaultScale (float def) {
        this.defaultScale = def;
    }


    public void addObject (MapObject moNew) {
        for (ListIterator iter = objects.listIterator(); iter.hasNext();) {
            MapObject moOld = (MapObject)iter.next();
            if (moNew.getZOrder() <= moOld.getZOrder()) {
                iter.previous();
                iter.add(moNew);
                return;
            }
        }
        objects.add(moNew);
    }

    
    public void removeObject (MapObject mo) {
        objects.remove(mo);
    }


    public void load (String fileName) 
        throws IOException, SAXException, ParserConfigurationException {

        edgeStyles.clear();
        polyStyles.clear();

        System.out.println("parsing map file: "+ fileName);

        String uri = "file:" + new File(fileName).getAbsolutePath();
        SAXParserFactory spf = SAXParserFactory.newInstance();
        spf.setValidating(false);

        Parser parser = spf.newSAXParser().getParser();
        parser.setDocumentHandler(new MapDocumentHandler());
        parser.parse(uri);

        System.out.println(fileName + " parsed successfully");
        System.out.println("loading images");
        
        for (Iterator iter = pixmaps.iterator(); iter.hasNext();) {
            Pixmap px = (Pixmap)iter.next();
            System.out.println(px.getFileName());
            px.load();
        }

        if (edgeStyles.size() == 0) edgeStyles.add(getDefaultEdgeStyle());
        if (polyStyles.size() == 0) polyStyles.add(getDefaultPolygonStyle());

        System.out.println("ready");
    }


    public void save (File file) throws IOException {

        int baseCount = 0;
        int ballCount = 0;
        int fuelCount = 0;
        int checkCount = 0;

        for (Iterator iter = objects.iterator(); iter.hasNext();) {
            Object o = iter.next();
            if (o instanceof MapBase) baseCount++;
            else if (o instanceof MapBall) ballCount++;
            else if (o instanceof MapFuel) fuelCount++;
            else if (o instanceof MapCheckPoint) checkCount++;
        }
        
        FileWriter fw = new FileWriter(file);
        try {
            PrintWriter out = new PrintWriter(new BufferedWriter(fw));
            out.println("<XPilotMap>");

            out.print("<Featurecount bases=\"");
            out.print(baseCount);
            out.print("\" balls=\"");
            out.print(ballCount);
            out.print("\" fuels=\"");
            out.print(fuelCount);
            out.print("\" checks=\"");
            out.print(checkCount);
            out.println("\"/>");

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
            
            for (Iterator iter = objects.iterator(); iter.hasNext();) {
                ((MapObject)iter.next()).printXml(out);
            }

            out.println("</XPilotMap>");
            
            out.flush();
            out.close();

        } finally {
            try { fw.close(); } catch (IOException ignore) {}
        }
    }


    private class MapDocumentHandler extends HandlerBase {

        private List polys;
        private Poly poly;
        private Map pstyles;
        private Map bstyles;
        private Map estyles;
        private Map opMap;
        private int ballAreaTeam;
        private int ballTargetTeam;


        public MapDocumentHandler () {
            polys = new ArrayList();
            estyles = new HashMap();
            pstyles = new HashMap();
            bstyles = new HashMap();
            opMap = new HashMap();
            ballAreaTeam = -1;
            ballTargetTeam = -1;
        }
        

        public void startElement (String name, AttributeList atts) 
            throws SAXException {

            try {
                if (name.equalsIgnoreCase("polygon")) {
                    
                    poly = new Poly();
                    poly.style = atts.getValue("style");
                    poly.points = new ArrayList();
                    
                    poly.points.add(new PolyPoint
                        (Integer.parseInt(atts.getValue("x")), 
                         Integer.parseInt(atts.getValue("y")), 
                         null));

                    if (ballAreaTeam != -1) {
                        poly.type = MapPolygon.TYPE_BALLAREA;
                        poly.team = ballAreaTeam;
                    } else if (ballTargetTeam != -1) {
                        poly.type = MapPolygon.TYPE_BALLTARGET;
                        poly.team = ballTargetTeam;
                    }
                    
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
                        new Color(Integer.parseInt(atts.getValue("color")));
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
                        col = new Color(Integer.parseInt(cstr));
                    }
                    
                    pstyles.put(id, new PolyStyle
                        (id, col,
                         atts.getValue("texture"),
                         atts.getValue("defedge"),
                         Integer.parseInt(atts.getValue("flags"))));
                    
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
                    MapFuel o = new MapFuel(x, y);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addObject(o);

                } else if (name.equalsIgnoreCase("ball")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    MapBall o = new MapBall(x, y, team);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addObject(o);

                } else if (name.equalsIgnoreCase("base")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    int dir = Integer.parseInt(atts.getValue("dir"));
                    int team = Integer.parseInt(atts.getValue("team"));
                    MapBase o = new MapBase(x, y, dir, team);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addObject(o);

                } else if (name.equalsIgnoreCase("check")) {

                    int x = Integer.parseInt(atts.getValue("x"));
                    int y = Integer.parseInt(atts.getValue("y"));
                    MapCheckPoint o = new MapCheckPoint(x, y);
                    Rectangle r = o.getBounds();
                    o.moveTo(r.x - r.width / 2, r.y - r.height / 2);
                    addObject(o);

                } else if (name.equalsIgnoreCase("ballarea")) {

                    ballAreaTeam = Integer.parseInt(atts.getValue("team"));

                } else if (name.equalsIgnoreCase("balltarget")) {

                    ballTargetTeam = Integer.parseInt(atts.getValue("team"));

                } else if (name.equalsIgnoreCase("scale")) {
                }

            } catch (Exception e) {
                e.printStackTrace();
                throw new SAXException(e);
            }
        }


        public void endElement (String name) throws SAXException {
            
            try {
                if (name.equalsIgnoreCase("polygon")) {
                    polys.add(poly);

                } else if (name.equalsIgnoreCase("ballarea")) {
                    ballAreaTeam = -1;

                } else if (name.equalsIgnoreCase("balltarget")) {
                    ballTargetTeam = -1;

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
                        style.setFillStyle(style.FILL_TEXTURED);

                    } else if (ps.color != null) {
                        style.setColor(ps.color);
                        style.setFillStyle(style.FILL_COLOR);

                    } else {
                        style.setFillStyle(style.FILL_NONE);
                    }
                    
                    if (ps.defEdgeId != null) {
                        style.setDefaultEdgeStyle
                            ((LineStyle)estyles.get(ps.defEdgeId));
                    }
                    polyStyles.add(style);

                    ps.ref = style;
                }

                
                for (Iterator iter = polys.iterator(); iter.hasNext();) {

                    Poly p = (Poly)iter.next();

                    PolygonStyle style = null;
                    if (p.style != null) {
                        PolyStyle ps = (PolyStyle)pstyles.get(p.style);
                        if (ps != null) style = ps.ref;
                    }
                    if (style == null) style = defPolyStyle;
                    
                    ArrayList edges = p.hasSpecialEdges ? 
                        new ArrayList() : null;

                    Polygon awtp = new Polygon();
                    
                    Iterator i2 = p.points.iterator();
                    PolyPoint pnt = (PolyPoint)i2.next();
                    int x = pnt.x;
                    int y = pnt.y;
                    awtp.addPoint(x, y);
                    
                    while (i2.hasNext()) {
                        pnt = (PolyPoint)i2.next();
                        x += pnt.x;
                        y += pnt.y;
                        awtp.addPoint(x, y);
                        if (edges != null) 
                            edges.add(pnt.style != null ? 
                                      estyles.get(pnt.style) : null);
                    }

                    MapPolygon mp = new MapPolygon(awtp, style, edges);
                    mp.setType(p.type);
                    mp.setTeam(p.team);
                    
                    addObject(mp);
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
            int type;
            int team;
            boolean hasSpecialEdges;
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
