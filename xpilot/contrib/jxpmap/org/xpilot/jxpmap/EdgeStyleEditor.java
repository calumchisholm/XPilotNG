package org.xpilot.jxpmap;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.Iterator;

public class EdgeStyleEditor extends EditorPanel implements ActionListener {

    private JComboBox cmbStyle;
    private JComboBox cmbWidth;
    private JButton bColor;
    private JCheckBox cbVisible;
    private JCheckBox cbVisibleInRadar;
    private JTextField tfName;

    private LineStyle style;
    private MapModel model;

    
    public EdgeStyleEditor (LineStyle style, MapModel model) {
        
        this.style = style;
        this.model = model;
        
        setTitle("Edge Style Properties");
        setLayout(new GridLayout(4, 2));
        
        add(new JLabel("Name:"));
        tfName = new JTextField();
        if (style.getId() != null) tfName.setText(style.getId());
        add(tfName);
        
        add(new JLabel("Style:"));
        cmbStyle = new JComboBox();
        cmbStyle.addItem("Solid");
        cmbStyle.addItem("Dashed");
        cmbStyle.addItem("Double Dashed");
        cmbStyle.addItem("Hidden");
        cmbStyle.setSelectedIndex(style.getStyle());
        add(cmbStyle);

        add(new JLabel("Width:"));
        cmbWidth = new JComboBox();
        for (int i = 0; i < 6; i++)
            cmbWidth.addItem(String.valueOf(i));
        cmbWidth.setSelectedIndex(style.getWidth());
        add(cmbWidth);
        
        add(new JLabel("Color:"));
        bColor = new JButton();
        bColor.addActionListener(this);
        Color c = style.getColor();
        if (c == null) c = Color.black;
        bColor.setBackground(c);
        add(bColor);
    }


    public boolean apply () {

        if (tfName.getText() == null || tfName.getText().length() == 0) {
            JOptionPane.showMessageDialog
                (this, "First specify a name for this style.",
                 "Information", JOptionPane.INFORMATION_MESSAGE);
            return false;
        }
        style.setId(tfName.getText());
        style.setStyle(cmbStyle.getSelectedIndex());
        style.setWidth(cmbWidth.getSelectedIndex());
        style.setColor(bColor.getBackground());
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
