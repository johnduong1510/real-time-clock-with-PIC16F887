#include <16F887.H>
#use I2C(master, SLOW, sda=PIN_C4, scl=PIN_C3)
#Fuses NOPROTECT,PUT,NOLVP,NOWDT,HS
#Use delay(CLOCK=20M)
#define SER Pin_D0
#define SCK Pin_D1
#define RCK Pin_D2
#define G Pin_D3
//-------------
#define  ADDR_WR_13B07   0xD0
#define  ADDR_RD_13B07   0xD1
#define  ADDR_MEM        0x01
//--------------
#DEFINE SETTING PIN_B0
#DEFINE UP  PIN_B1
#DEFINE DW PIN_B2
#DEFINE OK PIN_B3
#DEFINE ALARM PIN_B4
#DEFINE BUZZER PIN_A0
#DEFINE LED_ALARM PIN_E1
//
unsigned int8 GIO_CHUC,GIO_DONVI,PHUT_CHUC,PHUT_DONVI;
UNSIGNED INT8 PHUT_DS1307,GIO_DS1307;
UNSIGNED INT8 PHUT_HIENTAI,GIO_HIENTAI;
UNSIGNED INT8 PHUT_HENGIO,GIO_HENGIO;
unsigned int8 Ma7D[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
UNSIGNED INT1 TTCT=0;
UNSIGNED INT1 ALRM=0;
UNSIGNED INT8 COUNT_SETTING=0;
UNSIGNED INT8 BDN=0;
UNSIGNED INT8 L;

void DOC_THOI_GIAN_REALTIME()
{   
      //start the condition
      I2C_START();
      I2C_WRITE(ADDR_WR_13B07);
      I2C_WRITE(ADDR_MEM);   
      //RESTART THE CONDITION
      I2C_START();                
      //--get time data          
      I2C_WRITE(ADDR_RD_13B07);
      PHUT_DS1307 = I2C_READ(1);
      GIO_DS1307  = I2C_READ(0); //LAY DU LIEU XONG NGUNG (NOT ACK)
      I2C_STOP();
}


void xuat_1byte(unsigned int8 bytexuat)
{
   unsigned int8 i;
   #BIT BSER=bytexuat.7; //dn dia chi bit cho bit thu 7
   for(i=0;i<8;i++)
   {
      output_bit(SER,BSER);
      output_low(SCK) ; delay_us(10); output_high(SCK);//tac dong xung CK de dich
      bytexuat=bytexuat<<1;
   }
}

void blink(void)
{
   OUTPUT_BIT(PIN_E0,1);
   delay_ms(500);
   OUTPUT_BIT(PIN_E0,0);
   delay_ms(500);
}

void xuat_4led_7doan(unsigned int8 led_h_chuc,led_h_dv,led_m_chuc,led_m_dv)
{
   xuat_1byte(led_h_chuc);
   DELAY_US(10);
   xuat_1byte(led_h_dv);
   DELAY_US(10);
   xuat_1byte(led_m_chuc);
   DELAY_US(10);
   xuat_1byte(led_m_dv);
   DELAY_US(10);
   output_low(RCK); delay_us(10); output_high(RCK); // tac dong xung CK de luu du lieu
   delay_us(10);
   output_low(G); //cho phep xuat du lieu ra ngoai
}


VOID SET_TIME_DS1307()
{
   I2C_START();
   I2C_WRITE(ADDR_WR_13B07);
   I2C_WRITE(0X01); 
   I2C_WRITE(PHUT_HIENTAI);
   I2C_WRITE(GIO_HIENTAI);
   I2C_STOP();
}

UNSIGNED INT8 CONVERT_TO_BCD(UNSIGNED INT8 PHUT_HIENTAI,GIO_HIENTAI,UNSIGNED INT1 UPDW)
{
   IF (PHUT_HIENTAI!=0XFF) 
   {
      L=PHUT_HIENTAI&0X0F;     
      IF(UPDW==1) {IF(L>=0X0A) PHUT_HIENTAI=PHUT_HIENTAI+0X06;}
      ELSE {IF(L==0X0F) PHUT_HIENTAI=PHUT_HIENTAI-0X06;}
      IF (PHUT_HIENTAI==0X60) PHUT_HIENTAI=0X00;   
      RETURN PHUT_HIENTAI;
   }
   ELSE IF  (GIO_HIENTAI!=0XFF)
   {
      L=GIO_HIENTAI&0X0F;
      IF(UPDW==1) {IF(L>=0X0A) GIO_HIENTAI=GIO_HIENTAI+0X06;}
      ELSE {IF (L==0X0F) GIO_HIENTAI=GIO_HIENTAI-0X06;}
      IF(GIO_HIENTAI==0X24) GIO_HIENTAI=0X00;
      RETURN GIO_HIENTAI;
   }
   
}

VOID MODE_SETTING()
{  
   IF(COUNT_SETTING==1)
   {
      xuat_4led_7doan(GIO_CHUC,GIO_DONVI,PHUT_CHUC,PHUT_DONVI);
      DELAY_MS(300);
      xuat_4led_7doan(GIO_CHUC,GIO_DONVI,0XFF,0XFF);
      DELAY_MS(300);
   }
   ELSE IF(COUNT_SETTING==2)
   {
      xuat_4led_7doan(GIO_CHUC,GIO_DONVI,PHUT_CHUC,PHUT_DONVI);
      DELAY_MS(300);
      xuat_4led_7doan(0XFF,0XFF,PHUT_CHUC,PHUT_DONVI);
      DELAY_MS(300);
   }  
}

VOID MODE_UP()
{
   IF(ALRM==0) {
    IF(COUNT_SETTING==1) //SET PHUT
      {        
          PHUT_HIENTAI=PHUT_DS1307;
          GIO_HIENTAI=GIO_DS1307; 
          PHUT_HIENTAI++;
          PHUT_HIENTAI=CONVERT_TO_BCD(PHUT_HIENTAI,0XFF,1); 
          SET_TIME_DS1307();
      }
      ELSE IF(COUNT_SETTING==2) //SET GIO
      {
         PHUT_HIENTAI=PHUT_DS1307;
         GIO_HIENTAI=GIO_DS1307;
         GIO_HIENTAI++;
         GIO_HIENTAI=CONVERT_TO_BCD(0XFF,GIO_HIENTAI,1);
         SET_TIME_DS1307();
      }
   }
   ELSE 
   {       
      PHUT_HENGIO=PHUT_DS1307;
      GIO_HENGIO=GIO_DS1307;
      IF(COUNT_SETTING==1)
      {         
         PHUT_HENGIO++;
         PHUT_HENGIO=CONVERT_TO_BCD(PHUT_HENGIO,0XFF,1);
         PHUT_DONVI=MA7D[PHUT_HENGIO%16];
         PHUT_CHUC=MA7D[PHUT_HENGIO/16];
         PHUT_DS1307=PHUT_HENGIO;  
      }
      ELSE IF(COUNT_SETTING==2)
      {
         GIO_HENGIO++;
         GIO_HENGIO=CONVERT_TO_BCD(0XFF,GIO_HENGIO,1);
         GIO_DONVI=MA7D[GIO_HENGIO%16];
         GIO_CHUC=MA7D[GIO_HENGIO/16];
         GIO_DS1307=GIO_HENGIO;
      }
   }
}

VOID MODE_DOWN()
{
   IF(ALRM==0){
      IF(COUNT_SETTING==1) //SET PHUT
      {
         PHUT_HIENTAI=PHUT_DS1307;
         GIO_HIENTAI=GIO_DS1307;
         PHUT_HIENTAI--;
         IF(PHUT_HIENTAI==0XFF) PHUT_HIENTAI=0X59;
         PHUT_HIENTAI=CONVERT_TO_BCD(PHUT_HIENTAI,0XFF,0);        
         SET_TIME_DS1307();
      }
      ELSE IF(COUNT_SETTING==2) //SET GIO
      {
         GIO_HIENTAI=GIO_DS1307;
         PHUT_HIENTAI=PHUT_DS1307;
         GIO_HIENTAI--;
         IF(GIO_HIENTAI==0XFF) GIO_HIENTAI=0X23;
         GIO_HIENTAI=CONVERT_TO_BCD(0XFF,GIO_HIENTAI,0);
         SET_TIME_DS1307();
      }
   }
   ELSE
   {
      PHUT_HENGIO=PHUT_DS1307;
      GIO_HENGIO=GIO_DS1307;
      IF(COUNT_SETTING==1)
      {         
         PHUT_HENGIO--;
         IF(PHUT_HENGIO==0XFF) PHUT_HENGIO=0X59;
         PHUT_HENGIO=CONVERT_TO_BCD(PHUT_HENGIO,0XFF,0);
         PHUT_DONVI=MA7D[PHUT_HENGIO%16];
         PHUT_CHUC=MA7D[PHUT_HENGIO/16];
         PHUT_DS1307=PHUT_HENGIO;   
      }
      ELSE IF(COUNT_SETTING==2)
      {
         GIO_HENGIO--;
         IF(GIO_HENGIO==0XFF) GIO_HENGIO=0X23;
         GIO_HENGIO=CONVERT_TO_BCD(0XFF,GIO_HENGIO,0);
         GIO_DONVI=MA7D[GIO_HENGIO%16];
         GIO_CHUC=MA7D[GIO_HENGIO/16];
         GIO_DS1307=GIO_HENGIO;
      }
   }
}

VOID RING_ALARM()
{
   IF ( (PHUT_HENGIO==PHUT_DS1307) &(GIO_HENGIO==GIO_DS1307)&(ALRM==1) )    
         {
            OUTPUT_BIT(BUZZER,1);
            OUTPUT_BIT(LED_ALARM,0);
            DELAY_MS(50);
            OUTPUT_BIT(BUZZER,0);
            OUTPUT_BIT(LED_ALARM,1);
            DELAY_MS(50);
            OUTPUT_BIT(BUZZER,1);
            OUTPUT_BIT(LED_ALARM,0);
            DELAY_MS(100);
            OUTPUT_BIT(BUZZER,0);
            OUTPUT_BIT(LED_ALARM,1);
            DELAY_MS(100);      
            OUTPUT_BIT(LED_ALARM,0);
         }
}


#INT_TIMER1
VOID CONVERT_TIME_FROM_DS1307() //LAY DU LIEU LAY TU DS1307 CHUYEN SANG DU LIEU XUAT RA LED7DOAN DUOC
{
   BDN++;
   IF(BDN==10)//1 GIAY
   {
      DOC_THOI_GIAN_REALTIME();
      PHUT_DONVI=MA7D[PHUT_DS1307%16];
      PHUT_CHUC=MA7D[PHUT_DS1307/16];
      GIO_DONVI=MA7D[GIO_DS1307%16];
      GIO_CHUC=MA7D[GIO_DS1307/16];
      set_timer1(-62500);
      BDN=0;
   }
}

#INT_RB
VOID XULY_NUTNHAN()
{
   IF(!INPUT(SETTING))
   {
      //xu ly chong doi
      DELAY_MS(20);
      if(!input(setting)){
      WHILE(!INPUT(SETTING));
      //CAU LENH
      //
      TTCT=1;
      COUNT_SETTING++;
      IF(COUNT_SETTING>2) COUNT_SETTING=1;
   }
   }
   ELSE IF(!INPUT(UP))
   {
      DELAY_MS(20);
      IF(!INPUT(UP))
      {
         WHILE(!INPUT(UP));
         //CAU LENH
         //
        MODE_UP();
      }
   }
   ELSE IF(!INPUT(DW))
   {
     DELAY_MS(20);
      IF(!INPUT(DW))
      {
         WHILE(!INPUT(DW));
         //CAU LENH
         //
         MODE_DOWN();
      } 
   }
    ELSE IF(!INPUT(OK))
   {
     DELAY_MS(20);
      IF(!INPUT(OK))
      {
         WHILE(!INPUT(OK));
         //CAU LENH
         //
         xuat_4led_7doan(GIO_CHUC,GIO_DONVI,PHUT_CHUC,PHUT_DONVI);
         IF(TTCT==1) {TTCT=0; COUNT_SETTING=0;}
         SETUP_TIMER_1(T1_internal | T1_div_by_4);
         set_timer1(-62500);
         IF(ALRM==1) OUTPUT_BIT(LED_ALARM,1);
         DOC_THOI_GIAN_REALTIME();
      } 
   }
    ELSE IF(!INPUT(ALARM))
   {
     DELAY_MS(20);
      IF(!INPUT(ALARM))
      {
         WHILE(!INPUT(ALARM));
         //CAU LENH
         //
         TTCT=1;
         ALRM=1;
         COUNT_SETTING++;
         IF(COUNT_SETTING>2) COUNT_SETTING=1;
         SETUP_TIMER_1(T1_DISABLED);
      } 
   }
}

void main()
{
   set_tris_D(0X00);
   SET_TRIS_A(0X00);
   set_tris_E(0x00);
   OUTPUT_BIT(BUZZER,0);
   OUTPUT_BIT(LED_ALARM,0);
   enable_interrupts(global);
   enable_interrupts(int_timer1);
   setup_timer_1(T1_internal | T1_div_by_8);
   ENABLE_INTERRUPTS(INT_RB);
   set_timer1(-62500);
   WHILE(TRUE)
   {     
      IF(TTCT==1) MODE_SETTING();
      ELSE
      {
         xuat_4led_7doan(GIO_CHUC,GIO_DONVI,PHUT_CHUC,PHUT_DONVI); BLINK(); 
         RING_ALARM();     
      }
      
   }
}

