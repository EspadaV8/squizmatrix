package ij.plugin.filter;
import java.awt.*;
import java.util.Vector;
import java.util.Properties;
import ij.*;
import ij.gui.*;
import ij.process.*;
import ij.measure.*;
import ij.text.*;
import ij.plugin.MeasurementsWriter;

/** This plugin implements ImageJ's Analyze/Measure and Analyze/Set Measurements commands. */
public class Analyzer implements PlugInFilter, Measurements {
	
	private String arg;
	private ImagePlus imp;
	private ResultsTable rt;
	private int measurements;
	private StringBuffer min,max,mean,sd;
	
	// Order must agree with order of checkboxes in Set Measurements dialog box
	private static final int[] list = {AREA,MEAN,STD_DEV,MODE,MIN_MAX,
		CENTROID,CENTER_OF_MASS,PERIMETER,RECT,ELLIPSE,CIRCULARITY, FERET,
		LIMIT,LABELS,INVERT_Y};

	private static final int UNDEFINED=0,AREAS=1,LENGTHS=2,ANGLES=3,MARK_AND_COUNT=4;
	private static int mode = AREAS;
	private static final String MEASUREMENTS = "measurements";
	private static final String MARK_WIDTH = "mark.width";
	private static final String PRECISION = "precision";
	//private static int counter;
	private static boolean unsavedMeasurements;
	public static Color darkBlue = new Color(0,0,160);
	private static int systemMeasurements = Prefs.getInt(MEASUREMENTS,AREA+MEAN+MIN_MAX);
	public static int markWidth = Prefs.getInt(MARK_WIDTH,3);
	public static int precision = Prefs.getInt(PRECISION,3);
	private static float[] umeans = new float[MAX_STANDARDS];
	private static ResultsTable systemRT = new ResultsTable();
	private static int redirectTarget;
	private static String redirectTitle = "";
	static int firstParticle, lastParticle;
	
	public Analyzer() {
		rt = systemRT;
		rt.setPrecision(precision);
		measurements = systemMeasurements;
	}
	
	/** Constructs a new Analyzer using the specified ImagePlus object
		and the system-wide measurement options and results table. */
	public Analyzer(ImagePlus imp) {
		this();
		this.imp = imp;
	}
	
	/** Construct a new Analyzer using an ImagePlus object and private
		measurement options and results table. */
	public Analyzer(ImagePlus imp, int measurements, ResultsTable rt) {
		this.imp = imp;
		this.measurements = measurements;
		this.rt = rt;
	}
	
	public int setup(String arg, ImagePlus imp) {
		this.arg = arg;
		this.imp = imp;
		IJ.register(Analyzer.class);
		if (arg.equals("sum"))
			{summarize(); return DONE;}
		else if (arg.equals("clear"))
			{clearWorksheet(); return DONE;}
		else
			return DOES_ALL+NO_CHANGES;
	}

	 public void run(ij.process.ImageProcessor ip) {}

		void clearWorksheet() {
		resetCounter();
	}
	 
	void setOptions(GenericDialog gd) {
		int oldMeasurements = systemMeasurements;
		int previous = 0;
		boolean b = false;
		for (int i=0; i<list.length; i++) {
			//if (list[i]!=previous)
			b = gd.getNextBoolean();
			previous = list[i];
			if (b)
				systemMeasurements |= list[i];
			else
				systemMeasurements &= ~list[i];
		}
		if ((oldMeasurements&(~LIMIT))!=(systemMeasurements&(~LIMIT))) {
			if (IJ.macroRunning()) {
				resetCounter();
				mode = AREAS;
			} else
				mode = UNDEFINED;
		}
		if ((systemMeasurements&LABELS)==0)
			systemRT.disableRowLabels();
	}
	
	
	// Update centroid and center of mass y-coordinate
	// based on value "Invert Y Coordinates" flag
	double updateY(double y) {
		if (imp==null)
			return y;
		else {
			if ((systemMeasurements&INVERT_Y)!=0) {
				Calibration cal = imp.getCalibration();
				y = imp.getHeight()*cal.pixelHeight-y;
			}
			return y;
		}
	}

