/*
 * $Id$
 */
package org.xpilot.jxpmap;

import java.io.IOException;
import java.io.PrintWriter;

import javax.swing.JComboBox;
import javax.swing.JLabel;

/**
 * @author jli
 */
public class Wormhole extends MapObject {
    
    private static final int TYPE_NORMAL = 0;
    private static final int TYPE_IN = 1;
    private static final int TYPE_OUT = 2;

    private static final String[] IMAGES = { 
        "wormhole_normal.gif", "wormhole_in.gif", "wormhole_out.gif" 
    };
    
    private static final String[] TYPES = {
        "normal", "in", "out"        
    };

    private int type;


    public Wormhole () {
        this(0, 0, "normal");
    }


    public Wormhole (int x, int y, String typeStr) {
        super(null, x, y, 35 * 64, 35 * 64);
        for (int i = 0; i < TYPES.length; i++) {
            if (typeStr.equals(TYPES[i])) {
                setType(i);
                break;
            }
        }
    }

    public int getType() {
        return type;
    }

    public void setType(int i) {
        if (i > TYPE_OUT) 
            throw new IllegalArgumentException("illegal wormhole type: " + i);
        type = i;
        setImage(IMAGES[i]);
    }
    
    public void printXml (PrintWriter out) throws IOException {
        out.print("<Wormhole x=\"");
        out.print(getBounds().x + getBounds().width / 2);
        out.print("\" y=\"");
        out.print(getBounds().y + getBounds().height / 2);
        out.print("\" type=\"");
        out.print(TYPES[getType()]);
        out.println("\"/>");
    }

    
    public EditorPanel getPropertyEditor (MapCanvas c) {
        return new WormholePropertyEditor(c);
    }


    private class WormholePropertyEditor extends EditorPanel {

        private JComboBox cmbType;
        private MapCanvas canvas;


        public WormholePropertyEditor (MapCanvas canvas) {

            setTitle("Wormhole");

            cmbType = new JComboBox();
            for (int i = 0; i <= TYPE_OUT; i++) 
                cmbType.addItem(TYPES[i]);
            cmbType.setSelectedIndex(getType());
            add(new JLabel("Type"));
            add(cmbType);
            
            this.canvas = canvas;
        }
        
        
        public boolean apply () {
            int newType = cmbType.getSelectedIndex() ;
            canvas.setWormholeType(Wormhole.this, newType);
            return true;
        }
    }    
}
