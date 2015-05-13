import processing.core.*; import com.brunchboy.util.swing.relativelayout.*; import java.awt.*; import java.awt.event.*; import javax.swing.*; import javax.swing.event.*; import javax.swing.colorchooser.*; import javax.swing.plaf.metal.*; import processing.serial.*; import com.brunchboy.util.swing.relativelayout.*; import java.applet.*; import java.awt.*; import java.awt.image.*; import java.awt.event.*; import java.io.*; import java.net.*; import java.text.*; import java.util.*; import java.util.zip.*; public class BlinkMSequencer extends PApplet {// Copyright (c) 2007-2008, ThingM Corporation

// 11-11 - adding color chooser component - 2 hours
// 11-11 - layout all components using relative layout manager - 4 hours
// 11-29 - added BurnDialog and ConnectDialog - 3 hours
// 11-30 - fixed bug with duplicate zip entry - .5 hours
// 11-30 - added code to hide processing PApplet and parent frame - .5 hours
// 12-05 - update ui to design v0.16 and integrate new images - 4 hours
// 12-07 - development of timeline custom component - 3.5 hours
// 12-15 - integrated new ui images - 1 hours
// 12-15 - added TimeSlice class for TimeLine class - 1 hours
// 12-15 - updated timeline- added selection capabilities, color preview, snapping, and multi-selection - .5
/////// above sent as invoice for first 20 hours

// 12-15 - updated timeline- added selection capabilities, color preview, snapping, and multi-selection - 4.5 hours
// 12-19 - bugfix- bgcolors, sampling rate of playhead, drag selection, play button synch. - 3 hours
//
// Tod's notes:
// ------------
// 12-21 - Added to ThingM SVN repository as "BlinkMSequencer"
//         Directory 'images' not needed, 
//         No source code to com.brunchboy.util.swing.relativelayout.*
//          (but available on the Net)
//         Not seeing need for ColorM class.
// 12-22 - Added "BlinkMComm" class to encapsulate all serial & BlinkM tasks
//         Removed ColorM and used Java's Color class instead
//         Works programming a BlinkM (tho no fade & duration adjustments yet)
//         Changed About & Help URLs to thingm.com/products/blinkm{/help}
//         Changed use flow on Connect & Burn dialogs to be more streamlined
//           (via some lame hacks i did)
// 12-23 - Still many visual differences between Mac & Windows. Why Java why?
//         Trying to figure out why there's a global ColorPreview and 
//           a private one for ColorChooser
//         Removed private ColorPreview in ColorChooser; all ref global one now
//         TimeSlices are now in an array instead of ArrayList; it doesn't grow
//           or shrink so no need for dynamic container
//         Parameterized number of Slices ('numSlices') and set to 48
//         Changed timeline rendering to be inset so it's centered (ish)
//         Added double-click to mean show color in ColorPreview, hopefully
//           will be able to add ColorChooser tracking too.
//         Changed BlinkMComm from entirely static class to one you must 
//           instantiate.  Done mostly because it allows use of Log l.
//         ColorChooser now global and control ColorPreview
//         Double-click on timeline -> color chooser working
//         Fixed tick marks (was tenMarks) to be %8 instead of %10
//         Using Java cross-platform Look-n-Feel to solve discrepancies
//         Fixed non-loop Play button bug by making PlayButton global
// 12-24 - Added real-time BlinkM color updating. it's cool
//           but dunno if we'll keep it.
//         Parameterized durations, since BlinkM might not do 1,10,100
//         Fixed loop speed timing inaccuracies
//         Fixed fast rubber-band multi-select problem
//         Added fading simulator to preview
//         Changed ConnectDialog (and soon BurnDialog) to Swing so its
//           visual style matches the rest (and is more manageable)
//         Add blinkm_thingm_corp.gif footer image in place of text
//           to prevent Windows/Mac layout variations
// 12-25 - Migrated BurnDialog Swing.
//         Added progress bar to BurnDialog, also msg & 'OK' button to
//           hilight to people stand-alone operation vs. real-time preview.
//         Fixed timing for fastest case: 2secs on timeline == 2 secs on blinkm
//         Fixed Windows serial bug by using Serial.print(byte[]) in BlinkMComm
//         Disconnecting now works
//         ConnectDialog now has different modes for connect vs disconnect
//         Parameterized timers.  Timers seem to bog down Windows, need tuning
// 12-26 - Added version number in footer, removed thingm corp image
//         Added connect/disconnect button toggle
// 01-03 - Added 1500 msec delay after connect to wait for Diecimila reset
//         Fixed programming of reps and boot setup in BlinkMComm
//         Since BurnDialog can't be modal, let it enable/disable button to
//           prevent multiple presses
//         Fixed bug where programmed play doesn't stop when doing preview play
//         Tuned programmed timing to be match better preview timing
//         
//
// 
// To-do:
// - tune fade time for both real blinkm & preview
// - tune & test other two loop durations 
// - research why timers on windows are slower (maybe use runnable)
// - need to deal with case of *no* serial ports available
//

/**
 * RelativeLayout is an open-source alternative layout manager
 * Details here:
 * @see: http://www.onjava.com/pub/a/onjava/2002/09/18/relativelayout.html?page=2
 */










String VERSION = "001";

Log l = new Log();

BlinkMComm blinkmComm = new BlinkMComm();

MainFrame mf;
ColorChooser colorChooser;
ColorPreview colorPreview;
TimeLine tl;
//RightPanel rp;
PlayButton pb;

JPanel connectPanel;

// number of slices in the timeline == number of script lines written to BlinkM
int numSlices = 48;

// overall dimensions
int mainWidth = 851;
int mainHeight = 455;
int mainHeightAdjForWindows = 12; // fudge factor for Windows layout variation


// the possible durations for the loop
int[] durations = { 3, 30, 120 };
int durationCurrent = durations[0];

PApplet p;
Util util = new Util();

Color fgLightGray = new Color(230, 230, 230);
Color bgLightGray = new Color(200, 200, 200);
Color bgMidGray   = new Color(140, 140, 140);
Color bgDarkGray  = new Color(100, 100, 100);
Color tlDarkGray  = new Color(55, 55, 55);          // dark color for timeline
Color highLightC  = new Color(255, 0, 0);           // used for selections


/**
 * Processing's setup()
 */
public void setup() {
  size(100, 100);   // Processing's frame, we'll turn this off in a bit

  try {  // use a Swing look-and-feel that's the same across all OSs
    MetalLookAndFeel.setCurrentTheme(new DefaultMetalTheme());
    UIManager.setLookAndFeel( new MetalLookAndFeel() );
  } catch(Exception e) { 
    l.error("drat: "+e);
  }

  String osname = System.getProperty("os.name");
  if( osname.toLowerCase().startsWith("windows") ) 
    mainHeight += mainHeightAdjForWindows;
  
  mf = new MainFrame(mainWidth, mainHeight, this);
  p = this;

  // layout MainFrameicon
  RelativeLayout layout = new RelativeLayout();
  mf.getContentPane().setLayout(layout);
  //mf.f.requestFocusInWindow();

  // add top control panel
  TopPanel tp = new TopPanel();
  mf.getContentPane().add(tp, "top_panel");

  layout.addConstraint("top_panel", AttributeType.TOP,
  new AttributeConstraint(DependencyManager.ROOT_NAME, AttributeType.TOP, 0));

  layout.addConstraint("top_panel", AttributeType.LEFT,
  new AttributeConstraint(DependencyManager.ROOT_NAME, AttributeType.LEFT, 0));

  //layout.addConstraint("top_panel", AttributeType.HORIZONTAL_CENTER,
  //new AxisConstraint(DependencyManager.ROOT_NAME, AttributeAxis.HORIZONTAL, 0.5));

  // add TimeLine
  tl = new TimeLine();
  mf.getContentPane().add(tl, "time_line");

  layout.addConstraint("time_line", AttributeType.TOP,
  new AttributeConstraint("top_panel", AttributeType.BOTTOM, 0));

  layout.addConstraint("time_line", AttributeType.HORIZONTAL_CENTER,
  new AttributeConstraint(DependencyManager.ROOT_NAME, AttributeType.HORIZONTAL_CENTER, 0));

  // add Color Preview panel  (must exist before colorChooser)
  colorPreview = new ColorPreview();
  mf.getContentPane().add(colorPreview, "color_preview");

  layout.addConstraint("color_preview", AttributeType.TOP,
  new AttributeConstraint("time_line", AttributeType.BOTTOM, 3));

  layout.addConstraint("color_preview", AttributeType.LEFT,
  new AttributeConstraint("color_chooser", AttributeType.RIGHT, 0));

  // add ColorChooser
  colorChooser = new ColorChooser();
  mf.getContentPane().add(colorChooser, "color_chooser");

  layout.addConstraint("color_chooser", AttributeType.TOP,
  new AttributeConstraint("time_line", AttributeType.BOTTOM, 3));

  layout.addConstraint("color_chooser", AttributeType.LEFT,
  new AttributeConstraint(DependencyManager.ROOT_NAME, AttributeType.LEFT, 0));


  // add RightPanel
  RightPanel rp = new RightPanel();
  mf.getContentPane().add(rp, "right_panel");

  layout.addConstraint("right_panel", AttributeType.RIGHT,
  new AttributeConstraint(DependencyManager.ROOT_NAME, AttributeType.RIGHT,0));

  layout.addConstraint("right_panel", AttributeType.TOP,
  new AttributeConstraint("time_line", AttributeType.BOTTOM, 0));

  layout.addConstraint("right_panel", AttributeType.BOTTOM,
  new AttributeConstraint("lower_panel", AttributeType.TOP, 0));

  layout.addConstraint("right_panel", AttributeType.LEFT,
  new AttributeConstraint("color_preview", AttributeType.RIGHT, 0));


  // add Lower Panel
  JPanel lp = new JPanel();
  lp.setBackground(bgMidGray);

  //ImageIcon llText = new Util().createImageIcon("blinkm_thingm_corp.gif", "thingm corp");
  //JLabel lowLabel = new JLabel(llText);
  JLabel lowLabel = new JLabel("  version "+VERSION+" \u00a9 ThingM Corporation", JLabel.LEFT);
  lowLabel.setHorizontalAlignment(JLabel.LEFT);
  lp.setPreferredSize(new Dimension(855, 30));
  lp.setLayout(new BorderLayout());
  lp.add(lowLabel, BorderLayout.WEST);
  //ImageIcon lllText = new Util().createImageIcon("blinkm_thingm_logo.gif", "thingm logo");
  //JLabel lowLogo = new JLabel(lllText);
  //lp.add(lowLogo, BorderLayout.EAST);
  mf.getContentPane().add(lp, "lower_panel");

  layout.addConstraint("lower_panel", AttributeType.TOP,
  new AttributeConstraint("color_chooser", AttributeType.BOTTOM, 7));

  layout.addConstraint("lower_panel", AttributeType.LEFT,
  new AttributeConstraint("color_chooser", AttributeType.LEFT, 0));

  mf.setVisible(true);
  mf.setResizable(false);

}


/**
 * Processing's draw()
 */
public void draw() {
  super.frame.setVisible(false);  // turn off Processing's frame
  super.frame.toBack();
  mf.toFront();

  if(frameCount > 30) {  // what is this here for?
    noLoop(); 
  }
}


// Copyright (c) 2007-2008, ThingM Corporation

/**
 * BlinkMComm -- Simple Processing 'library' to talk to BlinkM 
 *               (via an Arduino programmed with BlinkMCommunicator)
 *
 *
 * Tod E. Kurt, ThingM, http://thingm.com/
 * 
 */



//import javax.swing.progressbar;

public class BlinkMComm {
  public final boolean fakeIt = false;

  boolean isConnected = false;

  public byte blinkm_addr = 0x09;
  public String portName = null;
  public final int portSpeed = 19200;

  Serial port;

  // mapping of duration to ticks      (must be same length as 'durations')
  public byte[] durTicks   = { (byte)   1, (byte) 18, (byte) 72 };
  // mapping of duration to fadespeeds (must be same length as 'durations')
  public byte[] fadeSpeeds = { (byte) 100, (byte) 25, (byte)  5 };
    
  Color cBlk = new Color(0,0,0);
  Color lastColor;

  // Return a list of potential ports
  // they should be ordered by best to worst (but are not right now)
  // this can't be static as a .pde, sigh.
  public String[] listPorts() {
    String[] a = Serial.list();
    String osname = System.getProperty("os.name");
    if( osname.toLowerCase().startsWith("windows") ) {
      // reverse list because Arduino is almost always highest COM port
      for(int i=0;i<a.length/2;i++){
        String t = a[i]; a[i] = a[a.length-(1+i)]; a[a.length-(1+i)] = t;
      }
      //for(int left=0, int right=list.length-1; left<right; left++, right--) {
      //  // exchange the first and last
      //  String tmp = list[left]; list[left] = list[right]; list[right] = tmp;
      //}
    }
    return a;
  }

  public BlinkMComm() {

  }

  /**
   * Connect to the given port
   * Can optionally take a PApplet (the sketch) to get serialEvents()
   * but this is not recommended
   *
   */
  public void connect( PApplet p, String portname ) throws Exception {
    l.debug("BlinkMComm.connect: portname:"+portname);
    try {
      if(port != null)
        port.stop(); 
      port = new Serial(p, portname, portSpeed);
      isConnected = true;
      portName = portname;
    }
    catch (Exception e) {
      isConnected = false;
      portName = null;
      port = null;
      throw e;
    }
  }

  // disconnect but remember the name
  public void disconnect() {
    if( port!=null )
      port.stop();
    isConnected = false;
  }

  // verifies connection is good
  public boolean checkConnection() {
    // FIXME: add echo check
    return true;
  }

  public boolean isConnected() {
    return isConnected; // FIXME: this is kinda lame
  }

  // uses global var 'durations'
  public byte getDurTicks(int loopduration) {
    for( int i=0; i<durations.length; i++ ) {
      if( durations[i] == loopduration )
        return durTicks[i];
    }
    return durTicks[0]; // failsafe
  }
  // this is so lame
  public byte getFadeSpeed(int loopduration) {
    for( int i=0; i<durations.length; i++ ) {
      if( durations[i] == loopduration )
        return fadeSpeeds[i];
    }
    return fadeSpeeds[0]; // failsafe
  }

  /**
   * Burn a list of colors to a BlinkM
   * @param colorlist an ArrayList of the Colors to burn (java Color objs)
   * @param nullColor a color in the list that should be treated as nothing
   * @param duration  how long the entire list should last for, in seconds
   * @param loop      should the list be looped or not
   * @param progressbar if not-null, will update a progress bar
   */
  public void burn(ArrayList colorlist, Color nullColor, 
                   int duration, boolean loop, 
                   JProgressBar progressbar) {

    byte[] cmd = new byte[8];
    byte fadespeed = getFadeSpeed(duration);
    byte durticks = getDurTicks(duration);
    byte reps = (byte)((loop) ? 0 : 1);  // sigh, java

    Color c;

    l.debug("BlinkMComm.burn: durticks:"+durticks+" fadespeed:"+fadespeed);

    // build up the byte array to send
    Iterator iter = colorlist.iterator();
    int i=0;
    while( iter.hasNext() ) {
      //l.debug("BlinkMComm.burn: writing script line "+i);
      c = (Color) iter.next();
      if( c == nullColor )
        c = cBlk;
      cmd[0] = (byte)'W';          // "Write Script Line" command
      cmd[1] = (byte)0;            // script id (0==eeprom)
      cmd[2] = (byte)i;            // script line number
      cmd[3] = (byte)durticks;     // duration in ticks
      cmd[4] = (byte)'c';          // fade to rgb color command
      cmd[5] = (byte)c.getRed();   // cmd arg1
      cmd[6] = (byte)c.getGreen(); // cmd arg2
      cmd[7] = (byte)c.getBlue();  // cmd arg3
      sendCommand( blinkm_addr, cmd );
      if( progressbar !=null) progressbar.setValue(i);  // hack
      i++;
      pause(50);
    }
    // set script length   cmd   id         length         reps
    byte[] cmdsetlength = { 'L', 0, (byte)colorlist.size(), reps };
    sendCommand( blinkm_addr, cmdsetlength );
    pause(50);

    // set boot params   cmd,mode,id,reps,fadespeed,timeadj
    byte cmdsetboot[] = { 'B', 1, 0, 0, fadespeed, reps };
    sendCommand( blinkm_addr, cmdsetboot );
    pause(50);

    // set fade speed
    byte[] cmdsetfade = { 'f', fadespeed };
    sendCommand( blinkm_addr, cmdsetfade );
    pause(30);

    // and cause the script to be played 
    //                 cmd,id,reps,pos
    byte[] cmdplay = { 'p', 0, reps, 0 };
    sendCommand( blinkm_addr, cmdplay );
    pause(30);
  }

  // prepare blinkm for playing preview scripts
  public void prepareForPreview(int loopduration) {
    byte fadespeed = getFadeSpeed(loopduration);
    l.debug("BlinkmComm.prepareForPreview: fadespeed:"+fadespeed);
    byte[] cmdstop    = {'o'};
    byte[] cmdsetfade = {'f', fadespeed};
    if( isConnected() ) {
      sendCommand( blinkm_addr, cmdstop );
      pause(40);
      sendCommand( blinkm_addr, cmdsetfade );
      pause(40);
    }
  }

  // stops any playing script
  public void stopPlayingScript() {
    l.debug("BlinkmComm.stopPlayingScript");
    byte[] cmd = {'o'};
    if( isConnected() ) 
      sendCommand( blinkm_addr, cmd );
  }
  
  // plays a blinkm light script
  // note: doesn't check for connectedness first
  public void playScript(int script_id, int reps, int pos) {
    byte[] cmd = { 'p', (byte)script_id, (byte)reps, (byte)pos};
    sendCommand( blinkm_addr, cmd );
  }

  // plays the eeprom script (script id 0)
  public void playScript() {
    if( isConnected() ) {
      playScript(0,0,0);
    }
  }

  /**
   *
   */
  public void sendColor( Color aColor, Color nullColor, int duration ) {
    if( aColor.equals( lastColor ) )   // don't clog the pipes!
      return;

    Color c = (aColor == nullColor) ? cBlk : aColor;
    byte[] cmd={'c',(byte)c.getRed(),(byte)c.getGreen(),(byte)c.getBlue()};
    if( isConnected() ) 
      sendCommand( blinkm_addr, cmd );
    pause(10); // FIXME: hack
    lastColor = aColor;
  }

  /**
   * Send an I2C command to addr, via the BlinkMCommander Arduino sketch
   * Byte array must be correct length
   */
  public synchronized void sendCommand( byte addr, byte[] cmd ) {
    if( fakeIt ) return;        // just pretend
    l.debug("BlinkMComm.sendCommand(): "+(char)cmd[0]+", "+cmd.length);
    byte cmdfull[] = new byte[4+cmd.length];
    cmdfull[0] = 0x01;
    cmdfull[1] = addr;
    cmdfull[2] = (byte)cmd.length;
    cmdfull[3] = 0x00;
    for( int i=0; i<cmd.length; i++) {
      cmdfull[4+i] = cmd[i];
    }
    port.write(cmdfull);
  }

  /**
   * A simple delay
   */
  public void pause( int millis ) {
    try { Thread.sleep(millis); } catch(Exception e) { }
  }

}


// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class BurnDialog extends JDialog implements ActionListener {

  private String msg_uploading = "Uploading...";
  private String msg_done = "Done";
  private String msg_nowplaying = "Now playing sequence stand-alone";
  private String msg_error = "ERROR: not connected to a BlinkM.";
  private String msg_empty = "     ";

  private JLabel msgtop;
  private JLabel msgbot;
  private JProgressBar progressbar;
  private JButton okbut;

  private JButton burnBtn;

  public BurnDialog(Dialog owner, JButton aBurnBtn) {
    //super(owner, "BlinkM Connect",true);  // modal
    super();
    burnBtn = aBurnBtn;
    burnBtn.setEnabled(false);

    setTitle("BlinkM Upload");

    JPanel panel = new JPanel(new GridLayout(0,1));
    panel.setBorder( BorderFactory.createEmptyBorder(20,20,20,20) );

    msgtop = new JLabel(msg_uploading);
    progressbar = new JProgressBar(0, numSlices-1);
    msgbot = new JLabel(msg_nowplaying);
    msgbot.setVisible(false);
    okbut = new JButton("Ok");
    okbut.setVisible(false);
    okbut.addActionListener(this);

    panel.add( msgtop );
    panel.add( progressbar );
    panel.add( msgbot );
    panel.add( okbut );
    getContentPane().add(panel);

    pack();
    setResizable(false);
    setLocationRelativeTo(null); // center it on the BlinkMSequencer
    setVisible(true);
    
    tl.reset(); // stop preview script
    pb.setToPlay();  // rest play button

    // so dumb we have to spawn a thread for this
    new Thread( new Burner() ).start();

  }

  public void actionPerformed(ActionEvent e) {
    burnBtn.setEnabled(true);  // seems like such a hack
    blinkmComm.prepareForPreview(durationCurrent);
    setVisible(false);
  }
      
  public void isDone() {
    msgbot.setVisible(true);
    okbut.setVisible(true);
  }

  class Burner implements Runnable {
    public void run() {
      // test
      //for( int i=0; i<numSlices; i++ ) {
      //  progressbar.setValue(i);
      //  blinkmComm.pause(3000/48);
      //}
      
      // if we are connected to a port, burn to port
      if (blinkmComm.isConnected()) {
        tl.stop();
        ArrayList colorlist = new ArrayList();
        for( int i=0; i<numSlices; i++) 
          colorlist.add( tl.timeSlices[i].getColor());
        msgtop.setText( msg_uploading );
        
        // burn the list, and saying which colors are 'unused'
        blinkmComm.burn( colorlist, tlDarkGray, durationCurrent,
                         tl.getLoop(), progressbar);
        
        msgtop.setText( msg_uploading + msg_done );
        msgbot.setText( msg_nowplaying );
      } 
      else {
        progressbar.setVisible(false);
        msgtop.setText(msg_error);
        msgbot.setText("");
      }
      
      isDone();
    } // run
  }
}




  /*
  private Choice portChoice;
  private Button closeBtn;
  private Label msg;
  private int w, h;
  private final Dialog confDialog;

  /**
   *
   *
  public BurnDialog() {
    super(new Frame(), "BlinkM Upload", false);
    confDialog = new Dialog(new Frame(), "Burn Confirmation", true);  

    // set layout manager
    this.setLayout(new FlowLayout());

    // listen for window closing events
    this.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          setVisible(false);
        }
      });

    // if we are connected to a port, burn to port
    if (blinkmComm.isConnected()) {
      tl.stop();
      ArrayList colorlist = new ArrayList();
      for( int i=0; i<numSlices; i++) 
        colorlist.add( tl.timeSlices[i].getColor());
      updateMessage("Uploading....");

      // burn the list, and saying which colors are 'unused'
      blinkmComm.burn( colorlist, tlDarkGray, durationCurrent, tl.getLoop());

      updateMessage("Done.");
      try { Thread.sleep(1000); } catch(Exception e) {} //FIXME: hack
      setVisible(false);
    } 
    else {
      updateMessage("ERROR: Check connection.");
    }

  }

  // this is such a hack
  void updateMessage(String str) {
    msg = new Label(str);
    this.add(msg);
    // center, size, and show dialog
    //this.w = 400;
    //this.h = 200;
    this.setSize(400, 200); // this.w, this.h);
    util.centerComp(this);
    this.show();
  }

  /**
   *
   *
  public void burnConf() {
    // set layout manager
    confDialog.setLayout(new FlowLayout());

    Label msg = new Label("\n\nBurned...\n\n");
    confDialog.add(msg);

    // listen for window closing events
    confDialog.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          confDialog.setVisible(false);
        }
      }); 

    // add close button
    Button okBtn = new Button("OK");
    okBtn.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          confDialog.setVisible(false);
          l.debug("ok...");
        }    
      });

    confDialog.add(okBtn);

    confDialog.setSize(this.w, this.h);
    util.centerComp(confDialog);
    confDialog.show();
  }

}

  */

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

