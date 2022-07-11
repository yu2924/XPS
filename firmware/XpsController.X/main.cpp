//
//  main.cpp
//  XpsController
//
//  created by yu2924 on 2021-11-11
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#define F_CPU 8000000L
#include <util/delay.h>
#include <util/atomic.h>

inline uint8_t getbit(uint8_t v, uint8_t ibit) { return (v >> ibit) & 0x01; }
inline void setbit(volatile uint8_t& vref, uint8_t ibit, uint8_t v) { vref = (vref & ~(1 << ibit)) | (v << ibit); }
inline void modifybyte(volatile uint8_t& vref, uint8_t m, uint8_t v) { vref = (vref & ~m) | (v & m); }
inline void atomicsetbit(volatile uint8_t& vref, uint8_t ibit, uint8_t v) { ATOMIC_BLOCK(ATOMIC_FORCEON) { setbit(vref, ibit, v); } }
inline void atomicmodifybyte(volatile uint8_t& vref, uint8_t m, uint8_t v) { ATOMIC_BLOCK(ATOMIC_FORCEON) { modifybyte(vref, m, v); } }

template<class T> T tmin(T a, T b) { return a < b ? a : b; }
template<class T> T tmax(T a, T b) { return a < b ? b : a; }
template<class T> T tlimit(T l, T h, T v) { return tmax(l, tmin(h, v)); }
template<class T> T tabs(T v) { return (0 <= v) ? v : -v; }

// --------------------------------------------------------------------------------
// building blocks

#define USE_CR 1
#if USE_CR
//
// CR high pass filter
// tc=1/(2*pi*fc)
// y=x-s; s=s+(x-s)*(2*pi*fc);
//
struct AcCoupler
{
	int32_t s, y;
	AcCoupler() : s(0), y(0)
	{
	}
	int16_t processNext(int16_t xi)
	{
		int32_t x = (int32_t)xi * 256;
		y = x - s;
		s = s + (x - s) / 512; // fc=1/(512*2*pi)=3.1085e-4=5.978Hz@fs=19.23kHz
		return (int16_t)(y / 256);
	}
};
#else // USE_CR
//
// Transposed Direct-Form II
//
//             b0
// x[n] ----+--|>--|+|------+----y[n]
//          |       | s1    |
//          |     |z-1|     |
//          |  b1   |  -a1  |
//          +--|>--|+|--<|--+
//
//        b0 + b1*z^-1   b0*z + b1
// H(z) = ------------ = ---------
//        1 + a1*z~-1     z + a1
//
// y = b0*x + s1; s1 = b1*x - a1*y;
//
// applying DC blocker configuration { a1 = -0.995, b0 = 1, b1 = -1 }:
// y=x+s1; s1=-x+0.995*y;
//
struct AcCoupler
{
	int32_t s1;
	AcCoupler() : s1(0)
	{
	}
	int16_t processNext(int16_t xi)
	{
		int32_t x = (int32_t)xi * 256;
		int32_t y = x + s1;
		s1 = -x + (y * 255) / 256;
		return (int16_t)(y / 256);
	}
};
#endif // USE_CR

struct PeakDetector
{
	int16_t threshold;
	uint16_t holdTime;
	uint16_t count;
	PeakDetector(int16_t vth, uint16_t thold)
		: threshold(vth)
		, holdTime(thold)
		, count(0)
	{
	}
	bool processNext(int16_t v)
	{
		v = tabs<int16_t>(v);
		if(threshold <= v) count = holdTime;
		else if(count) -- count;
		return 0 < count;
	}
};

struct Debouncer
{
	uint8_t bits; // the bit queue
	enum Edge { None, Rising, Falling } edge;
	bool high;
	Debouncer()
		: bits(0)
		, edge(None)
		, high(false)
	{
	}
	void reset(bool h)
	{
		edge = None;
		bits = h ? 0b11 : 0b00;
		high = h;
	}
	Edge processNext(bool h)
	{
		// assumes that called at 1~10ms intervals
		bits <<= 1;
		bits |= h ? 1 : 0;
		bits &= 0b11;
		edge = None;
		if(bits == 0b11)
		{
			if(!high) { high = true; edge = Rising; }
		}
		else if(bits == 0b00)
		{
			if(high) { high = false; edge = Falling; }
		}
		return edge;
	}
	bool isLow() const { return !high; }
	bool isHigh() const { return high; }
};

// --------------------------------------------------------------------------------
// the controller
// NOTE: the loop interval is generated from the free running ADC

// abbreviations
// * AL: Audio Level
// * PS: Power Supply
// * TG: Transmission Gate
// * XP: eXternal Power
// * HV: High Voltage
// * NL: Nominal Level
// * PL: Peak Level

