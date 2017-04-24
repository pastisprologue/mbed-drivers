# Hey mbed Community!

I’ve been using mbed a bunch and now I finally feel like I have something I can contribute back!  

I was using the standard drivers for quite a while until I ran into issues trying to make my micro do too much all at once.  To free up some processor time, I pushed some workload from ISRs to hardware timers.  I modified the PwmOut/InterruptIn/Timeout drivers to provide a [hopefully] recognizable interface to using them.  
 
You can see I’ve only done hal implementations for the STM32F429/F439, and only in the configuration I am using.  I know other STM32 devices can support these features, but I’m not sure about other manufacturers.  I am also under the impression that portions of the code provided are automagically generated.  

I should also note that I did my best to not modify any mbed source. For example, I needed an mbed_app.json file to add DEVICE_TRIGGERIN, et. al, to the DEVICE_HAS macros list, but if these drivers were added to the standard mbed library they would obviously be added to targets.json.  And in each hal file, I included definitions that would need to be moved from the hal files to ST’s PeripheralNames, PeripheralPins, and common_objects files.

I tried my absolute best to follow mbed’s style guide, but it’s totally possible I missed something.

## TriggeredTimeout
When two drivers love each other very much, something special can sometimes happen.  In this case, a TriggeredTimeout is the love-child of an InterruptIn and Timeout.  When the micro sees a rising or falling edge on the specified pin, a hardware timer starts counting.  When the specified count is reached by the timer, an interrupt occurs.

### Original Code:
```cpp
InterruptIn event(p16);
Timeout timeout;

void atttimeout() {
	//delayed ISR;
}

void trigger() {
	timeout.attach(&attimeout, 5);
}

int main() {
	event.rise(&trigger);
	while(1) {
		//Loop forever
	}
}
```
### Using TriggeredTimeout:
```cpp
TriggeredTimeout triggeredTimeout(PA_15);

void delayedTrigger() {
	//delayedISR;
}

int main() {
	triggeredTimeout.attach(&delayedTrigger, 5);
	while(1) {
		//Loop forever
	}
}
```

## CounterIn
Pulse Trains are a pretty popular sensor output in which the sensor sends out a pulse for every specified amount of whatever it is sensing.  Examples of sensors that use this are Geiger counters, coloumb counters, and Hall Sensors.  Using InterruptIn is the way I initially counted these pulses, but if you have a really fast pulse train, then the MCU can end up spending a lot of time in the ISR, and it’s even possible to miss pulses.  Then I learned that some hardware timers can actually accept an external clock by which they increment their internal counter register.  The CounterIn can be configured to increment on rising edges, falling edges, or both, then you just read the counter register to know the count.  Beware: some timers are 16-bit, some are 32-bit, so where that counter register overflows will vary. 

### Original Code:
```cpp
InterruptIn event(p16);

int counter = 0;

void trigger() {
	counter++;
}

int main() {
	event.rise(&trigger);
	while(1) {
		printf("Count: %d\r\n", counter);
	}
}
```
### Using CounterIn:
```cpp
CounterIn counter(p16);

int main() {
	counter.start();
	while(1) {
		printf("Count: %d\r\n", counter.read());
	}
}
```

## EncoderIn
Wow, wasn’t CounterIn super useful in freeing up processor time!?  What if CounterIn didn’t just count up, but instead also counted down depending on some other variable?  That’s where EncoderIn comes in. So EncoderIn takes two physical inputs, one that counts edges (just like CounterIn), and one that compares levels to know whether to count up or down.  Again, just read the counter register of the hardware timer you’re using and you’ll know position of your encoder, no processor time needed!  You can even set interrupts to trigger when certain positions are met! Woohoo!

### Using EncoderIn:
```cpp
EncoderIn qei(PB_4, PB_5);

int main() {
	qei.start();
	while(1) {
		printf("Position: %d\r\n", qei.read());
	}
}
```