// Copyright (c) 2007-2008, ThingM Corporation

/**
 * A pseudo-LED 
 *
 */
public class ColorPreview extends JPanel {
  private Color colorCurrent = new Color(100, 100, 100);
  private Color colorTarget = new Color(100, 100, 100);
  private javax.swing.Timer fadetimer;

  // turning this off makes it more time-accurate on slower computers? wtf
  private static final boolean dofade = true;

  public int fadeMillis = 25;
  public int fadespeed  = 25;
  
  /**
   *
   */
  public ColorPreview() {
    super();
    this.setPreferredSize(new Dimension(105, 250));
    this.setBackground(bgLightGray);
    ImageIcon tlText = new Util().createImageIcon("blinkm_text_preview.gif", "Preview");
    JLabel tlLabel = new JLabel(tlText);
    this.add(tlLabel);
    if( dofade ) {
      fadetimer = new javax.swing.Timer(fadeMillis, new ColorFader());
      fadetimer.start();
    }
  }

  /**
   * @Override
   */
  public void paintComponent(Graphics g) {
    Graphics2D g2 = (Graphics2D) g;
    super.paintComponent(g2); 
    g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, 
                        RenderingHints.VALUE_ANTIALIAS_ON);
    g2.setColor(colorCurrent);
    g2.fillRect(10, 70, 80, 160);
  }

  /**
   *
   */
  public void setColor(Color c) {
    if( dofade ) {
      fadespeed = getFadeSpeed(durationCurrent,numSlices,fadeMillis);
      colorTarget = c;
    } else {
      colorCurrent = c;
    }
    // make BlinkM color match preview color
    blinkmComm.sendColor(c, tlDarkGray, durationCurrent);
      
    repaint();
  }

  /**
   *
   */
  public Color getColor() {
    return this.colorCurrent; 
  }


  public int getFadeSpeed(int loopduration,int numsteps,int fadeMillis) {
    float time_per_step = ((float)loopduration / numsteps);
    float time_half_millis = (time_per_step / 2) * 1000;
    int f =  fadeMillis / (int)time_half_millis;
    //l.debug("ColorPreview: fadeMillis:"+fadeMillis+" time_half:"+time_half_millis+", fadespeed:"+f);
    return 25; // (int)time_half_millis;
  }

  /**
   * Somewhat replicates how BlinkM does color fades
   * called by the fadetimer every tick
   *
   * NOTE: this is constant rate, not constant time
   */
  class ColorFader implements ActionListener {
    public void actionPerformed(ActionEvent e) {
      int r = colorCurrent.getRed();
      int g = colorCurrent.getGreen();
      int b = colorCurrent.getBlue();
          
      int rt = colorTarget.getRed();
      int gt = colorTarget.getGreen();
      int bt = colorTarget.getBlue();
          
      r = color_slide(r,rt, fadespeed);
      g = color_slide(g,gt, fadespeed);
      b = color_slide(b,bt, fadespeed);
      colorCurrent = new Color( r,g,b );

      repaint();
    }

    public int color_slide(int curr, int dest, int step) {
      int diff = curr - dest;
      if(diff < 0)  diff = -diff;
          
      if( diff <= step ) return dest;
      if( curr == dest ) return dest;
      else if( curr < dest ) return curr + step;
      else                   return curr - step;
    }
  }

}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 * A holder for the various dialogs to deal with serial port connecting
 *
 */