// suffixes
// * M: Monitor input
// * S: Switch input
// * C: Control output
// * D: Display output
// * O: Output

// pin assignment
// * PA0: ADC ALM
// * PA1: DI ~XPM
// * PA2: AIN1 PSM
// * PA3: DI ~TGS
// * PA4: DI ~XPS
// * PA5: DI ~HVS
// * PA6: DO NLD
// * PA7: DO PLD
// * PB0: DO TGC
// * PB1: DO XPC
// * PB2: DO HVC
// * PB3: ~RESET

// inputs

#define PORT_XPM PORTA
#define PIN_XPM PINA
#define DDR_XPM DDRA
#define BIT_XPM PORTA1

#define PORT_TGS PORTA
#define PIN_TGS PINA
#define DDR_TGS DDRA
#define BIT_TGS PORTA3

#define PORT_XPS PORTA
#define PIN_XPS PINA
#define DDR_XPS DDRA
#define BIT_XPS PORTA4

#define PORT_HVS PORTA
#define PIN_HVS PINA
#define DDR_HVS DDRA
#define BIT_HVS PORTA5

// outputs

#define PORT_NLD PORTA
#define PIN_NLD PINA
#define DDR_NLD DDRA
#define BIT_NLD PORTA6

#define PORT_PLD PORTA
#define PIN_PLD PINA
#define DDR_PLD DDRA
#define BIT_PLD PORTA7

#define PORT_TGC PORTB
#define PIN_TGC PINB
#define DDR_TGC DDRB
#define BIT_TGC PORTB0

#define PORT_XPC PORTB
#define PIN_XPC PINB
#define DDR_XPC DDRB
#define BIT_XPC PORTB1

#define PORT_HVC PORTB
//#define PIN_HVC PINB
#define DDR_HVC DDRB
#define BIT_HVC PORTB2

