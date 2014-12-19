import grafica.*;

import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.util.Calendar;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import processing.core.*;
import de.bezier.data.sql.*; //Library for the MySQL connection

@SuppressWarnings("serial")
public class ProcessingJava extends PApplet {

	private static final String TABLE = "hwr2014e_db.sensors";

	private static final String SELECT_ROW_COUNT = "SELECT count(sensor_data) FROM "
			+ TABLE + " WHERE group_no='3';";
	private static final String SELECT_ROW_DATA = "SELECT date_time, sensor_name FROM "
			+ TABLE
			+ " WHERE group_no='3' "
			+ " ORDER BY date_time ASC LIMIT %s;";

	private static final String SERVER = "jclarsen.dk";
	private static final String DATABASE = "hwr2014e_db";
	private static final String DB_USER = "hwr";
	private static final String DB_PASS = "hwr_e_14";

	public static final String F_DESC = "%02d:%02d";
	public static final String F_FLOAT = "%d.%02d";

	private static final float MAX_GAIN = 20;

	private static final float[][] coordinates = new float[][] { { 0, 50 },
			{ 400, 50 }, { 0, 350 }, { 400, 350 }, { -1, 50 } };
	private static final float[][] dimensions = new float[][] { { 300, 200 },
			{ 300, 200 }, { 300, 200 }, { 300, 200 } };

	public Map<Long, Data> datamap = new TreeMap<Long, Data>();
	public Map<Integer, SpecificData<Integer>> weekmap = new TreeMap<Integer, SpecificData<Integer>>();
	public Map<Integer, SpecificData<Integer>> datemap = new TreeMap<Integer, SpecificData<Integer>>();
	public Map<Float, SpecificData<Float>> hourmap = new TreeMap<Float, SpecificData<Float>>();

	public GPlot plot1, plot2, plot3, plot4;

	public GPointsArray weekPoints, datePoints, hourPointsAvg, hourPointsMin,
			hourPointsMax, dbPoints;

	public boolean hasChanged = false;

	public int selectedWeek;
	public int selectedDate;
	private int currWidth;
	private int currHeight;

