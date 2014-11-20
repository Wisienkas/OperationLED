
# Stores a collection of sensor inputs to a MySQL database
# Input: unixtime sensor_name data
# example: 1416506319 'sensorname' 10
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
    print("Received following query")

    sql = "INSERT INTO sensors (date_time, group_no, sensor_name, sensor_data) VALUES(%s, %s, %s, %s);"
    curser = cnx.cursor()

    j = 1; # first index with useable data
    dataRange = (len(argv) - 1)/3 # amount of useable data
    
    for i in range(0, dataRange):
        data_sensor = list()
        data_sensor.append(datetime.fromtimestamp(int(argv[j + i + 0])))
        data_sensor.append(3)
        data_sensor.append(argv[j + i + 1])
        data_sensor.append(argv[j + i + 2])
        curser.execute(sql, data_sensor)
        j += 1
    
    cnx.commit()
    cnx.close()
