import de.bezier.data.sql.*; //Library for the MySQL connection
import grafica.*; //Library from the plotting
import java.util.Date;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Calendar;

MySQL msql;

int[] percentStdVariation; //Creates an int array

int MAX_SENSOR_READING = 720;

Map<Integer, GPointsArray> plottingData;

//Create an array to store the points for plotting
GPointsArray pointsMin;
GPointsArray pointsMax;
GPointsArray pointsAvg;

GPlot plot;

//Enters the setup() 
void setup()
{
    // width, height
    size( 800, 600 ); //Sets the size of the canvas

    //Connection parameters to the database
    String user     = "hwr";
    String pass     = "hwr_e_14";
    String database = "hwr2014e_db";
    String server   = "jclarsen.dk";
    
    //Connects to the database
    msql = new MySQL( this, server, database, user, pass );
    
    if ( msql.connect() ) //If database connection is made
    {
        msql.query("SELECT count(sensor_data) FROM hwr2014e_db.sensors WHERE group_no='3';");
        msql.next();
      
        int rows = msql.getInt(1);
        
        pointsMin = new GPointsArray(rows);  
        pointsMax = new GPointsArray(rows);
        pointsAvg = new GPointsArray(rows);
      
        // date_time BETWEEN '2014-11-16' AND '2014-11-22';");
        //msql.query("SELECT date_time, sensor_name FROM hwr2014e_db.sensors WHERE group_no='3' ORDER BY date_time DESC LIMIT " + rows + ";");
        msql.query("SELECT date_time, sensor_name FROM hwr2014e_db.sensors WHERE group_no='3' ORDER BY date_time ASC LIMIT " + 40 + ";");

        List<Date> dates = new ArrayList<Date>(); //Creates an int array
        List<Float> percentMin = new ArrayList<Float>(); //Creates an int array
        List<Float> percentMax = new ArrayList<Float>(); //Creates an int array
        List<Float> percentAvg = new ArrayList<Float>(); //Creates an int array
        List<Float> percentStdVariation = new ArrayList<Float>(); //Creates an int array
        
        //for (int i = nPoints - 1; i > 0; i--)
        while(msql.next()) 
        {
          String[] data = msql.getString(2).split(",");
          dates.add(new Date(msql.getTimestamp(1).getTime()));
          percentAvg.add(float(data[0])*100/MAX_SENSOR_READING);
          percentStdVariation.add(float(data[1]));
          percentMax.add(float(data[2])*100/MAX_SENSOR_READING);
          percentMin.add(float(data[3])*100/MAX_SENSOR_READING);
        }
        
        plottingData = new HashMap<Integer, GPointsArray>();
        
        for (Date d : dates)
        {
            Calendar c = Calendar.getInstance();
            c.setTime(d);
            int week = c.get(Calendar.WEEK_OF_YEAR);
            
            GPointsArray arr = plottingData.get(week);
            
            if (arr == null)
            {  
               arr = new GPointsArray(60 * 24 * 7); // there are 1 element per minute. 60 times an hour, 24 hours a day, 7 days
               plottingData.put(week, arr);
            }
            
            int dayOfWeek = c.get(Calendar.DAY_OF_WEEK);
			int dayOfWeek = c.get(Calendar.DAY_OF_WEEK);
			
        }
        
        
        
        println(plottingData.size());
        
        //Populating the points into the points array
        for (int i = 0; i < rows; i++) {
          pointsMin.add(i, percentMin.get(i));
          pointsMax.add(i, percentMax.get(i));
          pointsAvg.add(i, percentAvg.get(i));//, new String[] { str(percentStdVariation[i]) });
        }
        
        println(percentMin.size());
    }
    else
    {
        // connection failed !
    }
    
     // Create a new plot and set its position on the screen
    plot = new GPlot(this, 0, 0);
    
    // Set the plot title and the axis labels
    plot.setTitleText("Noise level");
    plot.getXAxis().setAxisLabelText("Measurement points");
    plot.getYAxis().setAxisLabelText("Percent");
    
    plot.setPoints(pointsAvg);
    plot.setLineColor(color(0,255,0));
    
    plot.addLayer("MinValue", pointsMin);
    plot.getLayer("MinValue").setLineColor(color(255,0,0));
   
    plot.addLayer("MaxValue", pointsMax);
    plot.getLayer("MaxValue").setLineColor(color(0,0,255));
    
    plot.activatePointLabels();

// Draw it!
  //plot.defaultDraw();
  plot.beginDraw();
  plot.drawBackground();
  plot.drawBox();
  plot.drawXAxis();
  plot.drawYAxis();
  plot.drawTitle();
  plot.drawLines();
  plot.drawLabels();
  plot.drawLegend(new String[] {"Average", "Minimum", "Maximum"}, new float[] { 0.45, 0.65, 0.85}, new float[] { 0.92, 0.92, 0.92});
  plot.endDraw();
}

void draw()
{
 
  
}
