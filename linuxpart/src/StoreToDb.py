
# Stores a collection of sensor inputs to a MySQL database
# Input: int group_no, string sensor_name, data
import mysql.connector
from sys import argv
#from datetime import datetime
try:
    cnx = mysql.connector.connect(user='hwr',
        		      password='hwr_e_14',
                              host='jclarsen.dk',
                              database='hwr2014e_db')
    #cnx = mysql.connector.connect(user='hwr',
    #    		      password='12345678',
    #                          host='localhost',
    #                          database='hardware')
except mysql.connector.Error as err:
  if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
    print("Something is wrong with your user name or password")
  elif err.errno == errorcode.ER_BAD_DB_ERROR:
    print("Database does not exists")
  else:
    print("Other error")
    print(err)
else:
    print("Received following query");
    print(argv[1]);
    curser = cnx.cursor();

    query = argv[1]

    curser.execute(query);
    #print("inserting ", argv[1], argv[2], argv[3])
    #data_sensor = ( datetime.now(), int(argv[1]), str(argv[2]), float(argv[3])) 
    #curser.execute(add_sensor_val, data_sensor)
    cnx.commit();
    cnx.close();
