/**************************************************************
* 程 序 名         : 电子音乐播放器
* 功能描述		   : K1开始播放；
K2暂停/继续播放；
K3播放下一首；
K4播放上一首；
					 矩阵键盘选音乐；
LCD与LED显示相关信息；
串口通信用APP远程控制播放器。
* 输    入         : 按键或APP输入数字
* 输    出         : 播放相对应音乐
****************************************************************/
#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int 
#define GPIO_DIG P0
#define GPIO_KEY P1

uint code smgduan[17]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,
					       0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
//显示0~F的值
sbit LSA=P2^2;
sbit LSB=P2^3;
sbit LSC=P2^4;
sbit K1=P3^1;
sbit K2=P3^0;
sbit K3=P3^2;
sbit K4=P3^3;
sbit Beep=P1^5;       
sbit rs=P2^6;
sbit rw=P2^5;
sbit ep=P2^7;

uchar KeyValue=0;					//用来存放读取到的键值
uchar i;
uchar Song_Index=0;
uchar Tone_Index=0;   				 //音乐片段索引，音符索引
uchar *Song_Tone,*Song_Time;		 //音符指针，延时指针
uchar *discode;						 //暂存LCD需要显示的歌名
uchar dis1[]="  1.Song Bie0";		 //第一首歌名dis1~dis5为LCD第一行显示内容
uchar dis2[]="2.LiangZhiLaoHu0";//第二首歌名
uchar dis3[]="  3.Ha Ba Gou0";		 //第三首歌名
uchar dis4[]=" 4.Lan Hua Cao0";	 //第四首歌名
uchar dis5[]=" 5.Bu Dao Weng0";	 //第五首歌名
uchar dis[]=" K3:Down K4:UP0";  	 //LCD第二行显示内容
uchar disp[8];

uint code Tone_tab[]={				 //音符频率对应的T计数值
 62018,62401,62419,62895,63184,63441,63506,
 63773,63965,64137,64215,64360,64488,64603,
 64654,64751,64836,64876,64948,65012,65067,65535
 };

/*第一首音乐*/
 uchar code Song1_Tone[]={								
  11,9,11,14,12,14,12,11,11,7,8,9,8,7,8,			//第一行曲子音符
  11,9,11,14,13,12,14,11,11,7,8,9,6,7,				//第二行曲子音符
  12,14,14,13,12,13,14,12,13,14,12,12,11,10,7,8,	//第三行曲子音符
  11,9,11,14,13,12,14,11,11,8,9,10,6,7,0xff			//第四行曲子音符
 };

 uchar code Song1_Time[]={							  
 4,2,2,8,4,2,2,8,4,2,2,4,2,2,12, 					//第一行曲子节拍
  4,2,2,4,2,4,4,8,4,2,2,4,2,12, 					//第二行曲子节拍
  4,4,8,4,2,2,8,2,2,2,2,2,2,2,2,16, 				//第三行曲子节拍
  4,2,2,4,2,4,4,8,4,2,2,4,2,12,0xff					//第四行曲子节拍
 };

/*第二首音乐*/
 uchar code Song2_Tone[]={					          
 7,8,9,7,7,8,9,7,9,10,11,9,10,11, 					//第一行曲子音符
11,12,11,10,9,7,11,12,11,10,9,7,7,4,7,7,4,7,0xff	//第二行曲子音符
 };

 uchar code Song2_Time[]={
 4,4,4,4,4,4,4,4,4,4,8,4,4,8,						  //第一行曲子节拍
 2,2,2,2,4,4,2,2,2,2,4,4,4,4,8,4,4,8,0xff			  //第二行曲子节拍
 };

