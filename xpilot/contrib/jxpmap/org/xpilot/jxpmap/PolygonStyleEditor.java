package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Iterator;

public class PolygonStyleEditor extends EditorPanel implements ActionListener {

    private JComboBox cmbFillStyle;
    private JComboBox cmbTexture;
    private JComboBox cmbEdgeStyle;
    private JButton bColor;
    private JCheckBox cbVisible;
    private JCheckBox cbVisibleInRadar;
    private JTextField tfName;

    private PolygonStyle style;
    private MapModel model;

    
    public PolygonStyleEditor (PolygonStyle style, MapModel model) {
        
        this.style = style;
        this.model = model;
        
        setTitle("Polygon Style Properties");
        setLayout(new GridLayout(7, 2));
        
        add(new JLabel("Name:"));
        tfName = new JTextField();
        if (style.getId() != null) tfName.setText(style.getId());
        add(tfName);
        
        add(new JLabel("Fill:"));
        cmbFillStyle = new JComboBox();
        cmbFillStyle.addItem("None");
        cmbFillStyle.addItem("Color");
        cmbFillStyle.addItem("Texture");
        cmbFillStyle.setSelectedIndex(style.getFillStyle());
        add(cmbFillStyle);
        
        add(new JLabel("Color:"));
        bColor = new JButton();
        bColor.addActionListener(this);
        Color c = style.getColor();
        if (c == null) c = Color.black;
        bColor.setBackground(c);
        add(bColor);

        add(new JLabel("Texture:"));
        cmbTexture = new JComboBox();
        cmbTexture.addItem("None");
        int i = 0;
        int j = 0;
        for (Iterator iter = model.pixmaps.iterator(); iter.hasNext(); j++) {
            Pixmap p = (Pixmap)iter.next();
            cmbTexture.addItem(p.getFileName());
            if (style.getTexture() == p) i = j;
        }
        add(cmbTexture);
        cmbTexture.setSelectedIndex(i);
        
        add(new JLabel("Edges:"));
        cmbEdgeStyle = new JComboBox();
        i = 0;
        j = 0;
        for (Iterator iter = model.edgeStyles.iterator(); 
             iter.hasNext(); j++) {
            LineStyle s = (LineStyle)iter.next();
            cmbEdgeStyle.addItem(s.getId());
            if (style.getDefaultEdgeStyle() == s) i = j;
        }
        add(cmbEdgeStyle);
        cmbEdgeStyle.setSelectedIndex(i);
        
        add(new JLabel("Visible:"));
        cbVisible = new JCheckBox();
        cbVisible.setSelected(style.isVisible());
        add(cbVisible);

        add(new JLabel("Radar:"));
        cbVisibleInRadar = new JCheckBox();
        cbVisibleInRadar.setSelected(style.isVisibleInRadar());
        add(cbVisibleInRadar);
    }


    public boolean apply () {

        if (tfName.getText() == null || tfName.getText().length() == 0) {
            JOptionPane.showMessageDialog
                (this, "First specify a name for this style.",
                 "Information", JOptionPane.INFORMATION_MESSAGE);
            return false;
        }
        style.setId(tfName.getText());
        style.setFillStyle(cmbFillStyle.getSelectedIndex());
        style.setColor(bColor.getBackground());
        int i = cmbTexture.getSelectedIndex();
        if (i == 0) {
            if (style.getFillStyle() == style.FILL_TEXTURED) {
                JOptionPane.showMessageDialog
                    (this, "Select a texture if you want textured filling.",
                     "Information", JOptionPane.INFORMATION_MESSAGE);
                return false;
            }
            style.setTexture(null);
        }
        else style.setTexture((Pixmap)model.pixmaps.get(i - 1));
        i = cmbEdgeStyle.getSelectedIndex();
        if (i > 0) 
            style.setDefaultEdgeStyle((LineStyle)model.edgeStyles.get(i - 1));
        style.setVisible(cbVisible.isSelected());
        style.setVisibleInRadar(cbVisibleInRadar.isSelected());
        
        return true;
    }


    public void actionPerformed (ActionEvent evt) {
        if (evt.getSource() == bColor) {
            Color c = JColorChooser.showDialog
                (this, "Pick a color", Color.black);
            if (c != null) bColor.setBackground(c);
        }
    }
}