	// Update bounding rectangle y-coordinate based
	// on value "Invert Y Coordinates" flag
	double updateY2(double y) {
		if (imp==null)
			return y;
		else {
			if ((systemMeasurements&INVERT_Y)!=0) {
				Calibration cal = imp.getCalibration();
				y = imp.getHeight()*cal.pixelHeight-y-cal.pixelHeight;
			}
			return y;
		}
	}

	/** Writes the last row in the results table to the ImageJ window. */
	public void displayResults() {
		int counter = rt.getCounter();
		if (counter==1)
			IJ.setColumnHeadings(rt.getColumnHeadings());		
		IJ.write(rt.getRowAsString(counter-1));
	}

	/** Updates the displayed column headings. Does nothing if
	    the results table headings and the displayed headings are
        the same. Redisplays the results if the headings are
        different and the results table is not empty. */
	public void updateHeadings() {
		TextPanel tp = IJ.getTextPanel();
		if (tp==null)
			return;
		String worksheetHeadings = tp.getColumnHeadings();		
		String tableHeadings = rt.getColumnHeadings();		
		if (worksheetHeadings.equals(tableHeadings))
			return;
		IJ.setColumnHeadings(tableHeadings);
		int n = rt.getCounter();
		if (n>0) {
			StringBuffer sb = new StringBuffer(n*tableHeadings.length());
			for (int i=0; i<n; i++)
				sb.append(rt.getRowAsString(i)+"\n");
			tp.append(new String(sb));
		}
	}

	/** Converts a number to a formatted string with a tab at the end. */
	public String n(double n) {
		String s;
		if (Math.round(n)==n)
			s = IJ.d2s(n,0);
		else
			s = IJ.d2s(n,precision);
		return s+"\t";
	}
		
	void incrementCounter() {
		//counter++;
		if (rt==null) rt = systemRT;
		rt.incrementCounter();
		unsavedMeasurements = true;
	}
	
	public void summarize() {
		rt = systemRT;
		if (rt.getCounter()==0)
			return;
		measurements = systemMeasurements;
		min = new StringBuffer(100);
		max = new StringBuffer(100);
		mean = new StringBuffer(100);
		sd = new StringBuffer(100);
		min.append("Min\t");
		max.append("Max\t");
		mean.append("Mean\t");
		sd.append("SD\t");
		if ((measurements&LABELS)!=0) {
			min.append("\t");
			max.append("\t");
			mean.append("\t");
			sd.append("\t");
		}
		if (mode==MARK_AND_COUNT) 
			summarizePoints(rt);
		else if (mode==LENGTHS) 
			add2(rt.getColumnIndex("Length"));
		else if (mode==ANGLES) 
			add2(rt.getColumnIndex("Angle"));
		else
			summarizeAreas();
		TextPanel tp = IJ.getTextPanel();
		if (tp!=null) {
			String worksheetHeadings = tp.getColumnHeadings();		
			if (worksheetHeadings.equals(""))
				IJ.setColumnHeadings(rt.getColumnHeadings());
		}		
		IJ.write("");		
		IJ.write(new String(mean));		
		IJ.write(new String(sd));		
		IJ.write(new String(min));		
		IJ.write(new String(max));
		IJ.write("");		
		mean = null;		
		sd = null;		
		min = null;		
		max = null;		
	}
	
	void summarizePoints(ResultsTable rt) {
		add2(rt.getColumnIndex("X"));
		add2(rt.getColumnIndex("Y"));
		add2(rt.getColumnIndex("Value"));
	}