public class ConnectDialog extends JDialog implements ActionListener {

  private JComboBox portChoices;
  private JLabel msg;
  private String msg_intro        = "Please select a port";
  private String msg_andclickconn = "and click 'connect'";
  private String msg_intro_disc   = "Always quit or disconnect before";
  private String msg_andclickdisc = "removing Arduino and BlinkM";
  private String msg_connecting   = "connecting...";
  private String msg_connected    = "connected";
  private String butlbl_connect   = "  connect  ";
  private String butlbl_disconnect= "disconnect";

  boolean wasConnected;

  /**
   *
   */
  public ConnectDialog(Dialog owner) {
    super(owner, "BlinkM Connect",true);
    //setTitle("BlinkM Connect");
    
    JPanel panel = new JPanel(new BorderLayout());
    panel.setBorder( BorderFactory.createEmptyBorder(20,20,20,20) );

    String[] portNames = blinkmComm.listPorts();
    String lastPortName = blinkmComm.portName;
    
    if( lastPortName == null ) 
      lastPortName = (portNames.length!=0) ? portNames[0] : null;

    // FIXME: need to catch case of *no* serial ports (setSelectedIndex fails)
    int idx = 0;
    for( int i=0; i<portNames.length; i++) 
      if( portNames[i].equals(lastPortName) ) idx = i;

    JPanel chooserPanel = new JPanel();
    JLabel msgtop = new JLabel("");
    portChoices = new JComboBox(portNames);
    portChoices.setSelectedIndex( idx );
    JButton connectButton = new JButton();
    msg = new JLabel();

    wasConnected = blinkmComm.isConnected();

    if( !wasConnected ) {
      msgtop.setText( msg_intro );
      connectButton.setText(butlbl_connect);
      connectButton.setActionCommand("connect");
      msg.setText(msg_andclickconn);
      chooserPanel.add(portChoices);
    }
    else {
      msgtop.setText( msg_intro_disc );
      connectButton.setText(butlbl_disconnect);
      connectButton.setActionCommand("disconnect");
      msg.setText(msg_andclickdisc);
    }

    connectButton.addActionListener(this);

    chooserPanel.add(connectButton);
    panel.add( msgtop, BorderLayout.NORTH );
    panel.add( chooserPanel, BorderLayout.CENTER );
    panel.add( msg, BorderLayout.SOUTH);
    getContentPane().add(panel);  // jdialog has limited container 

    pack();
    setResizable(false);
    setLocationRelativeTo(null); // center it on the BlinkMSequencer
    setVisible(true);

    // Handle window closing correctly.
    //setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    // not sure diff btwn windowDeactivated vs windowClosing
    addWindowListener( new WindowAdapter() {
        public void windowDeactivated(WindowEvent event) {
          if( !wasConnected && !blinkmComm.isConnected() )
            JOptionPane.showMessageDialog(null, "No port chosen");
        }
      });
  }
  

