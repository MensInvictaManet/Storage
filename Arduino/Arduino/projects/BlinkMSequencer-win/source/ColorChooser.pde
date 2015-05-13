// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */

public class ColorChooser extends JPanel implements ChangeListener {
  
    private JColorChooser chooser = new JColorChooser();
  
    /**
     *
     */
    public ColorChooser() {
        this.setBackground(bgLightGray);
        chooser.getSelectionModel().addChangeListener(this);
        chooser.setPreviewPanel(colorPreview); // warning, global variable
        chooser.setBackground(bgLightGray);

        this.add(chooser);
    
        // add separator
        //ImageIcon sepImg = new Util().createImageIcon("blinkm_separator_vert_large.gif", "vert separator");
        //JLabel sep1 = new JLabel(sepImg);
        //this.add(sep1);

    }
  
    /**
     * called by Timeline and anyone else who wants to set the color
     */
    public void setColor(Color c) {
        chooser.setColor(c);
    }

    /**
     * Set color preview and slice color after user has changed the color
     */
    public void stateChanged(ChangeEvent e) {
        Color c = chooser.getColor();

        // update ColorPreview panel
        colorPreview.setColor(c);

        // update selected TimeSlice in TimeLine
        for( int i=0; i<numSlices; i++) {
            TimeSlice ts = tl.timeSlices[i];
            if (ts.isActive)
                ts.setColor(c);
            //ts.isActive = false;
        }
        tl.repaint();
    }
}
