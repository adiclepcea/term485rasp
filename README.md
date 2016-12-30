# 485 thermostat app

The thermostat for this application uses a non standard protocol.
This application is meant to be run from a raspberry. 
The application polls the gpio port where the 485 line is connected and translates the signal into a known package.
This package will then be sent to a remote server and saved locally.

If the application is compiled on a desktop computer, it will use a mock reader that will feed reads from a capture file captured on a raspberry.


