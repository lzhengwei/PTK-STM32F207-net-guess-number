// -------------------------------------------------------------------------
//  Title       : Example Code
//              :
//  Library     :
//              :
//  Developer   : MICROTIME MDS group
//              :
//  Purpose     : uC/TCP-IP middleware demostration : (Pinging)
//              : This demo code reply the ping request from other nodes
//              :
//  Limitation  : 1. Support ststic IP only
//              :
//  Note        : 1. The default IP is 10.0.0.232 (change it in your need)
//              : 2. The default gateway is set to 10.0.0.1
//              :
// -------------------------------------------------------------------------
//  (c) Copyright 2012; Microtime Computer Inc.; Taiwan (R.O.C.)
//
//  All rights reserved.  Protected by international copyright laws.
//  Knowledge of the source code may NOT be used to develop similar product
//  Your honesty is greatly appreciated.
// -------------------------------------------------------------------------
//  Modification History :
// -------------------------------------------------------------------------
//  Version | Mdy Date : | Descriptions
//          | mm/dd/yyyy |
//  V1.00   | 08/10/2012 | First release
// -------------------------------------------------------------------------



/***************************************************************************
Include Files
***************************************************************************/
#include <includes.h>
#include <string.h>
#include <stdlib.h>
#include "lcd\module_gui.h"
#include "audio\module_audio.h"
#include "audio\audio_alc5622.h"
#include "rtc\module_rtc.h"
#include "touch\module_touch.h"
#include "uart\module_uart.h"
#include "buzzer\module_buzzer.h"
#include "app_platform_routing.h"
#include "platform\platform_board.h"

/***************************************************************************
Constant Define
***************************************************************************/
#ifdef DEBUG_MSG
#define APP_TRACE_INFO(...) printf(__VA_ARGS__)
#else
#define APP_TRACE_INFO(...)
#endif

#define  APP_TASK_STOP();                             { while (DEF_ON) { \
                                                            ;            \
                                                        }                \
                                                      }


#define  APP_TEST_FAULT(err_var, err_code)            { if ((err_var) != (err_code)) {   \
                                                            APP_TASK_STOP();             \
                                                        }                                \
                                                      }


#define  RX_BUF_SIZE         15
/***************************************************************************
Function Prototype (External)
***************************************************************************/



/***************************************************************************
Function Prototype (Local)
***************************************************************************/
static  void  App_TaskStart         (void *p_arg);
static  void  App_TaskUDPreceive    (void *p_arg);
static  void UDPSendTask (void *p_arg);
static  void App_Task_keynum(void *p_arg);
static  void App_Task_Result(void *p_arg);

static void clearnumber();
static void socketcreat(void);
static void gameview();
static void answertake();
//static  void  UDPSendTask           (void *p_arg);
/***************************************************************************
Variable Define (External)
***************************************************************************/



/***************************************************************************
Variable Define (Global)
***************************************************************************/



/***************************************************************************
Variable Define (Local)
***************************************************************************/
static  OS_STK         App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK         App_TaskUDPreceiveStk[APP_TASK_START_STK_SIZE];
static  OS_STK         App_TaskUDPSendStk[APP_TASK_START_STK_SIZE];
static  OS_STK         App_Task_keynum_Stk[APP_TASK_START_STK_SIZE];
static  OS_STK         App_Task_Result_Stk[APP_TASK_START_STK_SIZE];
static  void        KEY0_IRQHandler      (void);
static  void        KEY0_Config          (void);
static  void        KEY1_IRQHandler      (void);
static  void        KEY1_Config          (void);

static bool click=false;
 
static int i=0;
static NET_SOCK_ID        sock;
static     NET_SOCK_ADDR_IP   server_sock_addr_ip;
static     NET_SOCK_ADDR_LEN  server_sock_addr_ip_size;
static     NET_SOCK_ADDR_IP   client_sock_addr_ip;
static    NET_SOCK_ADDR_LEN  client_sock_addr_ip_size;
//--------------------------1234-------------------------------------------
static char number[5]={' ',' ',' ',' '};static char numset[]="0123456789";
static char answer[5]={' ',' ',' ',' '},bans[5]="    ",bnum[5]="     ";
static char review[][1000];
static int numchioce=10,turn=1,printline=16,resultA=0,resultB=0,reviewindex=0,reprint=1,reprintmax=0;;
static bool touchgood=false,answerin=false,gamestart=false,clear=false,
            roundgo=false,bansok=false,bnumok=false,mynumok=false,
            showreview,numchange=false;
