package org.xpilot.jxpmap;

import java.awt.*;
import javax.swing.*;
import java.util.Iterator;

public class EdgePropertyEditor extends EditorPanel {

    private JComboBox cmbStyle;
    private MapModel model;
    private MapCanvas canvas;
    private MapPolygon polygon;
    private int edgeIndex;

    
    public EdgePropertyEditor (MapCanvas canvas, 
                               MapPolygon polygon, 
                               int edgeIndex) {

        this.canvas = canvas;
        this.model = canvas.getModel();
        this.polygon = polygon;
        this.edgeIndex = edgeIndex;
        
        setTitle("Edge Properties");

        add(new JLabel("Style:"));
        cmbStyle = new JComboBox();
        for (Iterator iter = model.edgeStyles.iterator(); iter.hasNext();) {
            LineStyle style = (LineStyle)iter.next();
            cmbStyle.addItem(style.getId());
        }
        add(cmbStyle);

        if (polygon.edgeStyles != null) {
            LineStyle style = (LineStyle)polygon.edgeStyles.get(edgeIndex);
            if (style != null) {
                cmbStyle.setSelectedItem(style.getId());
                return;
            }
        }

        cmbStyle.setSelectedItem(model.getDefaultEdgeStyle().getId());
    }


    public boolean apply () {

        int styleIndex = cmbStyle.getSelectedIndex();
        LineStyle style = (LineStyle)model.edgeStyles.get(styleIndex);
        if (style == polygon.getStyle().getDefaultEdgeStyle()) style = null;
        polygon.setEdgeStyle(edgeIndex, style);
        canvas.repaint();
        return true;
    }
}
