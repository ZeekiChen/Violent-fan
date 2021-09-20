#include "STC8Gxx.h"
#include "intrins.h"

#define	EN3V3		P32
#define	KEY1		P33
#define	LED1		P54
#define PWM_OUT	P55
#define	PWM_T		40000

unsigned int T_h = 2000;
unsigned int T_l = 38000;
unsigned char mode = 0;
unsigned int inactive_cnt = 0;

void setPulsewidth(float pw)
{
	T_h = (pw - 1.0) * 2000 + 2000;
	T_l = PWM_T - T_h;
	T_h = 65536 - T_h;
	T_l = 65536 - T_l;
}

void Timer0Init(void)		//20����@24.000MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = T_l;
	TH0 = T_l >> 8;
	TF0 = 0;				//���TF0��־
	TR0 = 1;				//��ʱ��0��ʼ��ʱ
	ET0 = 1;        //ʹ�ܶ�ʱ���ж�
}
void TM0_Isr() interrupt 1
{	
	static unsigned char k = 0;
	if(k == 0)
	{
		k = 1;
		TL0 = T_h;
		TH0 = T_h >> 8;
		PWM_OUT = 0;
	}
	else
	{
		k = 0;
		TL0 = T_l;
		TH0 = T_l >> 8;
		PWM_OUT = 1;
	}
}
void Timer1Init(void)		//10����@24.000MHz
{
	AUXR &= 0xBF;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0x0F;		//���ö�ʱ��ģʽ
	TL1 = 0xE0;		//���ö�ʱ��ʼֵ
	TH1 = 0xB1;		//���ö�ʱ��ʼֵ
	TF1 = 0;		//���TF1��־
	TR1 = 1;		//��ʱ��1��ʼ��ʱ
	ET1 = 1;
}
void TM1_Isr() interrupt 3
{
	if(inactive_cnt++ == 30000)
	{
		KEY1 = 0;
		mode = 3;
	}
}
void INT4_Isr() interrupt 16
{
	IAP_CONTR = 0x60;
}
void INT1_Isr() interrupt 2
{
}
void Delay100ms()		//@24.000MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 10;
	j = 31;
	k = 147;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
void main()
{
	P3M0 = 0x00;
	P3M1 = 0x00;
	P5M0 = 0x10;
	P5M1 = 0x00;
	EN3V3 = 1;
	KEY1 = 1;
	LED1 = 0;
	PWM_OUT = 0;
	IT1 = 1;                                    //INT1�½����ж�
  EX1 = 1;                                    //ʹ��INT1�ж�
	INTCLKO = 0x40;                             //ʹ��INT4�ж�
	Timer0Init();
	Timer1Init();
	setPulsewidth(1.0);
	mode = 0;
  EA = 1;
	while(1)
	{
		Delay100ms();
		if(KEY1 == 0)
		{
			mode++;
			inactive_cnt = 0;
			if(mode == 1)
				setPulsewidth(1.3);
			else if(mode == 2)
				setPulsewidth(1.6);
			else if(mode == 3)
				setPulsewidth(1.9);
			else
			{
				setPulsewidth(1.0);
				EN3V3 = 0;
				LED1 = 1;
				KEY1 = 1;
				PCON = 0x02;
				_nop_();
				IAP_CONTR = 0x60;
			}
			while(KEY1 == 0);
		}
		
	}
}