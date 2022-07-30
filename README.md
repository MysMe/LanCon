# LanCon

A non-essential component of the overall Stocker project.  
This library is designed to allow block messages to be sent to other devices on a connected LAN without knowing their IP address in advance.  
It does so using a "call and response" method for obtaining details followed by a block message delivery.  
The library can be built as a command line tool, enabling easy testing.  
When built as a library, functionality is exported over a C interface.  
The application relies on ASIO.  

Additional components of the Stocker project:  
- StockerBackend: https://github.com/MysMe/StockerBackend  
A library containing common core functionality of the Stocker project.  
- StockerFrontend: https://github.com/MysMe/StockerFrontend  
A Windows desktop application for displaying inventory information with basic management functionality.  
- StockerAndroid: https://github.com/MysMe/StockerAndroid  
An android mobile application for portably inputting inventory information.
