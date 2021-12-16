# Table of contents 

# Features 

The project will involve the combination of two programs with different functionalities implemented on the **mbed** platform with **MBED NXP LPC1768** as the microcontroller. 

The program combinations:

- Simple game: Circle jumping over randomly generated blocks over a straight line
- Timer: Countdown clock with adjustable initial values

Program features

- General
  - If the **temperature** is more than 40ο C it displays a message that "The device is not safe to use"
  - If the **humidity** is more than 70% it will display that "The device is not safe to use"
  - It will also display "The device is not safe to use" if both **temperature** and **humidity** are high
  - Switching to each part of the program is done with a **switch**
  - Switching to the timer will pause the game
  - Switching to the game will reset the timer
- The game
  - Has logical collision detection
  - Displays the score at all times
  - Score increases only when the player (circle) is on the ground and pauses when jumping
  - Player jumping is animated smoothly
  - Jumping up makes a sound through a **speaker**
  - Reaching the ground makes a sound through the speaker
  - Jumping is controlled by a **button**
  - Block generation height will be accounted for when it is compared to the player's jump height
  - Game over makes a sound through the speaker
  - Game over will print the final score of the game
  - Game over will light the **4** **LEDS** on the microcontroller
- The timer
  - Timer initial setting is controlled using a **potentiometer** with the following format: "minutes : seconds"
  - There is a **start** / **pause** functionality using the button 
  - There is a **reset** functionality using another **button** and this functionality can work at any time of using the timer
  - The last 1 minute will light up the 4 LEDS in a consecutive sequence such that each LED lights up after the other
  - The last 10 seconds will have a beep from the speaker produced after each second
  - When the timer finishes we will have another sound through the speaker
  - There will be a simple animation as the timer finishes then it will return to the default value set by the potentiometer
  - The timer will have a limit of 1 hour

# Design

* Components 

|                          Component                          | Variable name |                        Functionality                         |
| :---------------------------------------------------------: | :-----------: | :----------------------------------------------------------: |
| ![image-20210325200523811](https://i.imgur.com/v7nvdDn.png) |    ``lcd``    |                     Displays everything                      |
| ![image-20210325200557499](https://i.imgur.com/PZRcGSj.png) |  ``speaker``  |                 Outputs all different sounds                 |
| ![image-20210325200716212](https://i.imgur.com/SkvJmjt.png) |  ``sensor``   | Measures humidity and <br /> tempreature of the surroundings |
| ![image-20210325200745603](https://i.imgur.com/wLMewly.png) |    ``key``    |            Switches between<br /> game and timer             |
| ![image-20210325202857227](https://i.imgur.com/rRycZGO.png) | ``button_2``  |                       reset for timer                        |
| ![image-20210325202917698](https://i.imgur.com/hCN3nn3.png) | ``button_1``  |           jump (game) <br /> start / pause (timer)           |
| ![image-20210325202935676](https://i.imgur.com/7ZtRtbk.png) |    ``pot``    |             Controls initial value of the timer              |

* Circuit 

![image-20210503044957018](https://i.imgur.com/KJw3Mcd.png)

* Code design 

![image-20210503045837418](https://i.imgur.com/prkQYCM.png)

The program will be mainly focused on the usage of classes such as ``class Player`` , ``class Obstacle`` and ``class Time``. After that we will have a set of functions that manipulate a combination of the classes so that they are implemented in the ``int main()`` function and induced in the ``while`` loop.

* UML diagram of classes

![image-20210503045024950](https://i.imgur.com/fnq4a2i.png)

* Player game design 

![image-20210503045358078](https://i.imgur.com/QiIookT.png)

The player is drawn as a circle however it is modelled as a square due to many reasons. Modelling the circle as a points can be done by creating a vector ``std::vector`` that contains ``x, y`` floating points of the circle. However the issue with this is that the coordinates of the LCD are all integers so modelling the points as a square will be much more efficient than rounding the float values of the circle points to integers. Eventually this can be used in the collision detection such that we loop through the ranges of the square (using a for loop) with respect to the radius ($y+R \rightarrow y-R$ and $x-R \rightarrow x+R$ ). Then we compare those values to the x-coordinate of the block and the height coordinate of the block. The visualisation of modelling the circle as a square is shown in **figure-4**.

* Grid of the game

![image-20210503045447720](https://i.imgur.com/JIQiP4g.png)

**Figure-5** shows how the game gird will work such that the block will move towards the circle and after reaching the end a new block will randomly generate (using ``rand()``) with a constant width and a limited height such that the player can jump over. However the x-coordinate of the block will generate within a certain range so that it can still be doable as a player.

* Timer

The countdown mechanism was made using ``#include <ctime>``  library. Such that the timer starts when pressing ``button_1`` and resets after each second and decrements the timer. However there is a hold time of $250 \text{ ms}$ between each reset of the timer so that the button can be responsive enough. 

* LCD outputs 

![image-20210401031645427](https://i.imgur.com/jT08OVP.png)

* Demo

<video src="/home/harithalsafi/Videos/1620.mp4"></video>

# Compiling

* Run on [mbed-simulator](https://simulator.mbed.com/) by copying the code in ``main.cpp`` and pasting it in the simulator 
* Can be also compiled locally using [mbed-cli](https://os.mbed.com/docs/mbed-os/v6.9/build-tools/mbed-cli-1.html) assuming you have [gcc-arm](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm) 

```bash
mbed config -G GCC_ARM_PATH /usr/gcc-arm-none-eabi-10-2020-q4-major/bin/
mbed compile --target LPC1768 --toolchain GCC_ARM
```

