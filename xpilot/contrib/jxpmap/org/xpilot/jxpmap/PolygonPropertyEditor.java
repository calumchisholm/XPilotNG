package org.xpilot.jxpmap;

import java.awt.GridLayout;
import java.util.Iterator;

import javax.swing.JComboBox;
import javax.swing.JLabel;

public class PolygonPropertyEditor extends EditorPanel {

    private JComboBox cmbStyle;

    private MapPolygon polygon;
    private MapModel model;
    private MapCanvas canvas;

    
    public PolygonPropertyEditor (MapCanvas canvas, MapPolygon poly) {

        this.polygon = poly;
        this.canvas = canvas;
        this.model = canvas.getModel();

        setTitle("Polygon");

        add(new JLabel("Style:"));
        cmbStyle = new JComboBox();
        for (Iterator iter = model.polyStyles.iterator(); iter.hasNext();) {
            PolygonStyle style = (PolygonStyle)iter.next();
            cmbStyle.addItem(style.getId());
        }
        cmbStyle.setSelectedItem(polygon.getStyle().getId());
        add(cmbStyle);       
    }


    public boolean apply () {
        int styleIndex = cmbStyle.getSelectedIndex();
        PolygonStyle style = (PolygonStyle)model.polyStyles.get(styleIndex);
        canvas.setPolygonProperties(polygon, style);
        return true;
    }
}
