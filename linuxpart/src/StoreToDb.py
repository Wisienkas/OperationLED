
# Stores a collection of sensor inputs to a MySQL database
# Input: int group_no, string sensor_name, data
import mysql.connector
from sys import argv
from datetime import datetime
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

    sql = "INSERT INTO sensors (time, group_no, sensor_name, sensor_data) VALUES(?, ?, ?, ?);"
    curser = cnx.cursor();
    
    for i in range(0, 5)
        data_sensor( datetime.now(), 3, argv[3 * i + 1], argv[3 * i + 2])
        curser.execute(sql, data_sensor, multi = True)
        cnx.commit();
    cnx.close();
