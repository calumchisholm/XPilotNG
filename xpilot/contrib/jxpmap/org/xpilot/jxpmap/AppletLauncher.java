package org.xpilot.jxpmap;

import java.applet.*;
import java.awt.*;
import java.awt.event.*;

public class AppletLauncher extends Applet implements ActionListener {

    public void init () {
        Button b = new Button("Start");
        b.addActionListener(this);
        add(b);
    }

    public void actionPerformed (ActionEvent ae) {
        try {
            MainFrame.main(new String[0]);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