/*第三首音乐*/
 uchar code Song3_Tone[]={
  7,7,7,8,9,9,9,9,10,11,12,12,11,10,9,11,11,8,9,7,	//第一行曲子音符
  7,7,7,8,11,9,9,9,10,11,12,12,11,10,9,11,11,8,9,7	//第二行曲子音符
 ,0xff
};
 
 uchar code Song3_Time[]={
   2,2,2,2,4,2,2,2,2,4,2,2,2,2,4,2,2,2,2,4,		  	//第一行曲子节拍
   2,2,2,2,4,2,2,2,2,4,2,2,2,2,4,2,2,2,2,5,0xff	  	//第二行曲子节拍
 };	
 
/*第四首音乐*/
 uchar code Song4_Tone[]={	
  5,9,9,9,9,8,7,8,7,6,5,12,12,12,12,12,11,		  	  //第一行曲子音符
  2,11,11,10,9,9,12,12,11,9,8,7,8,7,6,5,9,  		  //第二行曲子音符
  2,7,7,6,5,9,8,7,6,4,12,0xff						  //第三行曲子音符
 };	
 
 uchar code Song4_Time[]={
   2,2,2,2,2,2,2,2,2,2,8,2,2,2,2,4,2, 				  //第一行曲子节拍
   2,2,2,2,8,2,2,2,2,4,2,2,2,2,2,4,2, 				  //第二行曲子节拍
   2,2,2,2,4,2,2,2,2,2,8,0xff						  //第二行曲子节拍
 };	
 
/*第五首曲子*/
 uchar code Song5_Tone[]={
   11,12,11,9,8,9,11,9,8,7,9,11,7,9,8,				  //第一行曲子音符
   11,12,11,9,8,9,11,9,8,7,8,7,8,9,7,0xff			  //第二行曲子音符
 };	
 
 uchar code Song5_Time[]={ 
 	4,4,8,4,4,8,4,4,4,4,2,2,2,2,8,					  //第一行曲子节拍
	4,4,8,4,4,8,4,4,4,4,2,2,2,2,8,0xff				  //第二行曲子节拍
 };
 /**************************************************************
* 函 数 名         : delayms
* 函数功能		   : 延时nms
* 输    入         : 需要延迟的时间
* 输    出         : 无
****************************************************************/		  
 void delayms(uint ms)		    						  {							    
 	uchar a;
	while(ms--)
 	{
		for(a=230;a>0;a--);
 	}
 } 

/**************************************************************
* 函 数 名         : KeyDown
* 函数功能		   : 检测有按键按下并读取键值
* 输    入         : 无
* 输    出         : 无
****************************************************************/
void KeyDown(void)
{

	uchar a=0;
	GPIO_KEY=0x0f;
	if(GPIO_KEY!=0x0f)						//读取按键是否按下
	{
		delayms(10);						//延时10ms进行消抖
		if(GPIO_KEY!=0x0f)					//再次检测键盘是否按下
		{									//测试列
			GPIO_KEY=0x0f;
			switch(GPIO_KEY)
			{
				case(0x07):	KeyValue=0;break;
				case(0x0b):	KeyValue=1;break;
				case(0x0d): 	KeyValue=2;break;
				case(0x0e):	KeyValue=3;break;
			}
			GPIO_KEY=0xF0; 					//测试行
			switch(GPIO_KEY)
			{
				case(0x70):	KeyValue=KeyValue;break;
				case(0xb0):	KeyValue=KeyValue+4;break;
				case(0xd0): 	KeyValue=KeyValue+8;break;
				case(0xe0):	KeyValue=KeyValue+12;break;
			}
			while((a<50)&&(GPIO_KEY!=0xf0))	//检测按键松手检测
			{
				delayms(10);
				a++;
			}
			KeyValue++;
		}
	}
	if(KeyValue>=5)
		KeyValue=5;

}

/**************************************************************
* 函 数 名         : Busy_Check
* 函数功能		   : 测试LCD忙碌状态
* 输    入         : 无
* 输    出         : 0or1
****************************************************************/
 uchar Busy_Check(void)
{
   uchar LCD_Status;
   rs=0;			         
   rw=1;
   ep=1;
   _nop_();
   _nop_();
   _nop_();
   _nop_();
   LCD_Status = P0&0x80;
   ep = 0;
   return LCD_Status;
 }