	public void setup() {
		size(850, 650);

		// request redraw when resized
		this.addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				if (e.getSource() == frame) {
					hasChanged = true;
				}
			}
		});

		coordinates[4][0] = width / 2;

		resetFields();
		setupGraphs();
		reloadData();
		reparseMap();
	}

	private void resetFields() {
		selectedWeek = 50;
		selectedDate = 4;
	}

	public void setupGraphs() {
		weekPoints = new GPointsArray(0);
		datePoints = new GPointsArray(0);
		hourPointsAvg = new GPointsArray(0);
		hourPointsMin = new GPointsArray(0);
		hourPointsMax = new GPointsArray(0);
		dbPoints = new GPointsArray(0);

		// Setup of plot 1
		plot1 = new GPlot(this);
		plot1.setPos(coordinates[0]);
		plot1.setDim(dimensions[0]);
		plot1.getTitle().setText("Weeks");
		plot1.getYAxis().getAxisLabel().setText("Avg noise per week");
		plot1.startHistograms(GPlot.VERTICAL);
		plot1.getHistogram().setDrawLabels(true);
		plot1.getHistogram().setRotateLabels(true);

		// Setup of plot 2
		plot2 = new GPlot(this);
		plot2.setPos(coordinates[1]);
		plot2.setDim(dimensions[1]);
		plot2.getTitle().setText("Day in week");
		plot2.getYAxis().getAxisLabel().setText("Avg noise per day");
		plot2.startHistograms(GPlot.VERTICAL);
		plot2.getHistogram().setDrawLabels(true);
		plot2.getHistogram().setRotateLabels(true);

		// Setup of plot 3
		plot3 = new GPlot(this);
		plot3.setPos(coordinates[2]);
		plot3.setDim(dimensions[2]);
		plot3.setLineColor(color(255, 242, 0));
		plot3.getTitle().setText("Hours in day");
		plot3.getYAxis().getAxisLabel().setText("Noise");
		plot3.getXAxis().getAxisLabel().setText("Hour");
		plot3.getXAxis().setDrawAxisLabel(true);
		plot3.getXAxis().setDrawTickLabels(true);
		plot3.addLayer("Minimum", hourPointsMin);
		plot3.getLayer("Minimum").setLineColor(color(0, 255, 0));
		plot3.addLayer("Maximum", hourPointsMax);
		plot3.getLayer("Maximum").setLineColor(color(255, 0, 0));
		plot3.setHorizontalAxesNTicks((24 / 4) + 1);

		// Setup of plot 4
		plot4 = new GPlot(this);
		plot4.setPos(coordinates[3]);
		plot4.setDim(dimensions[3]);
		plot4.setYLim(0, MAX_GAIN);
		plot4.setLineColor(color(150, 150, 255));
		plot4.getTitle().setText("Gain between min and max in dB");
		plot4.getYAxis().getAxisLabel().setText("dB");
		plot4.setHorizontalAxesNTicks((24 / 4) + 1);
		plot4.getXAxis().getAxisLabel().setText("Hour of day");
		plot4.getXAxis().setDrawAxisLabel(true);
		plot4.getXAxis().setDrawTickLabels(true);
	}

	public void reloadData() {
		// Connects to the database
		SQL msql = new MySQL(this, SERVER, DATABASE, DB_USER, DB_PASS);

		if (!msql.connect()) // If database connection is not made
		{
			println("Cannot connect to sql");
			status.setText("Cannot connect to sql", true);
		}

		msql.query(SELECT_ROW_COUNT);
		msql.next();

		int rows = msql.getInt(1);

		msql.query(String.format(SELECT_ROW_DATA, rows));

		datamap.clear();

		while (msql.next()) {
			String[] data = msql.getString(2).split(",");

			long date = msql.getTimestamp(1).getTime();
			float avg = Float.parseFloat(data[0]);
			float stdvariation = Float.parseFloat(data[1]);
			float max = Float.parseFloat(data[2]);
			float min = Float.parseFloat(data[3]);

			datamap.put(date, new Data(date, min, max, avg, stdvariation));
		}
	}

	public void reparseMap() {
		reparseMap(true);
	}

	public void reparseMap(boolean requestRedraw) {
		weekmap.clear();
		datemap.clear();
		hourmap.clear();

		float week_min = Float.MAX_VALUE;
		float week_max = -1;

		float hour_min = Float.MAX_VALUE;
		float hour_max = -1;

		for (Map.Entry<Long, Data> entry : datamap.entrySet()) {
			Data entrydata = entry.getValue();

			Calendar cal = Calendar.getInstance();
			cal.setTimeInMillis(entrydata.date);

			int week = cal.get(Calendar.WEEK_OF_YEAR);
			int day = Helper.getDayOfWeek(cal.get(Calendar.DAY_OF_WEEK));
			int hour = cal.get(Calendar.HOUR_OF_DAY);
			int minute = cal.get(Calendar.MINUTE);

			week_min = Math.min(entry.getKey(), week_min);
			week_max = Math.max(entry.getKey(), week_max);

			selectedWeek = (selectedWeek == -1 ? week : selectedWeek);

			Helper.addData(weekmap, week, String.valueOf(week), entrydata);

			if (selectedWeek == week) {
				selectedDate = (selectedDate == -1 ? day : selectedDate);

				Helper.addData(datemap, day, Helper.reverseGetDayOfWeek(day),
						entrydata);

				if (selectedDate == day) {
					String description = String.format(F_DESC, hour, minute);
					float time = Float.valueOf(String.format(F_FLOAT, hour,
							minute));

					Helper.addData(hourmap, time, description, entrydata);
				}
			}
		}

		weekPoints = new GPointsArray(weekmap.size());
		datePoints = new GPointsArray(datemap.size());
		hourPointsAvg = new GPointsArray(hourmap.size());
		hourPointsMin = new GPointsArray(hourmap.size());
		hourPointsMax = new GPointsArray(hourmap.size());
		dbPoints = new GPointsArray(hourmap.size());

		for (Entry<Integer, SpecificData<Integer>> entry : weekmap.entrySet()) {
			weekPoints.add(entry.getKey(), entry.getValue().avg());
		}

		for (Entry<Integer, SpecificData<Integer>> entry : datemap.entrySet()) {
			SpecificData<Integer> val = entry.getValue();

			datePoints.add(entry.getKey(), val.avg(), val.description);
		}

		for (Entry<Float, SpecificData<Float>> entry : hourmap.entrySet()) {
			float key = entry.getKey();
			SpecificData<Float> val = entry.getValue();

			hour_min = Math.min(key, hour_min);
			hour_max = Math.max(key, hour_max);

			float dBgain = Helper.dBGain(val.min(), val.max());

			hourPointsAvg.add(key, val.avg(), val.description);
			hourPointsMin.add(key, val.min(), val.description);
			hourPointsMax.add(key, val.max(), val.description);
			dbPoints.add(key, dBgain, val.description);
		}

		plot1.setPoints(weekPoints);
		plot2.setPoints(datePoints);
		plot3.setPoints(hourPointsAvg);
		plot3.getLayer("Minimum").setPoints(hourPointsMin);
		plot3.getLayer("Maximum").setPoints(hourPointsMax);
		plot4.setPoints(dbPoints);

		plot3.updateLimits();
		plot4.updateLimits();

		if (hour_min != Float.MAX_VALUE && hour_max != -1) {
			plot3.setXLim(hour_min, hour_max);
			plot4.setXLim(hour_min, hour_max);
		} else {
			println("limits: " + hour_min + ", " + hour_max + " over "
					+ hourmap.size() + " elements");
		}

		status.setText(Helper.reverseGetDayOfWeek(selectedDate)
				+ ", week " + selectedWeek
				+ ". Click on a bar so select another date", requestRedraw);

		if (requestRedraw)
			hasChanged = true;
	}

	public void draw() {

		if (currWidth != width || currHeight != height) {
			currWidth = width;
			currHeight = height;
			hasChanged = true;
		}

		if (hasChanged) {

			background(255);

			// Draw the first plot
			plot1.beginDraw();
			plot1.setPoints(weekPoints);
			plot1.drawBackground();
			plot1.drawBox();
			plot1.drawXAxis();
			plot1.drawYAxis();
			plot1.drawRightAxis();
			plot1.drawTitle();
			plot1.drawLabels();
			plot1.drawHistograms();
			plot1.drawGridLines(GPlot.HORIZONTAL);
			plot1.endDraw();

			plot2.beginDraw();
			plot2.setPoints(datePoints);
			plot2.drawBackground();
			plot2.drawBox();
			plot2.drawYAxis();
			plot2.drawRightAxis();
			plot2.drawTitle();
			plot2.drawLabels();
			plot2.drawHistograms();
			plot2.drawGridLines(GPlot.HORIZONTAL);
			plot2.endDraw();

			plot3.beginDraw();
			plot3.drawBackground();
			plot3.drawBox();
			plot3.drawXAxis();
			plot3.drawYAxis();
			plot3.drawRightAxis();
			plot3.drawTitle();
			plot3.drawGridLines(GPlot.BOTH);
			plot3.drawLines();
			plot3.drawLegend(new String[] { "Average", "Minimum", "Maximum" },
					new float[] { 0.20f, 0.5f, 0.8f }, new float[] { 0.92f,
							0.92f, 0.92f });
			plot3.endDraw();

			plot4.beginDraw();
			plot4.setPoints(dbPoints);
			plot4.drawBackground();
			plot4.drawBox();
			plot4.drawXAxis();
			plot4.drawYAxis();
			plot3.drawRightAxis();
			plot4.drawTitle();
			plot4.drawGridLines(GPlot.BOTH);
			plot4.drawLines();
			plot4.endDraw();

			status.draw();

			hasChanged = false;
		}

	}

	public void mouseClicked() {

		if (plot1.isOverBox(mouseX, mouseY)) {

			int choice = Helper.calculateIndex(plot1, weekPoints, this);

			if (weekmap.containsKey(choice)) {
				selectedWeek = choice;

				reparseMap(false);

				if (!datemap.containsKey(selectedDate)) {
					selectedDate = ((TreeMap<Integer, ?>) datemap).firstKey();
				}

				reparseMap();
			}
		} else if (plot2.isOverBox(mouseX, mouseY)) {
			selectedDate = Helper.calculateIndex(plot2, datePoints, this);
			reparseMap();
		}
	}

	public static void main(String[] args) {
		PApplet.main(new String[] { "--present", "ProcessingJava" });
	}

	public static class Data {
		private long date;
		private float min;
		private float max;
		private float avg;
		private float stdvariation;

		public Data(long date, float min, float max, float avg,
				float stdvariation) {
			this.date = date;
			this.min = min;
			this.max = max;
			this.avg = avg;
			this.stdvariation = stdvariation;
		}

		public float getStdvariation() {
			return stdvariation;
		}
	}

	public static class SpecificData<T> {
		private T id;
		private String description;
		private int datasize;
		private float min;
		private float max;
		private float avg;

		public SpecificData(T id, String description) {
			this.id = id;
			this.description = description;
		}

		public void addDataPoint(float avg, float min, float max) {
			this.avg += avg;
			this.min += min;
			this.max += max;
			datasize++;
		}

		public float avg() {
			return avg / datasize;
		}

		public float min() {
			return min / datasize;
		}

		public float max() {
			return max / datasize;
		}

		public T getId() {
			return id;
		}
	}

	public static class Helper {

		public static int calculateIndex(GPlot plot, GPointsArray data,
				PApplet applet) {
			int index = (int) (plot.getRelativePlotPosAt(applet.mouseX,
					applet.mouseY)[0] * (data.getNPoints()));

			index = min(index, data.getNPoints());
			index = max(index, 0);

			return (int) (data.get(index).getX());
		}

		public static float dBGain(float min, float max) {
			return (float) (10 * Math.log10(max / min));
		}

		public static <T> void addData(Map<T, SpecificData<T>> map, T id,
				String description, Data g_data) {
			SpecificData<T> data = map.get(id);

			if (data == null) {
				data = new SpecificData<T>(id, description);
				map.put(id, data);
			}

			data.addDataPoint(g_data.avg, g_data.min, g_data.max);
		}

		/**
		 * 
		 * @param dayOfWeekCalendarAPI
		 *            the values given as DAY_OF_WEEK in the Calendar API
		 * @return the day of week as -1 to 6, where 0 is monday and 6 is
		 *         sunday. Return -1 if invalid
		 */
		public static int getDayOfWeek(int dayOfWeekCalendarAPI) {
			switch (dayOfWeekCalendarAPI) {
			case Calendar.MONDAY:
				return 0;
			case Calendar.TUESDAY:
				return 1;
			case Calendar.WEDNESDAY:
				return 2;
			case Calendar.THURSDAY:
				return 3;
			case Calendar.FRIDAY:
				return 4;
			case Calendar.SATURDAY:
				return 5;
			case Calendar.SUNDAY:
				return 6;
			default:
				return -1;
			}
		}

		/**
		 * @param dayOfWeek
		 *            A value between 0-6, 0 for monday and 6 for sunday
		 * @return the string representation of the number. Returns empty string
		 *         if outside 0-6
		 */
		public static String reverseGetDayOfWeek(int dayOfWeek) {
			switch (dayOfWeek) {
			case 0:
				return "Monday";
			case 1:
				return "Tuesday";
			case 2:
				return "Wednesday";
			case 3:
				return "Thursday";
			case 4:
				return "Friday";
			case 5:
				return "Saturday";
			case 6:
				return "Sunday";
			default:
				return "";
			}
		}
	}

	private final StatusText status = new StatusText();

	public class StatusText {
		private String text = "Click a bar to change data points...";

		public PFont f = createFont("Arial", 16, true); // Arial, 16 point,
														// anti-aliasing on

		public void setText(String text, boolean requestRedraw) {
			this.text = text;
			if (requestRedraw)
				hasChanged = true;
		}

		public void draw() {
			textFont(f, 16);
			fill(0);
			textAlign(CENTER);
			text(this.text, coordinates[4][0], coordinates[4][1]);
		}
	}
}
