#include <asf.h>
#include "conf_board.h"

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* prototypes and types                                                 */
/************************************************************************/
int genius_get_sequence(int level, int *sequence);
void pin_toggle(Pio *pio, uint32_t mask);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq) ;

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/
#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOA
#define BUT_2_PIO_ID ID_PIOA
#define BUT_2_IDX 19
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOC
#define BUT_3_PIO_ID ID_PIOC
#define BUT_3_IDX 31
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)

#define BUZZER          PIOD
#define BUZZER_ID       ID_PIOD
#define BUZZER_IDX     30
#define BUZZER_IDX_MASK (1u << BUZZER_IDX) // esse já está pronto.


#define TASK_OLED_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

void but1_callback(void);
void but2_callback(void);
void but3_callback(void);
void io_init(void);

QueueHandle_t xQueueBtn;
/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/
void but1_callback(void) {
	int id = 0;
	xQueueSendFromISR(xQueueBtn, &id, 0);
}

void but2_callback(void) {
	int id = 1;
	xQueueSendFromISR(xQueueBtn, &id, 0);
}
void but3_callback(void) {
	int id = 2;
	xQueueSendFromISR(xQueueBtn, &id, 0);
}
/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
static void task_game(void *pvParameters) {
	gfx_mono_ssd1306_init();
	
	int level = 0;
	for (;;)  {
		int id;
		int nSequence = 0;
		int sequence[512];
		int i = 0;
		int j = 0;
		nSequence = genius_get_sequence(level, sequence);
		char stringify[20];
		sprintf(stringify, "lvel %d",level);
		gfx_mono_draw_string(stringify, 0, 0, &sysfont);
		for(i = 0; i < nSequence; i++){
			pio_set(LED_1_PIO,LED_1_IDX_MASK);
			pio_set(LED_2_PIO,LED_2_IDX_MASK);
			pio_set(LED_3_PIO,LED_3_IDX_MASK);
			if(sequence[i] == 0){
				TC_init(TC0, ID_TC1, 1, 1000);
				tc_start(TC0, 1);
				pio_clear(LED_1_PIO,LED_1_IDX_MASK);
				delay_ms(200);
				tc_stop(TC0,1);
			}
			if(sequence[i] == 1){
				TC_init(TC0, ID_TC1, 1, 1500);
				tc_start(TC0, 1);
				pio_clear(LED_2_PIO,LED_2_IDX_MASK);
				delay_ms(200);
				tc_stop(TC0,1);
			}
			if(sequence[i] == 2){
				TC_init(TC0, ID_TC1, 1, 2000);
				tc_start(TC0, 1);
				pio_clear(LED_3_PIO,LED_3_IDX_MASK);
				delay_ms(200);
				tc_stop(TC0,1);
			}
		}
		int c =0 ;
		while(c < nSequence){
			if (xQueueReceive(xQueueBtn, &id, 1000)) {
				pio_set(LED_1_PIO,LED_1_IDX_MASK);
				pio_set(LED_2_PIO,LED_2_IDX_MASK);
				pio_set(LED_3_PIO,LED_3_IDX_MASK);
				if(id == 0){
					TC_init(TC0, ID_TC1, 1, 1000);
					tc_start(TC0, 1);
					pio_clear(LED_1_PIO,LED_1_IDX_MASK);
					delay_ms(200);
					tc_stop(TC0,1);
					c++;
				}
				if(id == 1){
					TC_init(TC0, ID_TC1, 1, 1500);
					tc_start(TC0, 1);
					pio_clear(LED_2_PIO,LED_2_IDX_MASK);
					delay_ms(200);
					tc_stop(TC0,1);
					c++;
				}
				if(id == 2){
					TC_init(TC0, ID_TC1, 1, 2000);
					tc_start(TC0, 1);
					pio_clear(LED_3_PIO,LED_3_IDX_MASK);
					delay_ms(200);
					tc_stop(TC0,1);
					c++;
				}
				if(id != sequence[j]){
					level = 0;
					gfx_mono_draw_string("Perdeu", 0, 0, &sysfont);

				}
				else{
						if(j == (nSequence-1)){
							level ++;
				}
				}
			}

		}
	}
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
int genius_get_sequence(int level, int *sequence){
	int n = level + 3;

	for (int i=0; i< n ; i++) {
		*(sequence + i) = rand() % 3;
	}

	return n;
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void TC1_Handler(void) {
	volatile uint32_t status = tc_get_status(TC0, 1);
	pin_toggle(BUZZER, BUZZER_IDX_MASK);
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq) {
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

void io_init(void) {
	pmc_enable_periph_clk(LED_1_PIO_ID);
	pmc_enable_periph_clk(LED_2_PIO_ID);
	pmc_enable_periph_clk(LED_3_PIO_ID);
	pmc_enable_periph_clk(BUT_1_PIO_ID);
	pmc_enable_periph_clk(BUT_2_PIO_ID);
	pmc_enable_periph_clk(BUT_3_PIO_ID);

	pio_configure(LED_1_PIO, PIO_OUTPUT_0, LED_1_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED_2_PIO, PIO_OUTPUT_0, LED_2_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED_3_PIO, PIO_OUTPUT_0, LED_3_IDX_MASK, PIO_DEFAULT);

	pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

	pio_handler_set(BUT_1_PIO, BUT_1_PIO_ID, BUT_1_IDX_MASK, PIO_IT_FALL_EDGE,
	but1_callback);
	pio_handler_set(BUT_2_PIO, BUT_2_PIO_ID, BUT_2_IDX_MASK, PIO_IT_FALL_EDGE,
	but3_callback);
	pio_handler_set(BUT_3_PIO, BUT_3_PIO_ID, BUT_3_IDX_MASK, PIO_IT_FALL_EDGE,
	but2_callback);

	pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
	pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
	pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);

	pio_get_interrupt_status(BUT_1_PIO);
	pio_get_interrupt_status(BUT_2_PIO);
	pio_get_interrupt_status(BUT_3_PIO);

	NVIC_EnableIRQ(BUT_1_PIO_ID);
	NVIC_SetPriority(BUT_1_PIO_ID, 4);

	NVIC_EnableIRQ(BUT_2_PIO_ID);
	NVIC_SetPriority(BUT_2_PIO_ID, 4);

	NVIC_EnableIRQ(BUT_3_PIO_ID);
	NVIC_SetPriority(BUT_3_PIO_ID, 4);
	
	pmc_enable_periph_clk(BUZZER_ID);
	pio_set_output(BUZZER,BUZZER_IDX_MASK,0,0,0);
	pio_configure(BUZZER, PIO_OUTPUT_0, BUZZER_IDX_MASK, PIO_DEFAULT);
}
/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();
	io_init();

	/* Initialize the console uart */
	configure_console();
	
	xQueueBtn= xQueueCreate(32, sizeof(int));
	if (xQueueBtn == NULL) {
		printf("Falha em criar xQueue \n");
	}

	/* Create task to control oled */
	if (xTaskCreate(task_game, "game", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create game task\r\n");
	}

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
