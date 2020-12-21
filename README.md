# gts-solar-car
Solar car for Gateway to Science. Component 1004

Exhibit consists of a small plastic solar car kit that drives in circles within a cabinet.
A halogen light is aimed by visitors at the car to make the car move.

An arduino (metro mini) controls a circle of Neopixel RBG LEDs around the track to create a virtual pace car to race against.
It also determines when the race is over and uses a motor to reset the solar car back to the starting line.


This code is based on the 1-year prototype that has been in use at GTS for over a year. 
A few months ago two new features were added:
- wheel encoder instead of 2nd hall sensor for better position tracking
- PWM halogen bulb control so "off" still keeps the bulb warm which should lead to longer bulb life.

Now as I commit changes for the final, I realize I could have written much cleaner code. I apologize if it is hard to follow, I don't foresee more features being added at this point and since I have working code I am not going to rewrite it.  -Joe
