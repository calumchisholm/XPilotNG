package org.xpilot.jxpmap;

import java.awt.Dimension;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import javax.swing.JOptionPane;

public class MapOptions extends HashMap {


    public Dimension size;
    public boolean edgeWrap;

    
    public MapOptions () {
        super();
        try {
            Properties props = new Properties();
            props.load(getClass().getResourceAsStream("defopts"));
            putAll(props);

        } catch (Exception e) {
            JOptionPane.showMessageDialog
                (null, "Failed to load default options: " + 
                 e.getMessage(), "Error",
                 JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
        }
        updated();
    }


    public MapOptions (Map options) {

        this();
        putAll(options);
        updated();
    }


    public void printXml (PrintWriter out) throws IOException {

        out.println("<GeneralOptions>");

        for (Iterator iter = entrySet().iterator(); iter.hasNext();) {
            
            Map.Entry entry = (Map.Entry)iter.next();
            out.print("<Option name=\"");
            out.print(entry.getKey());
            out.print("\" value=\"");
            out.print(xmlEncode((String)entry.getValue()));
            out.println("\"/>");
        }

        out.println("</GeneralOptions>");
    }


    public void putAll (Map map) {
        for (Iterator iter = map.entrySet().iterator(); iter.hasNext();) {
            Map.Entry entry = (Map.Entry)iter.next();
            put(entry.getKey().toString().toLowerCase(), entry.getValue());
        }
    }


    public void updated () {

        size = new Dimension();
        size.width = Integer.parseInt((String)get("mapwidth"));
        size.height = Integer.parseInt((String)get("mapheight"));
        
        size.width *= 64;
        size.height *= 64;
        
        Object oew = get("edgewrap");
        if (oew != null) {
            String ew = ((String)oew).toLowerCase();
            edgeWrap = "yes".equals(ew) || "true".equals(ew);
        }
        
    }


    private String xmlEncode (String str) {

        StringBuffer sb = new StringBuffer();

        for (int i = 0; i < str.length(); i++) {

            char ch = str.charAt(i);
            switch (ch) {
            case '&':  sb.append("&amp;");  break;
            case '"':  sb.append("&quot;"); break;
            case '\'': sb.append("&apos;"); break;
            case '<':  sb.append("&lt;");   break;
            case '>':  sb.append("&gt;");   break;
            default:   sb.append(ch);
            }
        }
        return sb.toString();
    }
}