/**************************************************************
* 函 数 名         : LCD_WCMD
* 函数功能		   : 写入指令数据到LCD
* 输    入         : 指令
* 输    出         : 无
****************************************************************/
 void LCD_WCMD(uchar cmd)   
 {
 	while(Busy_Check());    	//等待LCD空闲
	rs=0;	   					//选择发送命令
	rw=0;					  	//选择写入
	ep=0;					 	//使能
	_nop_();
	_nop_();
	P0=cmd;						//P0接受到指令
	_nop_();			   		//等待时序稳定
	_nop_();
	_nop_();
	_nop_();
	ep=1;						 //写入时序
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep=0;						 //使能
 
 }

 void LCD_POS(uchar pos)   		//设置显示位置
 {
 	LCD_WCMD(pos|0x80);    		//设置LCD当前光标位置
 }						   	

/**************************************************************
* 函 数 名         : LCD_WDAT
* 函数功能		   : 写入字符到LCD
* 输    入         : 字符
* 输    出         : 无
****************************************************************/
 void LCD_WDAT(uchar dat)	
{
 	while(Busy_Check());
	rs=1; 						//选择输入数据 
	rw=0;						//选择写入
	ep=0;						//使能清零
	P0=dat;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep=1;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	ep=0;
 }

/**************************************************************
* 函 数 名         : LCD_Init
* 函数功能		   : LCD初始化设定
* 输    入         : 无
* 输    出         : 无
****************************************************************/
 void LCD_Init(void)    
 {
 	LCD_WCMD(0x38);    		//设置显示格式为16*2行显示，5*7点阵，8位
	LCD_WCMD(0x0c);	     	//开显示不显示光标
	LCD_WCMD(0x06);		 	//读写后指针加1
	LCD_WCMD(0x01);		  	//清除LCD的显示内容
}

/**************************************************************
* 函 数 名         : Int0()	interrupt 0
* 函数功能		   : 中断服务程序播放下一首音乐
* 输    入         : 按下K3
* 输    出         : 播放音乐
****************************************************************/
 void Int0()	interrupt 0    
 {
 	LCD_WCMD(0x01);	
 	delayms(100);
	if(INT0==0)				 	 //按键按下
	{
		TR0=0;					 //打开定时器0
		if (KeyValue == 5)
		{
			KeyValue=1;
		}
		else
		{
			KeyValue++;
		}
			TR0=1;
			i=0;
	}
 }

/**************************************************************
* 函 数 名         : Int1()	interrupt 2
* 函数功能		   : 中断服务程序播放上一首音乐
* 输    入         : 按下K4
* 输    出         : 播放音乐
****************************************************************/
void Int1()	interrupt 2	                
{
 	LCD_WCMD(0x01);	  				//消除重影(比如Lan Hua Cao 后面会多ao H)
   	delayms(100);
   	if(INT1==0)						//如果按键按下
	{
		TR1=0;
		if (KeyValue == 1)
		{
			KeyValue=5;
		}
		else
		{
			KeyValue--;
		}
	
		TR1=1;
		i=0;
	}  
}

void Timer0()	interrupt 1
{
	TH0=Tone_tab[Tone_Index]/256;  //重置T0定时初值
	TL0=Tone_tab[Tone_Index]%256;
	Beep=~Beep;
}

/**************************************************************
* 函 数 名         : LCD_disp
* 函数功能		   : LCD显示
* 输    入         : 字符串
* 输    出         : 当前音乐信息
****************************************************************/
void LCD_disp(void)
{
	uchar i;
	LCD_POS(0);					  	  //设置显示位为第一行第一个字符
	i=0;
	while(discode[i]!='0')		  	  //显示第一行字符串
	{
		LCD_WDAT(discode[i]);
		i++;
	}
	LCD_POS(0x40);    				  //设置显示位为第二行第一个字符
	i=0;
	while(dis[i]!='0')
	{
		LCD_WDAT(dis[i]);		 	  //显示第二行  UP&DOWN
		i++;
	}
}

