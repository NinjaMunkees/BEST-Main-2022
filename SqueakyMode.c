#pragma config(Motor,  port2,           LDrive,        tmotorServoContinuousRotation, openLoop)
#pragma config(Motor,  port3,           RDrive,        tmotorServoContinuousRotation, openLoop)
#pragma config(Motor,  port4,           DriveServo,    tmotorServoStandard, openLoop)
#pragma config(Motor,  port5,           RotateServo,   tmotorServoStandard, openLoop, reversed)
#pragma config(Motor,  port6,           ArmServo,      tmotorServoStandard, openLoop, reversed)
#pragma config(UART_Usage, UART1, uartUserControl, baudRate1200, IOPins, None, None)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*

// custom baud rate set code

typedef unsigned long  uint32_t;
typedef unsigned short uint16_t;

typedef struct
{
  uint16_t SR;
  uint16_t RESERVED0;
  uint16_t DR;
  uint16_t RESERVED1;
  uint16_t BRR;
  uint16_t RESERVED2;
  uint16_t CR1;
  uint16_t RESERVED3;
  uint16_t CR2;
  uint16_t RESERVED4;
  uint16_t CR3;
  uint16_t RESERVED5;
  uint16_t GTPR;
  uint16_t RESERVED6;
} USART_TypeDef;

/* Peripheral memory map */
#define PERIPH_BASE           ((unsigned long)0x40000000)
#define APB1PERIPH_BASE       PERIPH_BASE
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define USART2                ((USART_TypeDef *) USART2_BASE)
#define USART3                ((USART_TypeDef *) USART3_BASE)

void setBaud( const TUARTs nPort, int baudRate ) {
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;

    /* pclk1 - 36MHz */
    apbclock = 36000000;

    /* Determine the integer part */
    integerdivider = ((0x19 * apbclock) / (0x04 * (baudRate)));
    tmpreg = (integerdivider / 0x64) << 0x04;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
    tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & 0x0F;

    /* Write to USART BRR */
    USART_TypeDef *uart = USART2;
    if( nPort == UART2 ) {
      uart = USART3;
    }
    uart->BRR = (uint16_t)tmpreg;
}

const int MaxStickPos = 127;
const int DeadZone = 25;             //Deadzone for joysticks
const int DriveAddition = 20;

task Chassis()
{
	//float LeftDriveStick, RightDriveStick;	//Variables for calculating
	while (true)
	{
		float LeftStick = vexRT[Ch3];
		float RightStick = vexRT[Ch2];

		//Slope calculation for smooth speed increase
		float RightDriveStick = 0;
		float LeftDriveStick = 0;

		if(fabs(LeftStick) < DeadZone)
		{
			LeftDriveStick = 0;
		}
		else if(fabs(LeftStick) > MaxStickPos)
		{
			LeftDriveStick = MaxStickPos * sgn(LeftDriveStick);
		}
		else
		{
			LeftDriveStick = (LeftStick * fabs(LeftStick)) / MaxStickPos + DriveAddition * sgn(LeftStick);
		}

		if(fabs(RightStick) < DeadZone)
		{
			RightDriveStick = 0;
		}
		else if(fabs(RightStick) > MaxStickPos)
		{
			RightDriveStick = MaxStickPos * sgn(RightDriveStick);
		}
		else
		{
			RightDriveStick = (RightStick * fabs(RightStick)) / MaxStickPos + DriveAddition * sgn(RightStick);
		}

		//Sends values to motors
		motor[LDrive] = LeftDriveStick;
		motor[RDrive] = RightDriveStick;
	}
}

const int IRCycleGoal = 20;

task IRSetup()
{
	for (i = 0; i < IRCycleGoal; i++)
	{
		sendChar( UART1, 0xF0); //Tests IR connection
		sendChar( UART1, 0xAA); //Resets squeaky
	}

	for (i = 0; i < IRCycleGoal; i++)
	{
		sendChar( UART1, 0xC3); //Sets squeaky drive speed to high
		sendChar( UART1, 0x0F); //Sets squeaky rotation speed to high
	}
}

const int HomePos = 0;

task SqueakyMode() //Control mode for squeaky
{
	//Servo Positions
	const int MaxPos = 127;         //Full forward position
	const int HalfPos = 60;         //Half-way position either way

	//Servo states
	int DriveServoState;
	int ArmServoState;
	int RotateServoState;

	//Joystick values to set servo position
	const int MidControlLimit = 100;      //Max thumbstick range

	/*
	* Checks current position of each joystick,
	* first checking if we have exceeded our dead-zone,
	* then if the joystick is in our full power
	* position or half power position,
	* then finally checking if it has returned to our
	* starting position when the joystick is released.
	*/

	while(true)
	{
		//Joystick axis mapping
		int DriveControl = vexRT[Ch2];
		int ArmControl = vexRT[Ch3];
		int RotateControl = vexRT[Ch4];

		//Section for moving Squeaky
		if(abs(DriveControl) < DeadZone)
		{
			DriveServoState = HomePos;
		}
		else if(DriveControl > DeadZone && DriveControl < MidControlLimit)
		{
			DriveServoState = HalfPos;
		}
		else if(DriveControl < -DeadZone && DriveControl > -MidControlLimit)
		{
			DriveServoState = -HalfPos;
		}
		else if(DriveControl >= MidControlLimit)
		{
			DriveServoState = MaxPos;
		}
		else if(DriveControl <= -MidControlLimit)
		{
			DriveServoState = -MaxPos;
		}

		//Section for moving Squeaky's arm
		if(abs(ArmControl) < DeadZone)
		{
			ArmServoState = HomePos;
		}
		else if(ArmControl > DeadZone && ArmControl < MidControlLimit)
		{
			ArmServoState = HalfPos;
		}
		else if(ArmControl < -DeadZone && ArmControl > -MidControlLimit)
		{
			ArmServoState = -HalfPos;
		}
		else if(ArmControl >= MidControlLimit)
		{
			ArmServoState = MaxPos;
		}
		else if(ArmControl <= -MidControlLimit)
		{
			ArmServoState = -MaxPos;
		}

		//Section for rotating Squeaky's arm
		if(abs(RotateControl) < DeadZone)
		{
			RotateServoState = HomePos;
		}
		else if(RotateControl > DeadZone && RotateControl < MidControlLimit)
		{
			RotateServoState = HalfPos;
		}
		else if(RotateControl < -DeadZone && RotateControl > -MidControlLimit)
		{
			RotateServoState = RevHalfPos;
		}
		else if(RotateControl >= MidControlLimit)
		{
			RotateServoState = MaxPos;
		}
		else if(RotateControl <= -MidControlLimit)
		{
			RotateServoState = -MaxPos;
		}

		//Sets our servo positions bsed on previous variable assignments
		motor[DriveServo] = DriveServoState;
		motor[ArmServo] = ArmServoState;
		motor[RotateServo] = RotateServoState;
	}
}

task main()
{
	startTask(Chassis);

	setBaud(UART1, 600);

	while(true)
	{
		if(vexRT[Btn5D]){
			stopTask(Chassis);
			startTask(IRSetup);
			startTask(SqueakyMode);
		}
		else if(vexRT[Btn6D]){
			stopTask(SqueakyMode);
			stopTask(IRSetup);
			startTask(Chassis);

			motor[DriveServo] = HomePos;
			motor[ArmServo] = HomePos;
			motor[RotateServo] = HomePos;
		}
	}
}