#define F_CPU 14745600UL
#include <avr/io.h>
#include <util/delay.h>
#include "Lcd/lcd.h"

//------------------------------
// LCD 및 키패드 설정
//------------------------------
#define ROWS 4            // 키패드 행 개수
#define COLS 3            // 키패드 열 개수
#define LCD_COLS 16       // LCD 한 줄 문자 수 (16x2 LCD 기준)
#define LCD_ROWS 2        // LCD 줄 수

//------------------------------
// 키패드 핀 연결
//------------------------------
unsigned char rowPins[ROWS] = {0x10, 0x20, 0x40, 0x80}; // PD4~PD7: 행 출력
unsigned char colPins[COLS] = {0x01, 0x02, 0x04};        // PD0~PD2: 열 입력

// 키패드 값 매핑
char keyValues[ROWS][COLS] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

//------------------------------
// LCD 현재 위치 추적
//------------------------------
int lcd_col = 0;  // LCD 현재 열 (0~15)
int lcd_row = 0;  // LCD 현재 행 (0~1)

//------------------------------
// 키패드 초기화 함수
//------------------------------
void keypad_Init(){
	// 상위 4비트(PD4~PD7) 출력, 하위 4비트(PD0~PD3) 입력
	DDRD = 0xF0;
}

//------------------------------
// 키패드 스캔 함수
//------------------------------
void scanKeypad(void)
{
	for(int r = 0; r < ROWS; r++)   // 모든 행을 순회
	{
		PORTD = rowPins[r];  // 현재 행을 LOW로 출력
		_delay_us(50);       // 신호 안정화 대기

		for(int c = 0; c < COLS; c++)  // 모든 열 확인
		{
			// 해당 열이 HIGH이면 해당 키가 눌린 상태
			if((PIND & colPins[c]) == colPins[c])
			{
				LCD_CHAR(keyValues[r][c]);  // LCD에 키 값 출력
				lcd_col++;                   // 다음 출력 위치로 이동

				// 한 줄이 가득 찼으면 다음 줄로 이동
				if(lcd_col >= LCD_COLS)
				{
					lcd_col = 0;                     // 열 초기화
					lcd_row = (lcd_row + 1) % LCD_ROWS; // 다음 줄로 이동, 2줄이면 순환
					LCD_pos(lcd_row, 0);            // LCD 커서 위치 갱신
				}
			}
		}
	}
	_delay_ms(150); // 디바운스 + 중복 입력 방지
}

//------------------------------
// 메인 함수
//------------------------------
int main(void)
{
	Port_Init();      // LCD 포트 초기화
	LCD_Init();       // LCD 초기화
	keypad_Init();    // 키패드 초기화

	LCD_pos(0, 0);   // LCD 커서 첫 줄 시작

	while(1)
	{
		scanKeypad();  // 키패드 스캔 + LCD 출력 반복
	}
}
