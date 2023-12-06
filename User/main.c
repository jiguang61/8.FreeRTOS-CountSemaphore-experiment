/**
    �����FreeRTOSʵ��
    ʹ�ÿ�����Ұ���STM32F429ϵ�п�����
    ʵ��8-�������ź���ʵ��
    ʱ�䣺23/12/6
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

/*************************����������********************************/
/*������ƿ�ָ�봴��*/
TaskHandle_t AppTaskCreate_Handle = NULL;
TaskHandle_t Receive_Task_Handle = NULL;
TaskHandle_t Send_Task_Handle = NULL;

/*************************�ں˾������********************************/
SemaphoreHandle_t CountSem_Flag_Handle = NULL;

/****************************��������********************************/
static void AppTaskCreate(void);
static void Receive_Test_Task(void * param);
static void Send_Test_Task(void * param);

/******************************�����ڱ������ĺ궨��*******************************/

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
    /*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
    BaseType_t xReturn = pdPASS;
    /*LED �˿ڳ�ʼ��*/
    LED_GPIO_Config();
    /*���ڳ�ʼ��*/
    Debug_USART_Config();
    /*��ʼ������*/
    Key_GPIO_Config();
    
    LED_RGBOFF;
    
    printf("FreeRTOSʵ��8,�����ź���ʵ�飬���߼���\n");
    
    //���������������������
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,			/* ������ں��� */
                        (const char*	)"AppTaskCreate",			/* �������� */
                        (uint16_t		)256,   					/* ����ջ��С */
                        (void*			)NULL,						/* ������ں������� */
                        (UBaseType_t	)1,	    					/* ��������ȼ� */
                        (TaskHandle_t*	)&AppTaskCreate_Handle);	/* ������ƿ�ָ�� */
    
    /*�����������*/
    if(xReturn == pdPASS)
    {
        /*�������������*/
        vTaskStartScheduler();
    }
    else
    {
        printf("�������ʧ�ܣ��������\n");
        return -1;
    }
    
    /*������˵����ִ�е�����*/
    while(1);
}

/**************************************************************************************/
static void AppTaskCreate(void)
{
    /*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
    BaseType_t xReturn = pdPASS;
    
    /*�����ٽ�α���*/
    taskENTER_CRITICAL();

    /*����һ�������ź���*/ 
    CountSem_Flag_Handle = xSemaphoreCreateCounting(5,0);

    /*��֤*/ 
    if( CountSem_Flag_Handle != NULL )
    {
        printf("�����ź��������ɹ�\n");
    }
    
    //������������
    xReturn = xTaskCreate((TaskFunction_t )Receive_Test_Task,   /* ������ں��� */
                        (const char*	)"Receive_Test_Task",	/* �������� */
                        (uint16_t		)512,					/* ����ջ��С */
                        (void*			)"�������н��ܺ���",    /* ������ں������� */
                        (UBaseType_t	)2,                     /* ��������ȼ� */
                        (TaskHandle_t*	)&Receive_Task_Handle); /* ������ƿ�ָ�� */
    
    /*�鿴�����Ƿ񴴽��ɹ�*/
    if(xReturn == pdPASS)
    {
        printf("�������񴴽��ɹ�\n");
    }
    
    //������������
    xReturn = xTaskCreate((TaskFunction_t )Send_Test_Task, 		/* ������ں��� */
                        (const char*	)"Send_Test_Task",		/* �������� */
                        (uint16_t		)512,                   /* ����ջ��С */
                        (void*			)"�������з��ͺ���",       /* ������ں������� */
                        (UBaseType_t	)3,						/* ��������ȼ� */
                        (TaskHandle_t*	)&Send_Task_Handle);	/* ������ƿ�ָ�� */
                            
    /*�鿴�����Ƿ񴴽��ɹ�*/
    if(xReturn == pdPASS)
    {
        printf("�������񴴽��ɹ�\n");
    }
    
    /*ɾ������*/
    /*���������ҳ�����ֱ������NULL��ɾ���������������ϾͻῨ����prvTaskExitError��������У��ܹ�*/
    vTaskDelete(AppTaskCreate_Handle);
    
    taskEXIT_CRITICAL();
}

static void Receive_Test_Task(void * param)
{
    /*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
    BaseType_t xReturn = pdPASS;
    while(1)
    {
        /*K2������*/
        if( Key_Scan(KEY2_GPIO_PORT,KEY2_PIN) == KEY_ON )
        {
            /*��ȡ�����ź��� xSemaphore,���ȴ�*/
            xReturn = xSemaphoreTake(CountSem_Flag_Handle,0); 
            if( xReturn == pdTRUE )
            {
                printf("�ź�����ȡ�ɹ�!\r\n");
            }
            else
            {
                printf("�ź�����ȡʧ�ܣ���ǰ�ź�������Ϊ��!\r\n");
            }
        } 

        vTaskDelay(20);
    }
}

void Send_Test_Task(void * param)
{
    /*������Ϣ��ֵ֤����ʼĬ��ΪpdPASS*/
    BaseType_t xReturn = pdPASS;
    while(1)
    {
        /*K1������*/
        if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON )
        {
            /*���������ź���*/
            xReturn = xSemaphoreGive( CountSem_Flag_Handle );
            if( xReturn == pdTRUE )
            {
                printf("�ź����ͷųɹ�!\r\n");
            }
            else
            {
                printf("�ź����ͷ�ʧ�ܣ���ǰ�ź�������Ϊ��!\r\n");
            }
        } 
        /*��ʱ20��tick*/
        vTaskDelay(20);
    }
}




/*********************************************END OF FILE**********************/