struct Controller
{
	static const int WAIT_TGC = 10;
	static const int WAIT_XPC = 200;
	AcCoupler acCoupler;
	PeakDetector peakDetectorP;
	PeakDetector peakDetectorN;
	Debouncer tgsDebouncer, xpsDebouncer, hvsDebouncer; // pressed:low, released:high
	Debouncer psmDebouncer; // pass:low, fail:high
	uint8_t loopCount;
	uint8_t xpmWaitCount;
	bool tgc, xpc, hvc;
	Controller()
		: peakDetectorP(32000, 385) // threshold=-0.21dBFS, holdtime=20.02ms@fs=19.23kHz
		, peakDetectorN(128, 1924) // threshold=-48.2dBFS, holdtime=100.1ms@fs=19.23kHz
		, loopCount(0)
		, xpmWaitCount(0)
		, tgc(false)
		, xpc(false)
		, hvc(false)
	{
	}
	void configure()
	{
		// --------------------
		// clock prescaler
		// * fosc=8MHz, fclk=fosc/1=8MHz
		// CLKPR = (1<<CLKPCE); // enable prescaler change
		// CLKPR = (0b0000<<CLKPS0); // divisor=1
		clock_prescale_set(clock_div_1);
		// --------------------
		// peripherals, sleepmode
		// PRR = (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI); // disable usi, timer0, timer1
		// MCUCR = (0b01<<SM0); // sleepmode=ADC
		power_usi_disable();
		power_timer0_disable();
		power_timer1_disable();
		set_sleep_mode(SLEEP_MODE_ADC);
		// --------------------
		// port configuration
		DDRA  = (1<<BIT_PLD) | (1<<BIT_NLD);
		PORTA = (1<<BIT_HVS) | (1<<BIT_XPS) | (1<<BIT_TGS) | (1<<BIT_XPM);
		DDRB  = (1<<BIT_HVC) | (1<<BIT_XPC) | (1<<BIT_TGC);
		PORTB = 0;
		// --------------------
		// analog comparator, adc
		// * fclkadc=fclk/32=250kHz, fs=fclkadc/13=19.23kHz
		ACSR = (1<<ACBG); // analog comparator bandgap(1.1V) select
		ADMUX = (0b00<<REFS0) | (0b000000<<MUX0); // vref=vcc, ADC0 singleend
		ADCSRA = (1<<ADEN) | (1<<ADATE) | (1<<ADIE) | (0b101<<ADPS0); // adc enable, adc auto trigger enable, adc interrupt enable, adc prescaler=1/32
		ADCSRB = (0<<BIN) | (0<<ACME) | (1<<ADLAR) | (0b000<<ADTS0); // adc unipolar, disable analog comparator mux (use AIN1), adc left adjust, adc free running
		// where is the definition of AIN1D?
#if !defined AIN1D
#define AIN1D ADC2D
#endif
		DIDR0 = (1<<ADC0D) | (1<<AIN1D); // disable digital input on ADC0(PA0), AIN1(PA2)
	}
	void start()
	{
		// --------------------
		// startup blinking
		for(int i = 0; i < 4; ++ i)
		{
			setbit(PORT_PLD, BIT_PLD, 0);
			setbit(PORT_NLD, BIT_NLD, 1);
			_delay_ms(250);
			setbit(PORT_PLD, BIT_PLD, 1);
			setbit(PORT_NLD, BIT_NLD, 0);
			_delay_ms(250);
		}
		setbit(PORT_PLD, BIT_PLD, 0);
		setbit(PORT_NLD, BIT_NLD, 0);
		// --------------------
		// initialization
		tgsDebouncer.reset(PIN_TGS & (1<<BIT_TGS)); psmDebouncer.reset(ACSR & (1<<ACO)); tgc = tgsDebouncer.isLow() && psmDebouncer.isLow();
		xpsDebouncer.reset(PIN_XPS & (1<<BIT_XPS)); xpc = xpsDebouncer.isLow();
		hvsDebouncer.reset(PIN_HVS & (1<<BIT_HVS)); hvc = hvsDebouncer.isLow();
		setbit(PORT_HVC, BIT_HVC, hvc ? 1 : 0);
		setbit(PORT_XPC, BIT_XPC, xpc ? 1 : 0);
		_delay_ms(WAIT_XPC);
		setbit(PORT_TGC, BIT_TGC, tgc ? 1 : 0);
		_delay_ms(WAIT_TGC);
		// trigger the first conversion with the ADC initialization sequence
		ADCSRA |= (1<<ADSC);
	}
	void handleAdcIsr()
	{
		int16_t v = acCoupler.processNext((int16_t)ADCW - 32768);
		setbit(PORT_PLD, BIT_PLD, peakDetectorP.processNext(v) ? 1 : 0);
		setbit(PORT_NLD, BIT_NLD, peakDetectorN.processNext(v) ? 1 : 0);
	}
	void handleMainLoop()
	{
		++ loopCount;
		if(96 <= loopCount) // t=4.992ms@fs=19.23kHz
		{
			loopCount = 0;
			doperiodicjob();
		}
	}
	void settgc(bool v)
	{
		if(tgc == v) return;
		tgc = v;
		atomicsetbit(PORT_TGC, BIT_TGC, tgc ? 1 : 0);
		_delay_ms(WAIT_TGC);
	}
	void setxpc(bool v)
	{
		if(xpc == v) return;
		bool tgcsave = tgc;
		settgc(false);
		xpc = v;
		atomicsetbit(PORT_XPC, BIT_XPC, xpc ? 1 : 0);
		_delay_ms(WAIT_XPC);
		settgc(tgcsave);
		xpmWaitCount = 0;
	}
	void sethvc(bool v)
	{
		if(hvc == v) return;
		bool tgcsave = tgc;
		settgc(false);
		bool xpcsave = xpc;
		setxpc(false);
		hvc = v;
		atomicsetbit(PORT_HVC, BIT_HVC, hvc ? 1 : 0);
		setxpc(xpcsave);
		settgc(tgcsave);
	}
	void doperiodicjob()
	{
		if(tgsDebouncer.processNext(PIN_TGS & (1<<BIT_TGS)) || psmDebouncer.processNext(ACSR & (1<<ACO))) settgc(tgsDebouncer.isLow() && psmDebouncer.isLow());
		if(xpsDebouncer.processNext(PIN_XPS & (1<<BIT_XPS))) setxpc(xpsDebouncer.isLow());
		if(hvsDebouncer.processNext(PIN_HVS & (1<<BIT_HVS))) sethvc(hvsDebouncer.isLow());
		if(xpc)
		{
			if(!(PIN_XPM & (1<<BIT_XPM))) xpmWaitCount = 0;
			else if(xpmWaitCount < 100) ++ xpmWaitCount; // t=499.2ms@interval=4.992ms
			else setxpc(false);
		}
	}
} gController;

// --------------------------------------------------------------------------------
// entry points

int main(void)
{
	gController.configure();
	gController.start();
	sei();
	sleep_mode();
	while(1)
	{
		// woken up by the ADC interrupt
		gController.handleMainLoop();
		sleep_mode();
	}
	return 0;
}

ISR(ADC_vect)
{
	gController.handleAdcIsr();
}
