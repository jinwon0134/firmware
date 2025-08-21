#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define LED_ACTIVE_LOW 1
#define PWM_STEPS 16
#define RAMP_STEP_MS 20
#define DEBOUNCE_MS 50

volatile uint8_t pwm_phase = 0;
volatile uint8_t led_level[8] = {0};   // 현재 밝기
uint8_t led_target[8] = {0};           // 목표 밝기

volatile uint16_t adc_value = 0;

// Timer2: 소프트웨어 PWM
void timer2_init(void) {
	TCCR2 = (1 << WGM21) | (1 << CS22); // CTC 모드, 분주 64
	OCR2  = 300;                        // 2kHz
	TIMSK |= (1 << OCIE2);              // 비교 일치 인터럽트 허용
}

ISR(TIMER2_COMP_vect) {
	pwm_phase++;
	if (pwm_phase >= PWM_STEPS) pwm_phase = 0;

	uint8_t bits = 0;
	for (uint8_t i = 0; i < 8; i++) {
		if (led_level[i] > pwm_phase) bits |= (1u << i);
	}

	#if LED_ACTIVE_LOW
	PORTB = ~bits;
	#else
	PORTB = bits;
	#endif
}

// ADC 완료 인터럽트
ISR(ADC_vect) {
	adc_value = ADCW;           // 0~1023
	ADCSRA |= (1 << ADSC);      // 다음 변환 시작
}

void adc_init(void) {
	ADMUX  = (1 << REFS0) | (0 << MUX0); // AVCC 기준 전압, ADC0 사용
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); // 128분주, 인터럽트
	ADCSRA |= (1 << ADSC); // 첫 변환 시작
}

static inline void update_targets_from_adc(void) {
	// ADC 값 (0~1023)을 PWM_STEPS(0~16)로 매핑
	uint8_t brightness = (adc_value * PWM_STEPS) / 1024;
	if (brightness > PWM_STEPS) brightness = PWM_STEPS;

	for (uint8_t i = 0; i < 8; i++) {
		led_target[i] = brightness;
	}
}

int main(void) {
	// LED 출력
	DDRB = 0xFF;
	#if LED_ACTIVE_LOW
	PORTB = 0xFF;
	#else
	PORTB = 0x00;
	#endif

	// 버튼 입력은 이번엔 사용 안함 (필요 시 추가 가능)
	DDRD = 0x00;
	PORTD = 0xFF;

	timer2_init();
	adc_init();
	sei();

	uint16_t ramp_timer = 0;

	while (1) {
		update_targets_from_adc();

		_delay_ms(2);
		ramp_timer += 2;
		if (ramp_timer >= RAMP_STEP_MS) {
			ramp_timer = 0;
			for (uint8_t i = 0; i < 8; i++) {
				if (led_level[i] < led_target[i]) led_level[i]++;
				else if (led_level[i] > led_target[i]) led_level[i]--;
			}
		}
	}
}