//---------------------------OOOOXXXX---------------------------------------
static int map[10],siteo=0,sitex=0,first=5,sitesend=3;
static char state=' ';
static char* o;static char* xx;

//static char *s,send;
static char* sendox;
static bool printx,printo;static bool conect=false,ready=false,cantouch=false;
//----------------------------------------------------------------------------
OS_EVENT    *Sendflag,*resultflag;
//--------------------------------------------------------------------------
#define UDP_SERVER_IP_ADDR      "140.125.33.10"
#define UDP_SERVER_PORT         5000
NET_SOCK_RTN_CODE  rx_size;
CPU_CHAR           rx_buf[RX_BUF_SIZE];

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
*********************************************************************************************************
*/
int
main (void)
{
        CPU_INT08U  os_err;


        BSP_IntDisAll();                                            /* Disable all ints until we are ready to accept them.  */

        OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */

        os_err = OSTaskCreateExt((void (*)(void *)) App_TaskStart,  /* Create the start task.                               */
                                 (void          * ) 0,
                                 (OS_STK        * )&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
                                 (INT8U           ) APP_TASK_START_PRIO,
                                 (INT16U          ) APP_TASK_START_PRIO,
                                 (OS_STK        * )&App_TaskStartStk[0],
                                 (INT32U          ) APP_TASK_START_STK_SIZE,
                                 (void          * ) 0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

#if (OS_TASK_NAME_EN > 0)
        OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Task", &os_err);
#endif

        OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */

        return (0);
}



/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument(s) : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void
App_TaskStart (void *p_arg)
{
        CPU_INT32U  cnts;
        NET_ERR     net_err;
        CPU_INT08U  os_err;
        
        (void)p_arg;

        BSP_Init();                                                 /* Initialize BSP functions.                            */
        CPU_Init();                                                 /* Init CPU name & int. dis. time measuring fncts.      */
        cnts = BSP_CPU_ClkFreq() / (CPU_INT32U)OS_TICKS_PER_SEC;
        OS_CPU_SysTickInit(cnts);  
        /* Initialize the SysTick.                              */
        

        
        
        platform_board_init(SystemCoreClock);
        Sendflag = OSSemCreate(0);    
        resultflag = OSSemCreate(0); 
        KEY0_Config();
        KEY1_Config();
     
         module_7segment_init();

#if (OS_TASK_STAT_EN > 0)
        OSStatInit();                                             /*  Determine CPU capacity.          */                    
#endif 

#if ((APP_PROBE_COM_EN == DEF_ENABLED) || \
     (APP_OS_PROBE_EN  == DEF_ENABLED))
        //App_InitProbe();
#endif
module_7segment_put_number(APP_SEG1,1);
        //App_EventCreate();                                          /* Create application events.                           */
        //App_TaskCreate();                                           /* Create application tasks.                            */

     
        
        module_gui_init();
        module_gui_set_color (GUI_BLACK,GUI_WHITE);
        module_gui_clear(GUI_WHITE);
        
         //module_buzzer_init();
        
        Mem_Init();                                                 /* Initialize memory management module.                 */
        App_TCPIP_Init(&net_err);                                   /* Initialize uC/TCP-IP and associated applications.    */
        //module_gui_text_string ("Hello this is Demo for LCD");
        if (net_err == NET_IF_ERR_NONE){
            module_led_on(APP_LED1);
             module_gui_text_string ("NET_IF_ERR_NONE");
          //  APP_TRACE_INFO(("    Ethernet: OK then Acitve TCP/IP.\n"));
        }
          socketcreat();
          OSTaskCreateExt((void (*)(void *)) App_TaskUDPreceive,  
                                 (void          * ) 0,
                                 (OS_STK        * )&App_TaskUDPreceiveStk[128 - 1],
                                 (INT8U           ) 10,
                                 (INT16U          ) 10,
                                 (OS_STK        * )&App_TaskUDPreceiveStk[0],
                                 (INT32U          ) 128,
                                 (void          * )0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
        
        OSTaskCreateExt((void (*)(void *)) UDPSendTask,  
                                 (void          * ) 0,
                                 (OS_STK        * )&App_TaskUDPSendStk[128 - 1],
                                 (INT8U           ) 11,
                                 (INT16U          ) 11,
                                 (OS_STK        * )&App_TaskUDPSendStk[0],
                                 (INT32U          ) 128,
                                 (void          * )0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
                       
                        OSTaskCreateExt((void (*)(void *))  App_Task_keynum  ,                       
                                 (void          * ) 0,
                                 (OS_STK        * )&App_Task_keynum_Stk[128 - 1],
                                 (INT8U           ) 12,
                                 (INT16U          ) 12,
                                 (OS_STK        * )&App_Task_keynum_Stk[0],
                                 (INT32U          ) 128,
                                 (void          * )0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));      

                           OSTaskCreateExt((void (*)(void *))  App_Task_Result ,                       
                                 (void          * ) 0,
                                 (OS_STK        * )&App_Task_Result_Stk[128 - 1],
                                 (INT8U           ) 15,
                                 (INT16U          ) 15,
                                 (OS_STK        * )&App_Task_Result_Stk[0],
                                 (INT32U          ) 128,
                                 (void          * )0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
        module_led_toggle(APP_LED0);  
       /*while (DEF_TRUE) {                                          
            
           // TestUDPServer();
            OSTimeDlyHMSM(0, 0, 0, 500);
        }*/
}
//----------------------------UDP Send--------------------------------------------
static void UDPSendTask(void *p_arg)
{
 /* NET_SOCK_ID           socksend;
  NET_IP_ADDR           send_ip_addr;
  NET_SOCK_ADDR_IP      send_sock_addr_ip;
  NET_SOCK_ADDR_LEN     send_sock_addr_ip_size;
  CPU_CHAR              *pbuf;
  CPU_INT16S            buf_len;
  
  NET_ERR               err;
  uint8_t               err1;
        
  pbuf = "send to sever";
  
  buf_len = Str_Len(pbuf);
  
  socksend = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                       NET_SOCK_TYPE_DATAGRAM,
                       NET_SOCK_PROTOCOL_UDP,
                       &err);
  
  
  
  send_ip_addr = NetASCII_Str_to_IP(UDP_SERVER_IP_ADDR, &err);
  
  send_sock_addr_ip_size = sizeof(send_sock_addr_ip);
  Mem_Clr((void     *) &send_sock_addr_ip,
          (CPU_SIZE_T) send_sock_addr_ip_size);
  send_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
  send_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(send_ip_addr);
  send_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);*/
//module_gui_text_string ("send task");
  uint8_t               err1;
  NET_ERR               err;
  NET_SOCK_ID           socksend;
  NET_SOCK_RTN_CODE     tx_size;
  NET_SOCK_ADDR_IP      send_sock_addr_ip;
  NET_SOCK_ADDR_LEN     send_sock_addr_ip_size;
  NET_IP_ADDR           send_ip_addr;
  char *pbuf= "12345678910";
  CPU_INT16S            buf_len;
  
  
   send_ip_addr = NetASCII_Str_to_IP(UDP_SERVER_IP_ADDR, &err);
    send_sock_addr_ip_size = sizeof(send_sock_addr_ip);

  
  
   
   Mem_Clr((void     *) &send_sock_addr_ip,
          (CPU_SIZE_T) send_sock_addr_ip_size);
  send_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
  send_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(send_ip_addr);
  send_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);
  while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
           
          OSSemPend(Sendflag,100,&err1);
            

                       if(err1==OS_ERR_NONE)
       {
            //module_gui_text_printf_line (4,"send %s",o);
            

           // module_gui_text_printf_line (4,"send");
            module_7segment_put_number(APP_SEG1,3);
           if(mynumok)
           {
             pbuf=number;
           }
            else if(answerin)
            {
              pbuf=answer;
            }
            else if(ready)
            {
              pbuf="ready";ready=false;
              module_gui_text_printf_line(14,"key in four number as number");
              numchioce=0;
              answerin=false;
            }
            else if(conect )
            {
               
              pbuf="second";
              conect=false;
            }
            buf_len = Str_Len(pbuf);
              
            tx_size = NetSock_TxDataTo((NET_SOCK_ID ) sock,
                             (void *) pbuf,
                             (CPU_INT16S) buf_len+1,
                             (CPU_INT16S) NET_SOCK_FLAG_NONE,
                             (NET_SOCK_ADDR *)&send_sock_addr_ip,
                             (NET_SOCK_ADDR_LEN) sizeof(send_sock_addr_ip),
                             (NET_ERR *)&err);
        
         
       }
       else
         OSTimeDlyHMSM(0, 0, 0, 100);
         
        
   }
  
  NetSock_Close(socksend, &err);
 
}
//--------------------------UDP RECEIVE------------------------------------------
static void
App_TaskUDPreceive (void *p_arg)
{
  /*  NET_SOCK_ID        sock;
    NET_SOCK_ADDR_IP   server_sock_addr_ip;
    NET_SOCK_ADDR_LEN  server_sock_addr_ip_size;
    NET_SOCK_ADDR_IP   client_sock_addr_ip;
    NET_SOCK_ADDR_LEN  client_sock_addr_ip_size;*/
    CPU_BOOLEAN        attempt_rx;
    NET_ERR            err;
    
  //  module_gui_text_string (" UDP SEVER ON");
    //module_7segment_put_number(APP_SEG1,3);
    /* ----------------- OPEN IPV4 SOCKET ----------------- */

    
    
    do {
     // module_7segment_put_number(APP_SEG1,10);
      /* ----- WAIT UNTIL RECEIVING DATA FROM A CLIENT ------ */
  //   module_gui_text_string (" UDP SEVER receive");
        client_sock_addr_ip_size = sizeof(client_sock_addr_ip);
        
        rx_size = NetSock_RxDataFrom((NET_SOCK_ID        ) sock,
                                     (void              *) rx_buf,
                                     (CPU_INT16U         ) RX_BUF_SIZE,
                                     (CPU_INT16S         ) NET_SOCK_FLAG_NONE,
                                     (NET_SOCK_ADDR     *)&client_sock_addr_ip,
                                     (NET_SOCK_ADDR_LEN *)&client_sock_addr_ip_size,
                                     (void              *) 0,
                                     (CPU_INT08U         ) 0,
                                     (CPU_INT08U        *) 0,
                                     (NET_ERR           *)&err);
      /*  if(first==0)
        {
          char ch;

          ch= rx_buf[0];
          sitex=ch-48;
          module_7segment_put_number(APP_SEG1,sitex);
          printx=true;
          cantouch=true;
        }
        else
        {
            char ch;

          ch= rx_buf[0];
          siteo=ch-48;
          module_7segment_put_number(APP_SEG1,siteo);
          printo=true;
          cantouch=true;
        }*/
        if(roundgo)
        {
          for(int i=0;i<=3;i++)
          {
            bnum[i]=rx_buf[i];
          }
          bnumok=true;
          OSSemPost(resultflag) ;
          
        }
       else if(!bansok &&strlen(rx_buf)==4)
        {
          for(int i=0;i<=3;i++)
          {
            bans[i]=rx_buf[i];
          }
          //module_gui_text_printf_line(16,"his ans %s",bans);
          bansok=true;
          
           
          
        }
       else if(strcmp (rx_buf,"second") == 0)
        {
          first=1;
          conect=true;
          module_gui_text_printf_line(5,"connect success!! press key0");
        }
        else if(strcmp (rx_buf,"ready") == 0)
        {
         module_gui_text_printf_line(14,"key in four number as number");
              numchioce=0;
              answerin=false;
          
        }
        
       //module_gui_text_printf_line(15,"recevie :\n%s",rx_buf);

        //module_gui_text_printf_line(9,"error:%d\n",err);
        switch (err) {
          
            case NET_SOCK_ERR_NONE:
                 attempt_rx = DEF_NO;
                 break;

            case NET_SOCK_ERR_RX_Q_EMPTY:
            case NET_OS_ERR_LOCK:                
                 attempt_rx = DEF_YES;
                 break;

            default:
                 attempt_rx = DEF_NO;
                 
                 break;
       
        }
        module_led_toggle(APP_LED1); 
        OSTimeDlyHMSM(0, 0, 0, 500);
    } while (1);
    
    
  

    
}
static void
App_Task_keynum (void *p_arg)
{
        gameview();
        uint8_t     i2c_idx = APP_I2C9;
        module_touch_init(i2c_idx);
        
        int index=0;
        while(1)
  {
    touchgood=false;
        int  x, y, z, pressed;
        int  err;
        if(clear)
        {
          gameview();
          clear=false;
        }
         if(showreview)
          {
            module_gui_draw_rect_fill_color(10, 130, 100, 300,GUI_RGB(200,235,235));
            module_gui_text_printf_line(28, "number %d",reprint);
            index=reprint*5-5;
           for(int i=18;i<=26;i+=2)
           {    
             if(index<reviewindex)
             {
               module_gui_text_printf_line(i, "%s",review[index]);
               index++;
             }
             
           }
            showreview=false;
            reprint++;
          }

       
        err = module_touch_active_x(i2c_idx);	
        if (err == -1){
        	return;
        }
        VK_DELAY_MS(1);
        
        x = module_touch_measure_x(i2c_idx);	
        if (x == -1){
        	return;
        }        
        
        err = module_touch_active_y(i2c_idx);	
        if (err == -1){
        	return;
        }
        VK_DELAY_MS(1);
        
        y = module_touch_measure_y(i2c_idx);	
        if (y == -1){
        	return;
        }              
        
        err = module_touch_active_z(i2c_idx);	
        if (err == -1){
        	return;
        }
        VK_DELAY_MS(1);
        
        z = module_touch_measure_z(i2c_idx);	
        if (z == -1){
        	return;
        }                    
         int num=0;
        if(z>10)
        { //module_gui_text_clear_line(2);
          
          
          if(x>2900)
          {
             if(y>500 && y<1100)
            {
              num=0;
            }
            else if(y>1100 && y<1700)
            {
              num=2;
            }
            else if(y>1700 && y<2300)
            {
              num=4;
            }
            else if(y>1700 && y<2900)
            {
              num=6;
            }
             else if(y>2900)
            {
              num=8;
            }
            touchgood=true;
          }
          else if(x>2200 &&x<2900)
          {
     
             if(y>500 && y<1100)
            {
              num=1;
            }
            else if(y>1100 && y<1700)
            {
              num=3;
            }
            else if(y>1700 && y<2300)
            {
              num=5;
            }
            else if(y>1700 && y<2900)
            {
              num=7;
            }
             else if(y>2900)
            {
              num=9;
            }
          touchgood=true;
        }
        
        }
       
       if(numchioce<4 && touchgood==true)
       { 
         bool numsame=false;
         for(int i=0;i<=3;i++)
         {
           if(numset[num]==number[i])
             numsame=true;
         }
         if(!numsame)
         {
         number[numchioce]=numset[num];
         numchioce++;
         }
         touchgood=false;
         
        // strcat(number, numset[num]);
         
         
        module_gui_text_clear_line(14);
        module_gui_text_printf_line(14, "keyin number is %s", number);
          if(numchioce==4&& !answerin)
          {
            answertake();
            OSSemPost(Sendflag); 
            numchioce=0;
            if(bansok)
              roundgo=true;
          }
       else if(numchioce==4&&roundgo)
       {
         mynumok=true;
         OSSemPost(Sendflag); 
         OSSemPost(resultflag) ;
         printline+=2;
         if(printline>=27)
         {
           for(int i=printline;i>=16;i-=2)
           {
             module_gui_text_clear_line(i);
           }
           printline=18;        
            module_gui_draw_rect_fill_color(10, 130, 100, 300,GUI_RGB(200,235,235));
         }
         
       }
       
      }
      if(gamestart)
      {
        module_gui_text_clear_line(0);
        module_gui_text_printf_line(0, "start %s round %d", answer,turn);
        module_gui_text_printf_line(15, "                Your        Enemy");
      }
      if(numchange)
      {
                module_gui_text_clear_line(14);
        module_gui_text_printf_line(14, "keyin number is %s", number);
        numchange=false;
      }
      if(answerin&&bansok)
      {
        gamestart=true;
        roundgo=true;
      }
        
        
         pressed = (z > 10);
         module_gui_text_printf_line(2, "%s", pressed ? "Down" : "Up  ");
        //module_gui_text_printf_line(2, "%s", pressed ? "Down" : "Up  ");
        OSTimeDly(600);
  }
}
static void
App_Task_Result (void *p_arg)
{
  uint8_t               err1;
  
  
while(1)
  {
    OSSemPend(resultflag,500,&err1);
      int brA=0,brB=0;
    if(err1==OS_ERR_NONE && mynumok && bnumok)
       {
         
         resultA=0;resultB=0;
            for(int i=0;i<=3;i++)
            {
              for(int j=0;j<=3;j++)
              {
                if(number[i]==bans[j] && i==j)
                  resultA++;
                else if (number[i]==bans[j])
                  resultB++;
                if(bnum[i]==answer[j] && i==j)
                  brA++;
                else if (bnum[i]==answer[j])
                  brB++;
              }
            }
            module_gui_text_printf_line(printline, "   round %d---%s-%dA%dB // %s---%dA%dB", turn,number,resultA,resultB,bnum,brA,brB);
            sprintf(review[reviewindex],"   round %d---%s-%dA%dB ", turn,number,resultA,resultB);
            if(reviewindex%5==0)
            {
              reprintmax++;
             // module_7segment_put_number(APP_SEG1,reprintmax);
            }
            reviewindex++;
             turn++;
        
          clearnumber();  
          if(resultA==4 && brA==4)
          {
            module_gui_set_text_color(GUI_ORANGE);
            module_gui_text_printf_line(5, "peace!! use %d round",turn-1);
          }
          else if(resultA==4)
          {
            module_gui_set_text_color(GUI_ORANGE);
            module_gui_text_printf_line(5, "you win!! use %d round",turn-1);
          }
          else if(brA==4)
          { module_gui_set_text_color(GUI_ORANGE);
            module_gui_text_printf_line(5, "you lose !! use %d round",turn-1);
          }
          mynumok=false;bnumok=false;
          numchioce=0;
          

       }
    else
    OSTimeDly(2000);
  }
}
static void clearnumber()
{
            for(int i=0;i<=3;i++)
         {
           
           number[i]=' ';
           
         }
}
static void answertake()
{

          for(int i=0;i<=3;i++)
         {
           answer[i]=number[i];
           number[i]=' ';
           
         }
        //module_gui_text_clear_line(15);
       //module_gui_text_printf_line(15, "answer is %s", answer);
       answerin=true;
       gameview();
}
static void gameview()
{ //horizontal
  
        module_gui_clear(GUI_WHITE);
          module_gui_draw_line(10,0,310,0,GUI_RED);
        module_gui_draw_line(10,110,310,110,GUI_RED);
        module_gui_draw_line(10,55,310,55,GUI_RED);
       //vertical
        module_gui_draw_line(10,0,10,110,GUI_RED);
        module_gui_draw_line(310,0,310,110,GUI_RED);
        module_gui_draw_line(70,0,70,110,GUI_RED); // 1
        module_gui_draw_line(130,0,130,110,GUI_RED); // 2
        module_gui_draw_line(190,0,190,110,GUI_RED); // 3
        module_gui_draw_line(250,0,250,110,GUI_RED); // 4
        //text
        module_gui_draw_rect_fill_color(10, 130, 100,300,GUI_RGB(200,235,235));
        int numx=35,numy=27,recx=17,recy=5;
        for(int i=0;i<=9;i++)
        {
          if(i%2==0)
          { 
          module_gui_draw_rect_fill_color(recx, recy, 45, 45, GUI_YELLOW);
          module_gui_text_char_at(i+48, numx, numy, GUI_BLUE, GUI_YELLOW);
            numy=numy+55;
            recy=recy+55;
          }
            else
            { 
             module_gui_draw_rect_fill_color(recx, recy, 45, 45, GUI_YELLOW);
             module_gui_text_char_at(i+48,numx, numy, GUI_BLUE, GUI_YELLOW);
            numx=numx+60;numy=27;
            recx=recx+60;recy=5;
            }
        }
        
        
}

