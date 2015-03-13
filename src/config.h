/* config.h -- defines for project cardrive  */

#define CMD_IDLE	0x00
#define CMD_LEFT	0x01
#define CMD_RIGHT	0x02
#define CMD_FWD		0x04
#define CMD_BWD		0x08
#define CMD_TURBO	0x10

// How the wires are connected
#define RX_LEFT		(D,2)
#define RX_RIGHT	(D,4)
#define RX_BWD		(B,6)
#define RX_FWD		(B,7)
#define RX_TURBO	(D,7)	
#define RX_ENC		(B,0)	// ICP1

#define MOT_LEFT	(D,5)	// OC0B
#define MOT_RIGHT	(D,6)	// OC0A
#define MOT_FWD		(B,2)	// OC1B
#define MOT_BWD		(B,1)	// OC1A

#define PWM1		(B,3)	// OC2A, MOSI
#define PWM2		(D,3)	// OC2B

#define LED1		(B,4)	// MISO


// horrible macro pasting stuff. 
// Just ignore this.
#define P_PORT_(_n,_i) PORT##_n
#define  P_PIN_(_n,_i) PIN##_n
#define  P_DDR_(_n,_i) DDR##_n
#define  P_BIT_(_n,_i) (1<<(_i))

#define P_PORT(_a) P_PORT_ _a
#define  P_PIN(_a) P_PIN_ _a
#define  P_DDR(_a) P_DDR_ _a
#define  P_BIT(_a) P_BIT_ _a
