package org.xpilot.jxpmap;

import java.awt.*;
import javax.swing.*;
import java.util.Iterator;

public class PolygonPropertyEditor extends EditorPanel {

    private JComboBox cmbStyle;
    private JComboBox cmbType;
    private JComboBox cmbTeam;

    private MapPolygon polygon;
    private MapModel model;

    
    public PolygonPropertyEditor (MapCanvas canvas, MapPolygon poly) {

        this.polygon = poly;
        this.model = canvas.getModel();

        setTitle("Polygon Properties");
        setLayout(new GridLayout(3, 2));

        add(new JLabel("Style:"));
        cmbStyle = new JComboBox();
        for (Iterator iter = model.polyStyles.iterator(); iter.hasNext();) {
            PolygonStyle style = (PolygonStyle)iter.next();
            cmbStyle.addItem(style.getId());
        }
        cmbStyle.setSelectedItem(polygon.getStyle().getId());
        add(cmbStyle);
        
        add(new JLabel("Type:"));
        cmbType = new JComboBox();
        cmbType.addItem("Normal");
        cmbType.addItem("Ball area");
        cmbType.addItem("Ball target");
        cmbStyle.setSelectedIndex(polygon.getType());
        add(cmbType);
        
        add(new JLabel("Team:"));
        cmbTeam = new JComboBox();
        for (int i = 0; i < 10; i++) 
            cmbTeam.addItem(new Integer(i + 1));
        if (polygon.getTeam() > 0 && polygon.getTeam() < 11) {
            cmbTeam.setSelectedIndex(polygon.getTeam() - 1);
        } else {
            cmbTeam.setSelectedIndex(0);
        }
        add(cmbTeam);
    }


    public boolean apply () {

        int styleIndex = cmbStyle.getSelectedIndex();
        PolygonStyle style = (PolygonStyle)model.polyStyles.get(styleIndex);
        polygon.setStyle(style);
        model.setDefaultPolygonStyle(styleIndex);
        
        polygon.setType(cmbType.getSelectedIndex());
        
        if (polygon.getType() != MapPolygon.TYPE_NORMAL) {
            polygon.setTeam(cmbTeam.getSelectedIndex() + 1);
        }
        return true;
    }
}
