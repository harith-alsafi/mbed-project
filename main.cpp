#include "mbed.h"
#include "C12832.h"
#include "Sht31.h"
#include <ctime>
#include <cstdlib>


// general constants 
#define color 1 // color of objects 
#define hold 0.01 // hold time 
#define duration 0.15 // sound duration
#define max_temp 40.0
#define max_humidity 70.0
#define max_seconds 60*60

// coordinate constants
#define ground_x 10 // origin-x 
#define ground_y 20 // origin-y
#define max_up 15 // max jump  
#define raduis 3 // radius
#define max_y ground_y-max_up-raduis // max y for player 
#define min_y ground_y+raduis // min y for player 
#define rate 1 // movement rate 

// program peripherals  
PwmOut speaker(p21); // p21
C12832 lcd(p5, p7, p6, p8, p11); //p5, p6, p7
DigitalIn key(p9); // p9 (game to timer)
Sht31 sensor(I2C_SDA, I2C_SCL); // p27, p28 
BusOut leds(LED4, LED3, LED2, LED1); // LEDSs
AnalogIn pot(p15); // set timer values
DigitalIn button_2(p10); // p9 (reset button)
DigitalIn button_1(p8); // p8 (jump button) / start/pause button for timer

// Game state 
typedef enum{
    playing, // game 
    jumpup, // jump up 
    jumpdown, // jump down 
    over, // game over
    out // to switch to timer 
} GameState;

// timer state 
typedef enum{
    init, // setting the timer 
    running, // running the timer 
    pause, // pause 
    stop // switches to game
} TimerState;

// states 
GameState game_state = playing;
TimerState timer_state = init;

// tone
void tone(float frequency, float volume, float interval, int rest) {
    speaker.period(1.0 / frequency);
    speaker = volume;
    wait(interval);
    speaker = 0.0;
    wait(rest);
}

// player class
class Player{
    private:
        int x; // position x (fixed)
        int y; // position y

    public:
        Player(){
            x = ground_x;
            y = ground_y;
        }
        // jump up
        void jump_up(){
            if(y == ground_y){
                // 75% of duration time 
                tone(300.0, 0.5, duration*0.75, 0);
            }
            if(y == ground_y-max_up){
                // switches to jumpdown state
                game_state = jumpdown;
            }
            else{
                y-=rate;
            }
        }
        // jump down 
        void jump_down(){
            // if y reaches ground 
            if (y == ground_y){
                // 25% of duartion time
                tone(100.0, 0.5, duration*0.25, 0);
                game_state = playing;
            }
            else{
                y+=rate;
            }
        }
        // draw 
        void draw(){
            lcd.fillcircle(x, y, raduis, color);
        }
        // reset (for new game)
        void reset(){
            x = ground_x;
            y = ground_y;            
        }
        // circle is modelled as a sqyare with sides R
        void collide(int x, int height){ // x of block and block height
            if(x <= this->x+raduis && x >= this->x-raduis){
                // going over the height of the block
                for(int i = min_y; i >= min_y-height; i--){
                    if(i <= this->y+raduis && i >= this->y-raduis){
                        // tone 
                        tone(100.0, 0.5, duration, 0);
                        // changing game_state 
                        game_state = over; 
                        return; // to stop running the for loop
                    }
                }
            }
            return;
        }
};

// obstacle class
class Obstacle{
    private:
        int x;
        int y;
        int width;
        int height;
    public:
        Obstacle(){
            width = 2*raduis;
            y = min_y;
        }
        // generate random x and height
        void Generate(){
            // 19 -> 122
            x = (ground_x+rate+raduis)+rand()%(lcd.width()-width);
            // 5 -> 11
            height = (raduis*2-rate)+rand()%(max_up-raduis*2-2*rate);
        }
        // get height
        int GetHeight(){
            return height;
        }
        // get x
        int GetX(){
            return x;
        }
        // draw 
        void draw(){
            lcd.fillrect(x, y, width+x, y-height, color);
        }
        // moving 
        void move(){
            x-= rate;
        }
};

// timer class 
class Time{
    private:
        int seconds_init;
        int minutes_init;
        int led_counter; // counter for leds
        int sound_counter; // counter for sound
    public:
    // constructor
    Time(){
        seconds_init = 0;
        minutes_init = 0;      
        led_counter = 1;  
        sound_counter = 0;
    }
    // display
    void display(){
        lcd.cls();
        if(minutes_init < 10){
            lcd.locate(45, 5);
        }
        else{
            lcd.locate(40, 5);
        }
        lcd.printf("%i  :  %i", minutes_init, seconds_init);
        lcd.locate(24, 15);
        lcd.printf("minutes : seconds");
        lcd.copy_to_lcd();
    }
    // setting up the timer 
    void setting(){
        int pot_time = (int) round(pot.read()*max_seconds); // pot time
        seconds_init = pot_time%60; // sec 
        minutes_init = pot_time/60; // mins
        display();
    }
    // over 
    void over(){
        int x;
        int y;
        lcd.cls();
        for(int i = 0; i < lcd.width()*0.2; i++){
            for (int j = 0; j < lcd.height()*0.2; j++){
                x = 0+rand()%lcd.width();
                y = 0+rand()%lcd.height();
                while(i == x){
                    x = 0+rand()%lcd.width();
                }
                while(j == y){
                    y = 0+rand()%lcd.height();
                }
                lcd.pixel(x, y, color);
                lcd.copy_to_lcd();
                wait(hold);
            }
        }
    }
    // running 
    void run(){
        if(minutes_init == 0){
            // leds
            leds = led_counter;
            led_counter *= 2;
            if(led_counter > 8){
                led_counter = 1;
            }
            // over
            if(seconds_init == 0){
                display();
                tone(400.0, 0.5, 2*duration, 0);
                timer_state = init;
                leds = 0;
                reset();
                over();
                return;
            }
            // last 10 seconds 
            if(seconds_init <= 10){
                tone(400.0, 0.5, duration, 0);
            }
        }
        if(seconds_init == 0){
            minutes_init--;
            seconds_init = 59;
        }
        else{
            seconds_init--;
        }

        display();
        return;
    }
    // pause 
    void pause(){
        display();
    }
    // reset 
    void reset(){
        seconds_init = 0;
        minutes_init = 0;  
        led_counter = 1;
        sound_counter = 0;   
        leds = 0;    
    }
};

