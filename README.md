# wxHID_API
wxThread based USB HID API for simple access 

Purpose of this HID_API was a simple, free, mostly real time access to USB-HID devices.
All OS uses also for Bluetooth HID device the same protocoll (USB tunneling over Bluetooth).
This means, with this HID API we have access to wired USB and Bluetooth HID-Devices.

The main reason to make this API was developing USB devices with Feature and Output capabilities.
The most implementations, like wxJoystick don´t support Output Reports.

The next problem is, that the USB Endpoint-Interupts are not handled like events to notify the application.
But polling in the main application is complicated and take a lot of time. 
With a timer based polling propally we get problems with fast HID devices and we can get a buffer overrun.
With the wxThread based API we are nearly at real time with the interrupts from the HID device.

So the wxThread based API creates a thread to handle the polling uses a callback function to inform the main application.

To Do:
-The API support only one device at once.
-The preparsed data only supports one report of each type.
(the API of sure can access to all reports, it´s only the enumeration)
