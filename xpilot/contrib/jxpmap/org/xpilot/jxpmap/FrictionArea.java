/*
 * $Id$
 */
package org.xpilot.jxpmap;

import java.awt.Polygon;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.math.BigDecimal;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JLabel;

/**
 * @author jli
 */
public class FrictionArea extends MapPolygon {
    
    private BigDecimal friction;
    
    public FrictionArea() {
        super();
        this.friction = new BigDecimal(0.0);
    }
    
    public FrictionArea(Polygon p, PolygonStyle style, ArrayList edgeStyles,
    BigDecimal firction) {
        super(p, style, edgeStyles);
        setFriction(friction);
    }
    
    public BigDecimal getFriction() {
        return friction;
    }
    
    public void setFriction(BigDecimal value) {
        this.friction = value;
    }

    public void printXml(PrintWriter out) throws IOException {
        out.println("<FrictionArea friction=\"" + getFriction() + "\">");
        super.printXml(out);
        out.println("</FrictionArea>");
    }

    public EditorPanel getPropertyEditor(MapCanvas canvas) {
        CompoundEditor ce = 
            new CompoundEditor("FrictionArea", canvas, this);
        ce.add(new FrictionEditor(canvas));
        ce.add(super.getPropertyEditor(canvas));
        return ce;
    }
    
    private class FrictionEditor extends EditorPanel {
    
        private MapCanvas canvas;
        private JTextField field;
    
        public FrictionEditor(MapCanvas canvas) {
            setTitle("Friction");
            field = new JTextField(4);
            field.setText(getFriction().toString());
            add(new JLabel("Friction:"));
            add(field);
            this.canvas = canvas;
        }
    
        public boolean apply() {
            try {
                BigDecimal newFriction = new BigDecimal(field.getText());
                double d = newFriction.doubleValue();
                if (d < -1.0 || d > 1.0) {
                    JOptionPane.showMessageDialog
                        (this, "The friction has to be between -1.0 and 1.0.",
                        "Information", JOptionPane.INFORMATION_MESSAGE);
                    return false;                              
                }
                setFriction(newFriction);
                return true;
            } catch (NumberFormatException nfe) {
                JOptionPane.showMessageDialog
                    (this, "Invalid friction value: " + field.getText(),
                     "Error", JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }
    }    
}