// main variables  
// game
Player player;
Obstacle obs;
int score = 0;
// timer
Time timer;
time_t elapsed_time;

// draw game objects
void draw(){
    lcd.cls();
    // displaying the score correctly
    if(score < 1000 && score >= 100){
        lcd.locate(110, 1);
    }
    if(score < 100 && score >= 10){
        lcd.locate(115, 1);
    }
    if(score < 10){
        lcd.locate(120, 1);
    }
    if (score >= 1000) {
        lcd.locate(105, 1);
    }
    lcd.printf("%i", score);
    lcd.line(0, 23, lcd.width(), 23, color); // ground
    player.draw();
    obs.draw();
    lcd.copy_to_lcd();
}

// move game and checks for collision
void move(){
    draw();
    obs.move();
    wait(hold);
    draw();
    player.collide(obs.GetX(), obs.GetHeight());
}

// check switch,temp and humidity
void check(){
    // game
    if (key == 0){
        // we dont change the prevoius game state when cheking
        if(game_state == playing || game_state == out){
            game_state = playing;
        }
        if(game_state == over){
            game_state = over;
        }
        if(game_state == jumpup){
            game_state = jumpup;
        }
        if(game_state == jumpdown){
            game_state = jumpdown;
        }
        timer_state = stop;
    }
    // timer
    if (key == 1){
        if(timer_state == init || timer_state == stop){
            timer_state = init;
        }
        if(timer_state == running){
            timer_state = running;
        }
        if (timer_state == pause){
            timer_state = pause;
        }
        game_state = out;
    }
    // temp
    if(sensor.readTemperature() >= max_temp){
        lcd.cls();
        lcd.locate(20, 6);
        lcd.printf("Teampreature is high");
        lcd.locate(13, 15);
        lcd.printf("The device is not safe");
        lcd.copy_to_lcd();
        game_state = out;
        timer_state = stop;
    }
    // humidity
    if(sensor.readHumidity() >= max_humidity){
        lcd.cls();
        lcd.locate(24, 6);
        lcd.printf("Humidity is high");
        lcd.locate(13, 15);
        lcd.printf("The device is not safe");   
        lcd.copy_to_lcd();  
        game_state = out;
        timer_state = stop;  
    }
    // humidity and temprature 
    if(sensor.readHumidity() >= max_humidity && sensor.readTemperature() >= max_temp){
        lcd.cls();
        lcd.locate(5, 3);
        lcd.printf("Tempreature and Humidity");
        lcd.locate(43, 12);
        lcd.printf("are high");
        lcd.locate(13, 21);
        lcd.printf("The device is not safe");   
        lcd.copy_to_lcd();  
        game_state = out;
        timer_state = stop;       
    }
}

int main()
{
    // seed for random 
    srand(time(NULL));
    obs.Generate();

    // main loop
    while(1){
        check();
        // game 
        while(game_state == playing){
            check();
            // dont want random blocks all the time 
            if(obs.GetX() <= 0){
                obs.Generate();
            }
            move();
            // jumping 
            if (button_1 == 1){
                game_state = jumpup;
            }
            while(game_state == jumpup){
                player.jump_up();
                move();
            }
            while(game_state == jumpdown){
                player.jump_down();
                move();
            }
            // score
            score += rate;
            wait(hold);
            // game over 
            while (game_state == over){
                lcd.cls();
                lcd.locate(37, 1);
                lcd.printf("Game over");
                lcd.locate(20, 10);
                lcd.printf("Your score was: %i", score);
                lcd.locate(13, 20);
                lcd.printf("Press the button again");
                lcd.copy_to_lcd();
                // start again
                if(button_1 == 1){
                    score = 0;
                    game_state = playing;
                    lcd.cls();
                    obs.Generate();
                    player.reset();
                    break;
                }
                check();
                leds = 0b1111;
                wait(hold);
                leds = 0b0000;
            }
        }
        // timer 
        while(timer_state == init){
            check();
            // init
            timer.setting();
            // start countdown 
            if (button_1 == 1){
                timer_state = running;
                time(&elapsed_time);
                wait(hold*25);
            }
            // running 
            while (timer_state == running){
                check();
                if(difftime(time(0), elapsed_time) == 1){
                    timer.run();
                    time(&elapsed_time);
                }
                // pause 
                if(button_1 == 1){
                    timer_state = pause;
                    time(&elapsed_time);
                    wait(hold*25);
                }
                while (timer_state == pause){
                    check();
                    timer.pause();
                    // reset
                    if(button_2 == 1){
                        timer.reset();
                        timer_state = init;
                    }
                    // start again 
                    if(button_1 == 1){
                        timer_state = running;
                        time(&elapsed_time);
                        wait(hold*25);
                    }
                    wait(hold);
                }
                // reset
                if(button_2 == 1){
                    timer.reset();
                    timer_state = init;
                }
                wait(hold);
            }

            wait(hold);
        }
        wait(hold);
    }
}