  // Implement ActionListener
  public void actionPerformed(ActionEvent event) {
    String action = event.getActionCommand();
    println("action: "+action);

    if( "connect".equals(action) ) {
      String portname = (String) portChoices.getSelectedItem();
      msg.setText( msg_connecting );  // why doesn't this refresh?
      try {
        blinkmComm.connect( p, portname );
        blinkmComm.pause(1500); // FIXME: wait for diecimila
        blinkmComm.prepareForPreview(durationCurrent);
      } 
      catch( Exception e ) {
        msg.setText("ERROR getting port: " + e.getMessage());
        e.printStackTrace();
        return;
      }
      msg.setText( msg_connected );  // no one will likely see this
      setVisible(false);
    }
    else if( "disconnect".equals(action) ) { // unused as of yet
      blinkmComm.playScript();
      blinkmComm.disconnect();
      setVisible(false);
    }
  }

}



// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class DurationPullDown extends JComboBox {

  /**
   *
   */
  public DurationPullDown() {
    this.addItem( durations[0]+ " seconds");  
    this.addItem( durations[1]+ " seconds");
    this.addItem( durations[2]+ " seconds");
  } 
}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
class Log {
  public Log() {
    info("Log started");
  }  

  // shortcut call to debug() method
  public void d(Object o) {
    debug(o);
  }  