static void 
KEY0_IRQHandler (void)
{
             

             
             if(gamestart)
             {
               if(reprint<=reprintmax )
               {
                 showreview=true;
               }
               else
                 reprint=1;
               
             }
             else
             {
                if(!conect)
                {
                conect=true;
             
                OSSemPost(Sendflag);
              
                }
                else
                {
               
               ready=true;
                              
               OSSemPost(Sendflag);
                }
             for(int i=0;i<=3;i++)
                 {
                  answer[i]=' ';
                  number[i]=' ';
                }
             }

         /*  */

             //cantouch=true;
            EXTI_ClearITPendingBit(EXTI_Line0);
}
static void 
KEY1_IRQHandler (void)
{
        number[numchioce-1]=' ';
        numchioce--;
        numchange=true;
            EXTI_ClearITPendingBit(KEY1_EXTI_LINE);
}


static void 
KEY0_Config (void)
{
        EXTI_InitTypeDef   EXTI_InitStructure;
        GPIO_InitTypeDef   GPIO_InitStructure;

        /* Enable GPIOA clock */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        /* Enable SYSCFG clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
        /* Configure PA0 pin as input floating */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        /* Connect EXTI Line0 to PA0 pin */
        SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

        /* Configure EXTI Line0 */
        EXTI_InitStructure.EXTI_Line = EXTI_Line0;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* Enable and set EXTI Line0 Interrupt */  
        BSP_IntVectSet(BSP_INT_ID_EXTI0, KEY0_IRQHandler); 
        BSP_IntEn(BSP_INT_ID_EXTI0);
}
static void 
KEY1_Config (void)
{
             EXTI_InitTypeDef   EXTI_InitStructure;
        GPIO_InitTypeDef   GPIO_InitStructure;

        /* Enable GPIOA clock */
        RCC_AHB1PeriphClockCmd(KEY1_GPIO_CLK, ENABLE);
        /* Enable SYSCFG clock */
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

        /* Configure PA0 pin as input floating */
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
        GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);

        /* Connect EXTI Line0 to PA0 pin */
        SYSCFG_EXTILineConfig(KEY1_EXTI_GPIO_PORT, KEY1_EXTI_GPIO_PINSRC);

        /* Configure EXTI Line0 */
        EXTI_InitStructure.EXTI_Line = KEY1_EXTI_LINE;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        /* Enable and set EXTI Line0 Interrupt */
        BSP_IntVectSet(BSP_INT_ID_EXTI15_10, KEY1_IRQHandler);
        BSP_IntEn(BSP_INT_ID_EXTI15_10);
}

