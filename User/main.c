/**
	极光的FreeRTOS实验
	使用开发版野火的STM32F429系列开发板
	实验3-使用队列进行任务与任务之间的通讯
	时间：23/11/17
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_led.h"
#include "bsp_key.h" 
#include "bsp_debug_usart.h"

/*************************任务句柄创建********************************/
/*任务控制块指针创建*/
TaskHandle_t AppTaskCreate_Handle = NULL;
TaskHandle_t LED_Receive_Task_Handle = NULL;
TaskHandle_t KEY_Send_Task_Handle = NULL;

/*************************内核句柄创建********************************/
QueueHandle_t Test_Queue = NULL;


/****************************函数声明********************************/
static void AppTaskCreate(void);
static void LED_Receive_Task(void);
static void KEY_Send_Task(void);

/******************************仅用于本函数的宏定义*******************************/
/*队列的长度，最大可包含多少个消息*/
#define  QUEUE_LEN    4   
/*队列中每个消息大小*/
#define  QUEUE_SIZE   4   

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
	/*定义消息验证值，初始默认为pdPASS*/
	BaseType_t xReturn = pdPASS;
	/*LED 端口初始化*/
	LED_GPIO_Config();
	/*串口初始化*/
	Debug_USART_Config();
	/*初始化按键*/
	Key_GPIO_Config();
	
	LED_RGBOFF;
	
	printf("FreeRTOS实验3,队列实验，作者极光\n");
	
	//创建用来创建任务的任务
	xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate, /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )256,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )1,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */
	
	/*启动任务调度*/
	if(xReturn == pdPASS)
	{
		/*启动任务调度器*/
		vTaskStartScheduler();
	}
	else
	{
		printf("任务调度失败，请检查程序\n");
		return -1;
	}
	
	/*正常来说不会执行到这里*/
	while(1);
}

/**************************************************************************************/
static void AppTaskCreate(void)
{
	/*定义消息验证值，初始默认为pdPASS*/
	BaseType_t xReturn = pdPASS;
	
	/*开启临界段保护*/
	taskENTER_CRITICAL();
	
	/* 创建Test_Queue */
  Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
  if(NULL != Test_Queue)
	{
		printf("创建Test_Queue消息队列成功!\r\n");
	}
	
	//创建LED任务
	xReturn = xTaskCreate((TaskFunction_t )LED_Receive_Task, /* 任务入口函数 */
                        (const char*    )"LED_Receive_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&LED_Receive_Task_Handle);/* 任务控制块指针 */
	
	/*查看任务是否创建成功*/
	if(xReturn == pdPASS)
	{
		printf("LED任务创建成功\n");
	}
	
	//创建KEY任务
	xReturn = xTaskCreate((TaskFunction_t )KEY_Send_Task, /* 任务入口函数 */
                        (const char*    )"KEY_Send_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&KEY_Send_Task_Handle);/* 任务控制块指针 */
							
	/*查看任务是否创建成功*/
	if(xReturn == pdPASS)
	{
		printf("KEY任务创建成功\n");
	}
	
	/*删除自身*/
	/*？？这里我尝试了直接输入NULL来删除自身，但程序马上就会卡死在prvTaskExitError这个函数中，很怪*/
	vTaskDelete(AppTaskCreate_Handle);
	
	taskEXIT_CRITICAL();
}

static void LED_Receive_Task(void)
{
	/*定义消息验证值，初始默认为pdPASS*/
	BaseType_t xReturn = pdPASS;
	/*定义一个接收消息的变量*/
	uint32_t led_queue = 0;
	
	while(1)
	{
		/*使用led_queue接受队列Test_Queue传来的变量，等待时间为最大*/
		xReturn = xQueueReceive(Test_Queue,&led_queue,portMAX_DELAY);

//		/*数据接受检测*/
		if(xReturn != pdPASS)
		{
			printf("数据接收出错,错误代码0x%lx\n",xReturn);
		}
		
		/*如果传来的数据是1*/
		if(led_queue == 1)
		{
			/*红灯翻转*/
			LED1_TOGGLE;
			printf("红灯翻转\n");
		}
		/*如果传来的数据是2*/
		if(led_queue == 2)
		{
			/*则蓝灯翻转*/
			LED2_TOGGLE;
			printf("蓝灯翻转\n");
		}
	}
}

void KEY_Send_Task(void)
{
	/*定义消息验证值，初始默认为pdPASS*/
	BaseType_t xReturn = pdPASS;
	
	uint32_t send_data1 = 1;
  uint32_t send_data2 = 2;
	while(1)
	{
		/* K1 被按下 */
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )
    {
      printf("发送消息send_data1！\n");
			/*立刻往队列Test_Queue发送send_data1值，不进行等待*/
      xReturn = xQueueSend(Test_Queue,&send_data1,0); 
      if(pdPASS == xReturn)
			{
				printf("消息send_data1发送成功!\n\n");
			}
			else
			{
				printf("消息send_data1发送失败\n\n");
			}
    } 
		
		/* K2 被按下 */
    if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )
    {
      printf("发送消息send_data2！\n");
      /*立刻往队列Test_Queue发送send_data2值，不进行等待*/
      xReturn = xQueueSend(Test_Queue,&send_data2,0); 
			
      if(pdPASS == xReturn)
			{
				printf("消息send_data2发送成功!\n\n");
			}
			else
			{
				printf("消息send_data2发送失败\n\n");
			}
    }
		
		/*延时20个tick*/
    vTaskDelay(20);
	}
}
 


/*********************************************END OF FILE**********************/