  public void debug(Object o) {
    println("DEBUG:  " + o.toString());
  }

  public void info(Object o) {
    println("INFO:   " + o.toString());
  }

  public void warn(Object o) {
    println("WARN:   " + o.toString());
  }

  // shortcut call to error() method
  public void err(Object o) {
    error(o);
  }

  public void error(Object o) {
    println("ERROR:  " + o.toString());
  }

}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class MainFrame extends JDialog {

  public Frame f = new Frame();

  private int width, height;
  private PApplet appletRef;     

  /**
   *
   */
  public MainFrame(int w, int h, PApplet appRef) {
    super(new Frame(), "BlinkM Sequencer", false);
    this.setBackground(bgDarkGray);
    this.setFocusable(true);
    this.width = w;
    this.height = h;
    this.appletRef = appRef;

    // handle window close events
    this.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent e) {
          // close mainframe
          dispose();
          // close processing window as well
          appletRef.destroy();
          appletRef.frame.setVisible(false);
          System.exit(0);
        }
      }); 

    // center MainFrame on the screen and show it
    this.setSize(this.width, this.height);
    Dimension scrnSize = Toolkit.getDefaultToolkit().getScreenSize();
    this.setLocation(scrnSize.width/2 - this.width/2, 
                     scrnSize.height/2 - this.height/2);
    this.setVisible(true);
   
  }
}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class PlayButton {
  private ImageIcon iconPlay;
  private ImageIcon iconPlayHov;
  private ImageIcon iconStop;
  private ImageIcon iconStopHov;
  private boolean isPlaying;
  private JButton b;

  /**
   *
   */
  public PlayButton() {
    iconPlay    = new Util().createImageIcon("blinkm_butn_play_on.gif", 
                                             "Play"); 
    iconPlayHov = new Util().createImageIcon("blinkm_butn_play_hov.gif", 
                                             "Play"); 
    iconStop    = new Util().createImageIcon("blinkm_butn_stop_on.gif", 
                                             "Stop"); 
    iconStopHov = new Util().createImageIcon("blinkm_butn_stop_hov.gif", 
                                             "Stop"); 
    b = new JButton();
    b.setOpaque(true);
    b.setBorderPainted( false );
    b.setBackground(bgDarkGray);
    b.setRolloverEnabled(true);
    setIcon();

    b.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          // if we are going from not playing to playing, start timeline
          if (!isPlaying) {
            // stop playing uploaded script, prep for preview playing
            blinkmComm.prepareForPreview(durationCurrent);
            tl.play(); 
          }
          else {
            tl.reset();
          }

          isPlaying = !isPlaying;
          l.debug("Playing: " + isPlaying);
          setIcon();

          // reset timeslice selections
          for( int i=0; i<numSlices; i++) {
            TimeSlice ts = tl.timeSlices[i];
            ts.isActive = false;
          }
          tl.repaint();    
        }
      });
  }

  /**
   *
   */
  public void setIcon() {
    if (isPlaying) {
      b.setIcon(iconStop);
      b.setRolloverIcon(iconStopHov); 
    } 
    else {
      b.setIcon(iconPlay);
      b.setRolloverIcon(iconPlayHov); 
    } 
  }

  /**
   *
   */
  public void setToPlay() {
    b.setIcon(iconPlay);
    b.setRolloverIcon(iconPlayHov); 
    isPlaying = false;
  }
}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class RightPanel extends JPanel {
  JButton connectBtn;
  JButton burnBtn;
  private ImageIcon iconConn;
  private ImageIcon iconConnHov;
  private ImageIcon iconDisc;
  private ImageIcon iconDiscHov;
  public boolean showConnect = true;

  /**
   *
   */
  public RightPanel() {
    this.setPreferredSize(new Dimension(310, 250));
    this.setBackground(bgDarkGray);

    // add play button
    pb = new PlayButton();
    this.add(pb.b);

    // add upload button
    burnBtn = new Util().makeButton("blinkm_butn_upload_on.gif", 
                                    "blinkm_butn_upload_hov.gif",
                                    "Upload to BlinkM", bgDarkGray);
    // action listener for burn button
    burnBtn.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          new BurnDialog(mf,burnBtn);
        }
      });
    this.add(burnBtn);

    // add separator
    ImageIcon connImg = new Util().createImageIcon("blinkm_separator_horiz_larg.gif", "separator horizontal");
    this.add(new JLabel(connImg));

    // add connect button
    iconConn    = new Util().createImageIcon("blinkm_butn_settings_on.gif", 
                                             "Connect"); 
    iconConnHov = new Util().createImageIcon("blinkm_butn_settings_hov.gif", 
                                             "Connect"); 
    iconDisc    = new Util().createImageIcon("blinkm_butn_disconnect_on.gif", 
                                             "Disconnect"); 
    iconDiscHov = new Util().createImageIcon("blinkm_butn_disconnect_hov.gif", 
                                             "Disconnect"); 
    
    // it's so lame one has to do all this
    connectBtn = new JButton();
    connectBtn.setOpaque( true );
    connectBtn.setBorderPainted( false );
    connectBtn.setBackground(bgDarkGray);
    connectBtn.setRolloverEnabled( true );
    setIcon();

    // action listener for connect button
    connectBtn.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          new ConnectDialog(mf);
          if( showConnect && blinkmComm.isConnected() ) {
            showConnect = false; // FIXME: sorta confusing
          }
          else if( !showConnect && !blinkmComm.isConnected() ) {
            showConnect = true;
          }
          setIcon();  // show the (potentially) new icon
        }
      });

    this.add(connectBtn);

  }

  public void setIcon() {
    if( showConnect ) {
      connectBtn.setIcon( iconConn );
      connectBtn.setRolloverIcon( iconConnHov );
    }
    else { 
      connectBtn.setIcon( iconDisc );
      connectBtn.setRolloverIcon( iconDiscHov );
    }
  }

}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class TimeLine extends JPanel implements MouseListener, MouseMotionListener {
  
  private int scrubHeight = 10;
  private int secSpacerWidth = 2;
  private int w = mainWidth;           // overall width of timeline object
  private int h = 90;                  // overall height of timeline object
  private int sx = 18;                 // offset from left edge of screen
  private boolean isMousePressed;
  private Point mouseClickedPt;
  private Point mouseReleasedPt;
  
  private Color playHeadC = new Color(255, 0, 0);
  private float playHeadCurr = sx;
  private javax.swing.Timer timer;
  private int timerMillis = 25;        // time between timer firings
  
  private boolean isPlayHeadClicked = false;
  private boolean isLoop = true;           // timer, loop or no loop
  private long startTime = 0;             // start time 
  
  public TimeSlice[] timeSlices = new TimeSlice[numSlices];
  
  /**
   *
   */
  public TimeLine() {
    this.setPreferredSize(new Dimension(this.w, this.h));
    this.setBackground(bgDarkGray);
    addMouseListener(this);
    addMouseMotionListener(this);
    //mf.addKeyListener(this);
        
    // initialize and add numSlices TimeSlice objects
    // draw guide rects
    int xStep = (this.w / numSlices) - secSpacerWidth;
    int xRemaining = this.w % numSlices - secSpacerWidth;
    int xCurr = sx;
    for (int i=1; i<numSlices+1; i++) {
      TimeSlice ts = new TimeSlice(xCurr, scrubHeight, 
                                   xStep, this.h - scrubHeight);
      if (i%8 == 0) {
        ts.isTicked = true; 
      }
      xCurr += xStep + secSpacerWidth;
      timeSlices[i-1] = ts;
    }
    timeSlices[0].isActive = true; // give people a nudge on what to do
  }

  /**
   * @Override
   */
  public void paintComponent(Graphics gOG) {
    Graphics2D g = (Graphics2D) gOG;
    super.paintComponent(g); 

    // paint light gray background
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, 
                       RenderingHints.VALUE_ANTIALIAS_ON);
    g.setColor(fgLightGray);
    g.fillRect(0, 0, this.getWidth(), scrubHeight);

    // paint each time slice, check if play head is over timeslice
    for( int i=0; i<numSlices; i++) {
      TimeSlice ts = timeSlices[i];
      if (ts.isCollision((int)playHeadCurr)) {
        // update ColorPreview panel based on current pos of slider
        colorPreview.setColor(ts.c);
        //colorChooser.setColor(ts.c);
      }
      ts.draw(g);
    }

    paintPlayHead(g);
  }

  /**
   *
   */
  public void paintPlayHead(Graphics2D g) {
    g.setColor(playHeadC);
    g.fillRect((int)playHeadCurr, 0, secSpacerWidth, this.getHeight());
    Polygon p = new Polygon();
    p.addPoint((int)playHeadCurr - 5, 0);
    p.addPoint((int)playHeadCurr + 5, 0);
    p.addPoint((int)playHeadCurr + 5, 5);
    p.addPoint((int)playHeadCurr + 1, 10);
    p.addPoint((int)playHeadCurr - 1, 10);
    p.addPoint((int)playHeadCurr - 5, 5);
    p.addPoint((int)playHeadCurr - 5, 0);    
    g.fillPolygon(p);
  }

  /**
   *
   */
  public void setLoop(boolean b) {
    isLoop = b; 
  }
  public boolean getLoop() {
    return isLoop;
  }

  /**
   *
   */
  public void play() {
    l.debug("starting to play for dur: " + durationCurrent);

    timer = new javax.swing.Timer( timerMillis, new TimerListener());
    //timer.setInitialDelay(0);
    //timer.setCoalesce(true);
    timer.start();
    startTime = System.currentTimeMillis();
  }

  /**
   *
   */
  class TimerListener implements ActionListener {
    /** Handle ActionEvent */
    public void actionPerformed(ActionEvent e) {
      int width = getWidth() - sx;
      // not quite sure why need to add one to durationCurrent here
      int durtmp = (durationCurrent>5) ?durationCurrent+1 : durationCurrent;
      float step = width / (durtmp*1000.0f/timerMillis);
      playHeadCurr += step;
      repaint();

      if (playHeadCurr > width) {        // check for end of timeline
        if (isLoop) {
          reset();
          play(); 
        } 
        else {
          reset();
          pb.setToPlay();
        }
      }
    }
  } 

  /**
   *
   */
  public void stop() {
    l.debug("stop"); 
    if (timer != null) 
      timer.stop();
    l.debug("elapsedTime:"+(System.currentTimeMillis() - startTime));
  }

  /**
   *
   */
  public void reset() {
    stop();
    playHeadCurr = sx;
    repaint();
  }

  public void mouseClicked(MouseEvent e) {
    //l.debug("clicked");
  }

  public void mouseEntered(MouseEvent e) {
    //l.debug("entered");
  }

  public void mouseExited(MouseEvent e) {
    //l.debug("exited");
  }

  public void mousePressed(MouseEvent e) {
    Point mp = e.getPoint();
    Polygon p = new Polygon();  // creating bounding box for mouseclick
    p.addPoint((int)playHeadCurr - 5, 0);
    p.addPoint((int)playHeadCurr + 5, 0);
    p.addPoint((int)playHeadCurr + 5, this.getHeight());
    p.addPoint((int)playHeadCurr - 5, this.getHeight());
    if (p.contains(mp)) {       // check if mouseclick inside box
      isPlayHeadClicked = true;
    }

    if (!isPlayHeadClicked) {
      // test for collision w/ timeslice
      for( int i=0; i<numSlices; i++) {
        TimeSlice ts = timeSlices[i];
        if (ts.isCollision(mp.x)) {
          ts.isActive = true;
        } 
        else if ((e.getModifiers() & InputEvent.META_MASK) == 0) {
          ts.isActive = false; 
        }
      }
    }

    isMousePressed = true;
    mouseClickedPt = mp;

    repaint();
  }

  public void mouseReleased(MouseEvent e) {
    mouseReleasedPt = e.getPoint();
    int clickCnt = e.getClickCount();

    isPlayHeadClicked = false;
    // snap playhead to closest time slice
    for( int i=0; i<numSlices; i++) {
      TimeSlice ts = timeSlices[i];
      if( ts.isActive && clickCnt >= 2 )   // double-click to set color
        colorChooser.setColor(ts.getColor());
      if( ts.isCollision((int)playHeadCurr)) {
        // update ColorPreview panel based on current pos. of slider
        playHeadCurr = ts.x - 1;        //break;
      } 
    }
    repaint();
  }

  public void mouseMoved(MouseEvent e) {
  }

  public void mouseDragged(MouseEvent e) {
    // if playhead is selected movie it
    if (isPlayHeadClicked) {
      playHeadCurr = e.getPoint().x;
          
      // bounds check for playhead
      if (playHeadCurr < 5)
        playHeadCurr = sx ;
      else if (playHeadCurr > this.getWidth() - 5)
        playHeadCurr = this.getWidth() - 5;
    } 
    else {
      // make multiple selection of timeslices on mousedrag
      int x = e.getPoint().x;
      for( int i=0;i<numSlices;i++) {
        TimeSlice ts = timeSlices[i];
        if (ts.isCollision(mouseClickedPt.x, x) )
          ts.isActive = true;
      }
    }
      
    repaint();
  }
  
} // TimeLine


