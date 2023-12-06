/**
    极光的FreeRTOS实验
    使用开发版野火的STM32F429系列开发板
    实验8-计数型信号量实验
    时间：23/12/6
  ******************************************************************************
  */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bsp_led.h"
#include "bsp_key.h" 
#include "bsp_debug_usart.h"

/*************************任务句柄创建********************************/
/*任务控制块指针创建*/
TaskHandle_t AppTaskCreate_Handle = NULL;
TaskHandle_t Receive_Task_Handle = NULL;
TaskHandle_t Send_Task_Handle = NULL;

/*************************内核句柄创建********************************/
SemaphoreHandle_t CountSem_Flag_Handle = NULL;

/****************************函数声明********************************/
static void AppTaskCreate(void);
static void Receive_Test_Task(void * param);
static void Send_Test_Task(void * param);

/******************************仅用于本函数的宏定义*******************************/

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
    
    printf("FreeRTOS实验8,计数信号量实验，作者极光\n");
    
    //创建用来创建任务的任务
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,			/* 任务入口函数 */
                        (const char*	)"AppTaskCreate",			/* 任务名字 */
                        (uint16_t		)256,   					/* 任务栈大小 */
                        (void*			)NULL,						/* 任务入口函数参数 */
                        (UBaseType_t	)1,	    					/* 任务的优先级 */
                        (TaskHandle_t*	)&AppTaskCreate_Handle);	/* 任务控制块指针 */
    
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

    /*创建一个计数信号量*/ 
    CountSem_Flag_Handle = xSemaphoreCreateCounting(5,0);

    /*验证*/ 
    if( CountSem_Flag_Handle != NULL )
    {
        printf("计数信号量创建成功\n");
    }
    
    //创建接收任务
    xReturn = xTaskCreate((TaskFunction_t )Receive_Test_Task,   /* 任务入口函数 */
                        (const char*	)"Receive_Test_Task",	/* 任务名字 */
                        (uint16_t		)512,					/* 任务栈大小 */
                        (void*			)"正在运行接受函数",    /* 任务入口函数参数 */
                        (UBaseType_t	)2,                     /* 任务的优先级 */
                        (TaskHandle_t*	)&Receive_Task_Handle); /* 任务控制块指针 */
    
    /*查看任务是否创建成功*/
    if(xReturn == pdPASS)
    {
        printf("接收任务创建成功\n");
    }
    
    //创建发送任务
    xReturn = xTaskCreate((TaskFunction_t )Send_Test_Task, 		/* 任务入口函数 */
                        (const char*	)"Send_Test_Task",		/* 任务名字 */
                        (uint16_t		)512,                   /* 任务栈大小 */
                        (void*			)"正在运行发送函数",       /* 任务入口函数参数 */
                        (UBaseType_t	)3,						/* 任务的优先级 */
                        (TaskHandle_t*	)&Send_Task_Handle);	/* 任务控制块指针 */
                            
    /*查看任务是否创建成功*/
    if(xReturn == pdPASS)
    {
        printf("发送任务创建成功\n");
    }
    
    /*删除自身*/
    /*？？这里我尝试了直接输入NULL来删除自身，但程序马上就会卡死在prvTaskExitError这个函数中，很怪*/
    vTaskDelete(AppTaskCreate_Handle);
    
    taskEXIT_CRITICAL();
}

static void Receive_Test_Task(void * param)
{
    /*定义消息验证值，初始默认为pdPASS*/
    BaseType_t xReturn = pdPASS;
    while(1)
    {
        /*K2被按下*/
        if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )
        {
            /*获取计数信号量 xSemaphore,不等待*/
            xReturn = xSemaphoreTake(CountSem_Flag_Handle,0); 
            if( xReturn == pdTRUE )
            {
                printf("信号量获取成功!\r\n");
            }
            else
            {
                printf("信号量获取失败，当前信号量队列为满!\r\n");
            }
        } 

        vTaskDelay(20);
    }
}

void Send_Test_Task(void * param)
{
    /*定义消息验证值，初始默认为pdPASS*/
    BaseType_t xReturn = pdPASS;
    while(1)
    {
        /*K1被按下*/
        if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )
        {
            /*给出计数信号量*/
            xReturn = xSemaphoreGive( CountSem_Flag_Handle );
            if( xReturn == pdTRUE )
            {
                printf("信号量释放成功!\r\n");
            }
            else
            {
                printf("信号量释放失败，当前信号量队列为空!\r\n");
            }
        } 
        /*延时20个tick*/
        vTaskDelay(20);
    }
}




/*********************************************END OF FILE**********************/