	void summarizeAreas() {
		if ((measurements&AREA)!=0) add2(ResultsTable.AREA);
		if ((measurements&MEAN)!=0) add2(ResultsTable.MEAN);
		if ((measurements&STD_DEV)!=0) add2(ResultsTable.STD_DEV);
		if ((measurements&MODE)!=0) add2(ResultsTable.MODE);
		if ((measurements&MIN_MAX)!=0) {
			add2(ResultsTable.MIN);
			add2(ResultsTable.MAX);
		}
		if ((measurements&CENTROID)!=0) {
			add2(ResultsTable.X_CENTROID);
			add2(ResultsTable.Y_CENTROID);
		}
		if ((measurements&CENTER_OF_MASS)!=0) {
			add2(ResultsTable.X_CENTER_OF_MASS);
			add2(ResultsTable.Y_CENTER_OF_MASS);
		}
		if ((measurements&PERIMETER)!=0)
			add2(ResultsTable.PERIMETER);
		if ((measurements&RECT)!=0) {
			add2(ResultsTable.ROI_X);
			add2(ResultsTable.ROI_Y);
			add2(ResultsTable.ROI_WIDTH);
			add2(ResultsTable.ROI_HEIGHT);
		}
		if ((measurements&ELLIPSE)!=0) {
			add2(ResultsTable.MAJOR);
			add2(ResultsTable.MINOR);
			add2(ResultsTable.ANGLE);
		}
		if ((measurements&CIRCULARITY)!=0)
			add2(ResultsTable.CIRCULARITY);
		if ((measurements&FERET)!=0)
			add2(ResultsTable.FERET);
	}

	private void add2(int column) {
		float[] c = column>=0?rt.getColumn(column):null;
		if (c!=null) {
			ImageProcessor ip = new FloatProcessor(c.length, 1, c, null);
			if (ip==null)
				return;
			ImageStatistics stats = new FloatStatistics(ip);
			if (stats==null)
				return;
			mean.append(n(stats.mean));
			min.append(n(stats.min));
			max.append(n(stats.max));
			sd.append(n(stats.stdDev));
		} else {
			mean.append("-\t");
			min.append("-\t");
			max.append("-\t");
			sd.append("-\t");
		}
	}

	/** Returns the current measurement count. */
	public static int getCounter() {
		return systemRT.getCounter();
	}

	/** Sets the measurement counter to zero. Displays a dialog that
	    allows the user to save any existing measurements. Returns
	    false if the user cancels the dialog.
	*/
	public synchronized static boolean resetCounter() {
		TextPanel tp = IJ.isResultsWindow()?IJ.getTextPanel():null;
		int counter = systemRT.getCounter();
		int lineCount = tp!=null?IJ.getTextPanel().getLineCount():0;
		ImageJ ij = IJ.getInstance();
		if (counter>0 && lineCount>0 && unsavedMeasurements && !IJ.macroRunning() && ij!=null) {
			SaveChangesDialog d = new SaveChangesDialog(ij, "Save "+counter+" measurements?");
			if (d.cancelPressed())
				return false;
			else if (d.savePressed())
				new MeasurementsWriter().run("");
		}
		umeans = null;
		systemRT.reset();
		unsavedMeasurements = false;
		if (tp!=null) {
			tp.selectAll();
			tp.clearSelection();
		}
		return true;
	}
	
	public static void setSaved() {
		unsavedMeasurements = false;
	}
	
	// Returns the measurements defined in the Set Measurements dialog. */
	public static int getMeasurements() {
		return systemMeasurements;
	}

	// Sets the system-wide measurements. */
	public static void setMeasurements(int measurements) {
		systemMeasurements = measurements;
	}

	/** Called once when ImageJ quits. */
	public static void savePreferences(Properties prefs) {
		prefs.put(MEASUREMENTS, Integer.toString(systemMeasurements));
		prefs.put(MARK_WIDTH, Integer.toString(markWidth));
		prefs.put(PRECISION, Integer.toString(precision));	}

	/** Returns an array containing the first 20 uncalibrated means. */
	public static float[] getUMeans() {
		return umeans;
	}

	/** Returns the ImageJ results table. */
	public static ResultsTable getResultsTable() {
		return systemRT;
	}

	/** Returns the number of digits displayed on the right of decimal point. */
	public static int getPrecision() {
		return precision;
	}

	/** Returns an updated Y coordinate based on
		the current "Invert Y Coordinates" flag. */
	public static int updateY(int y, int imageHeight) {
		if ((systemMeasurements&INVERT_Y)!=0)
			y = imageHeight-y-1;
		return y;
	}
	
}
	
