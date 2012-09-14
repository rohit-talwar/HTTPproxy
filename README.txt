---------------------------------------------------------------
			HTTP PROXY
---------------------------------------------------------------
Kshitij Prasad  https://github.com/kshitijiiith
Rohit Talwar  https://github.com/rohit-talwar

TCP based proxy web server

Features


Local Cache
Data Filter
Multiple requests can be issued by client
Multiple clients can be handled at same time

Language - C++ is used
Pthread library is needed
	

Technical
Screen Flow - proxy


1. Proxy waits for connection
2. Connection recieved
3. make a thread to handle the get req
4. Print GET req
5. Check if conditional get
6. check is cache control is on
7. send data
8. Select a place to store the file
9. Map the file into a data structure map
10. Save data in cache folder
11. file name is present in the data structure map
12. print in the log file about the status
13. check for further request at the same time


Screen Flow - client


1. connect to proxy
2. Connection recieved
3. enter data/http request
3. thread created
 3.a check if already similar http is sent
 3.b if so make a conditional req
 3.c else send normal req
4. wait to receive data
5. Print data
6. enter more data/http request



Data structures used

We have used a map to store the requested url. The URL is mapped to a integer and the name of the file in which (if caching is on) the data will be saved


Cache Design:

Consistent Objects- 

1. Folder - ProxyServerCache
	Contains upto 100 html files
2. LogFile.txt
	Contains the log data

No Temporary files created as such

The map gets destroyed and a new map is created when the user restarts the client program.
The map contains the mapping information - i.e the knowledge about what file contains which url�s data

The data can be interpreted by any browser.