/**************************************************************
* 函 数 名         : Usart() interrupt 4
* 函数功能		   : 串口中断
* 输    入         : 数组
* 输    出         : 选择的音乐
****************************************************************/
void Usart() interrupt 4
{
	uint receiveData;
	uint num[3];			 		//添加通信协议
	uint i;
	for(i=0;i<3;i++)
	{
		while(!RI);
		num[i]=SBUF;			    //接收数字
		RI = 0;						//清除接收中断标志位
	}
	if(num[0]== 0 && num[2]== 0)	//判断是否符合通信协议
	 {
		receiveData = num[1];
	 }
	KeyValue = receiveData;
	EA=1;
}	 

/**************************************************************
* 函 数 名         : int_init
* 函数功能		   : 初始化设置
* 输    入         : 无
* 输    出         : 无
****************************************************************/
void int_init(void)
{
	ET0=1;		       				//允许Timer0中断
	EX0=1;			   				//允许INT0中断
	IT0=1;			   				//INT0为边沿触发
	EX1=1;			   				//允许INT1中断
	IT1=1;			   				//INT1为边沿触发
	EA=0;			   				//关闭总中断
	TR0=0;			   				//Timer0停止

	SCON=0X50;						//设置为工作方式1
	TMOD=0X20;						//设置计数器工作方式2
	PCON=0X80;						//波特率加倍
	TH1=0XF3;						//计数器初始值设置，将波特率设置为4800
	TL1=0XF3;
	ES=1;							//打开接收中断
	TR1=1;							//打开计数器
}

/**************************************************************
* 函 数 名         : main
* 函数功能		   : 实现所有功能
* 输    入         : 无
* 输    出         : 无
****************************************************************/
void main(void)
{

	int_init();
	LCD_Init();
	Song_Tone=Song1_Tone;
	Song_Time=Song1_Time;
	discode=dis1;
	KeyValue=1;
	LSA=0; 							//给一个数码管提供位选
	LSB=0;
	LSC=0;
				  
	GPIO_DIG=smgduan[KeyValue]; 	//显示在数码管上 


	while(1)
	{
		   
        if (K1 == 0)           		//当K1按下时，总中断打开可以播放音乐
		{
			EA=1;
		}
		if (K2 == 0)				//当K2按下时继续或暂停播放音乐	
		{
			EA=~EA;
		}
		KeyDown();		   			//按键判断函数
		Song_Index=KeyValue;
		if(Song_Index==2)
		{
			LCD_WCMD(0x01);
			Song_Tone=Song2_Tone;
			Song_Time=Song2_Time;
			discode=dis2;
		}
			else if(Song_Index==3)
			{
				LCD_WCMD(0x01);
				Song_Tone=Song3_Tone;
				Song_Time=Song3_Time;
				discode=dis3;
			}
			else if(Song_Index==4)
			{
				LCD_WCMD(0x01);
				Song_Tone=Song4_Tone;
				Song_Time=Song4_Time;
				discode=dis4;
			}
			else if(Song_Index==5)
			{
				LCD_WCMD(0x01);
				Song_Tone=Song5_Tone;
				Song_Time=Song5_Time;
				discode=dis5;
			}
			else if(Song_Index==1)
			{  
				LCD_WCMD(0x01);
				Song_Tone=Song1_Tone;
				Song_Time=Song1_Time;
				discode=dis1;
			}
		
			LSA=0;						//给一个数码管提供位选
			LSB=0;
			LSC=0;
				 
			LCD_disp();					//LCD显示
			GPIO_DIG=smgduan[KeyValue]; //显示在数码管上 
			Tone_Index=Song_Tone[i];
			if(Tone_Index==0xff)
			{
				i=0;
				TR0=0;
			}  
			TR0=1;
			delayms(Song_Time[Tone_Index]*60);
			TR0=0;
			i++;		
	} 
}
