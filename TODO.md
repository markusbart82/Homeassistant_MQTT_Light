# Get LEDs to light up correctly
* Is library called correctly?
* Are color values calculated correctly? Repeated "ON" commands lead to different white balance...

# Unburden the ISR for receiving a message
* add MessageHandler buffer to store received message and to process it later, during main loop
* add "memory busy" flag to avoid overwriting this buffer if messages come in too quickly (is this an issue?)

# Fix errors
* find cause for random reconnects (Wifi)

# Cleanup
* remove printlns from Channelmanager::loop()