/**
 * Represents a single slice of time on the timeline. 
 * There are 'numSlices' time slices, regardless of duration.
 */
public class TimeSlice {
  private int x, y, w, h;
  private boolean isActive;
  private boolean isTicked;
  private Color c = tlDarkGray;

  /**
   *
   */
  public TimeSlice(int x, int y, int w, int h) {
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
  }

  /**
   *
   */
  public void draw(Graphics2D g) {
    g.setColor(c);
    g.fillRect(x, y, w, h);
    if (this.isTicked) {
      g.setColor(bgDarkGray);
      g.fillRect(x+w, 5, 2, h);  
    }

    if (this.isActive) {
      BasicStroke wideStroke = new BasicStroke(2.0f);
      g.setStroke(wideStroke);
      g.setColor(highLightC);
      g.drawRect(x, y, w, h-1);
    }
  }

  /**
   *
   */
  public boolean isCollision(int x) {
    return (x <= (this.x + this.w) && x >= this.x); 
  }

  /**
   * ???
   */
  public boolean isCollision(int x1, int x2) {
    if( x2 > x1 ) 
      return (x1 <= (this.x + this.w) && x2 >= this.x);
    else 
      return (x2 <= (this.x + this.w) && x1 >= this.x);
  }

  /**
   *
   */
  public void setColor(Color c) {
    this.c = c;
  }