static void socketcreat(void)
{
      CPU_BOOLEAN        attempt_rx;
      NET_ERR            err;
      sock = NetSock_Open( NET_SOCK_ADDR_FAMILY_IP_V4,
                         NET_SOCK_TYPE_DATAGRAM,
                         NET_SOCK_PROTOCOL_UDP,
                        &err);
    if (err != NET_SOCK_ERR_NONE) {
         module_gui_text_printf_line(10,"ET_SOCK_ERR_NONE error");
        module_7segment_put_number(APP_SEG1,14);
    }
    
    /* ------------ CONFIGURE SOCKET'S ADDRESS ------------ */
    //server struct set
    server_sock_addr_ip_size = sizeof(server_sock_addr_ip);
    Mem_Clr((void     *)&server_sock_addr_ip,
            (CPU_SIZE_T) server_sock_addr_ip_size);
    server_sock_addr_ip.AddrFamily = NET_SOCK_ADDR_FAMILY_IP_V4;
    server_sock_addr_ip.Addr       = NET_UTIL_HOST_TO_NET_32(NET_SOCK_ADDR_IP_WILDCARD); 
    server_sock_addr_ip.Port       = NET_UTIL_HOST_TO_NET_16(UDP_SERVER_PORT);
    
    
    /* ------------------- BIND SOCKET -------------------- */
     NetSock_Bind((NET_SOCK_ID      ) sock,
                 (NET_SOCK_ADDR   *)&server_sock_addr_ip,
                 (NET_SOCK_ADDR_LEN) sizeof(server_sock_addr_ip),
                 (NET_ERR         *)&err);
    
    if (err != NET_SOCK_ERR_NONE) {
        module_led_on(APP_LED1);
        NetSock_Close(sock, &err);
         module_gui_text_printf_line(15,"ET_SOCK_ERR_NONE error");
    }
}