  /**
   *
   */
  public Color getColor() {
    return this.c; 
  }
}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class TopPanel extends JPanel {

  DurationPullDown durChoice = new DurationPullDown();
    
  /**
   *
   */
  public TopPanel() {
    // set color of this panel
    setBackground(bgLightGray);
        
    // add space label
    JLabel spc = new JLabel("  ");  // wow, what a hack, but so easy
    this.add(spc);
        
    // add Timeline title
    ImageIcon tlText = new Util().createImageIcon("blinkm_text_timeline.gif",
                                                  "TIMELINE");
    JLabel tlLabel = new JLabel(tlText);
    this.add(tlLabel);
    ImageIcon tlSubText = new Util().createImageIcon("blinkm_text_select.gif",
                                                     "Select time slice");
    JLabel tlSubLabel = new JLabel(tlSubText);
    this.add(tlSubLabel);
        
    // add separator
    ImageIcon sepImg = new Util().createImageIcon("blinkm_separator_vert_small.gif", "vert separator");
    JLabel sep1 = new JLabel(sepImg);
    this.add(sep1);
        
    // add loop label
    ImageIcon loopTxt = new Util().createImageIcon("blinkm_text_loop_speed.gif", "Loop Speed");
    JLabel loopLbl = new JLabel(loopTxt);
    this.add(loopLbl);
        
    // action listener for duration choice pull down
    durChoice.setBackground(bgLightGray);
    durChoice.addItemListener(new ItemListener() {
        public void itemStateChanged(ItemEvent ie) {
          int indx = durChoice.getSelectedIndex();
          durationCurrent = durations[indx];
          blinkmComm.prepareForPreview(durationCurrent);
          //l.debug("duration: " + durationCurrent);
          //tl.reset();
        }        
      }
                              );
    this.add(durChoice);
        
    // add Loop Check Box
    ImageIcon loopCheckIcn = new Util().createImageIcon("blinkm_text_loop.gif",
                                                        "Loop");
    JLabel loopCheckLbl = new JLabel(loopCheckIcn);
    this.add(loopCheckLbl);
    JCheckBox loopCheck = new JCheckBox("", true);
    loopCheck.setBackground(bgLightGray);
    this.add(loopCheck);
    ActionListener actionListener = new ActionListener() {
        public void actionPerformed(ActionEvent actionEvent) {
          AbstractButton abButton = (AbstractButton) actionEvent.getSource();
          boolean selected = abButton.getModel().isSelected();
          tl.setLoop(selected);
                
        }
      };
    loopCheck.addActionListener(actionListener);
        
    // add separator
    JLabel sep2 = new JLabel(sepImg);
    this.add(sep2);
        
    // add Help button
    JButton helpBtn = new Util().makeButton("blinkm_butn_help_on.gif", 
                                            "blinkm_butn_help_hov.gif", 
                                            "Help", bgLightGray);
    this.add(helpBtn);
        
    helpBtn.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          l.debug("help...");
          p.link("http://thingm.com/products/blinkm/help", "_blank"); 
        }    
      }
                              );
        
    // add About button
    JButton aboutBtn = new Util().makeButton("blinkm_butn_about_on.gif", 
                                             "blinkm_butn_about_hov.gif", 
                                             "About", bgLightGray);
    this.add(aboutBtn);
        
    aboutBtn.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent ae) {
          l.debug("help...");
          p.link("http://thingm.com/products/blinkm", "_blank"); 
        }    
      }
                               );
        
    JLabel spc2 = new JLabel("      ");  // FIXME: ???
    this.add(spc2);
  }
    
}

// Copyright (c) 2007-2008, ThingM Corporation

/**
 *
 */
public class Util {
  /** 
   * Returns an ImageIcon, or null if the path was invalid. 
   */
  public ImageIcon createImageIcon(String path, String description) {
    java.net.URL imgURL = getClass().getResource(path);
    if (imgURL != null) {
      return new ImageIcon(imgURL, description);
    } 
    else {
      System.err.println("Couldn't find file: " + path);
      return null;
    }
  }

  /**
   *
   */
  public void centerComp(Component c) {
    Dimension scrnSize = Toolkit.getDefaultToolkit().getScreenSize();
    c.setBounds(scrnSize.width/2 - c.getWidth()/2, scrnSize.height/2 - c.getHeight()/2, c.getWidth(), c.getHeight());
  }

  /**
   *
   */
  public JButton makeButton(String onImg, String rollImg, String txt, Color bgColor) {
    ImageIcon btnImg = createImageIcon(onImg, txt);
    JButton b = new JButton(btnImg);
    //setContentAreaFilled( false );
    b.setOpaque(true);
    b.setBorderPainted( false );
    b.setBackground(bgColor);

    if (rollImg != null && !rollImg.equals("")) {
      b.setRolloverEnabled(true);
      ImageIcon img = createImageIcon(rollImg, txt);
      b.setRolloverIcon(img); 
    }

    return b;
  }

}

  static public void main(String args[]) {     PApplet.main(new String[] { "BlinkMSequencer" });  }}